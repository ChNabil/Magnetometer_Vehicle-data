#include<msp430f5529.h>

//void Clock_setup(void);
void I2C_setup();
void MAG3110_setup(void);
unsigned char I2C_receive();
void I2C_transmit(unsigned char NoOfBytes,unsigned char *transmit_data);

unsigned char TXConfigData[4]={0x10,0x01,0x11,0x80};
unsigned char RegX1[1]={0x1};
unsigned char RegX2[1]={0x2};
unsigned char RegY1[1]={0x3};
unsigned char RegY2[1]={0x4};
unsigned char RegZ1[1]={0x5};
unsigned char RegZ2[1]={0x6};
unsigned char X1;
unsigned char X2;
unsigned char Y1;
unsigned char Y2;
unsigned char Z1;
unsigned char Z2;
int x;
int y;
int z;
unsigned char RXData;
unsigned char TXByteCtr;
unsigned char *TI_transmit_field;

void main(){
  WDTCTL = WDTPW + WDTHOLD;
  I2C_setup();
  _EINT();
  MAG3110_setup();
  while(1){
   I2C_transmit(1,RegX1);
   X1 = I2C_receive();
   I2C_transmit(1,RegX2);
   X2 = I2C_receive();
   I2C_transmit(1,RegY1);
   Y1 = I2C_receive();
   I2C_transmit(1,RegY2);
   Y2 = I2C_receive();
   I2C_transmit(1,RegZ1);
   Z1 = I2C_receive();
   I2C_transmit(1,RegZ2);
   Z2 = I2C_receive();
   x = (X1 << 8) | X2;
   y = (Y1 << 8) | Y2;
   z = (Z1 << 8) | Z2;
  }


}

void I2C_setup() {
  P3SEL |= 0x03;                            //  P3.0(UCB0_SDA), P3.1(UCB0_SCL)
  UCB0CTL1 |= UCSWRST;                      // reset enable
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // master + I2C mode + Sync
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            //use SMCLK + still reset
  UCB0BR0 = 4;                              // default SMCLK 1M/4 = 250KHz
  UCB0BR1 = 0;                              //
  UCB0I2CSA = 0x0E;                         // MAG3110 address 0x0E
  UCB0CTL1 &= ~UCSWRST;                     // reset clear
  UCB0IE |= UCTXIE + UCRXIE;             //TX and RX interrupt enabled
}

void MAG3110_setup(void){
    I2C_transmit(4,TXConfigData);
}


void I2C_transmit(unsigned char NoOfBytes, unsigned char *transmit_data){
  TXByteCtr = NoOfBytes;                            // never sending more than 1 byte
  TI_transmit_field = (unsigned char *)transmit_data;                   //
  while (UCB0CTL1 & UCTXSTP);               // wait untill stop condition is sent
  UCB0CTL1 |= UCTR + UCTXSTT;               // TX mode + start condition
  LPM0;
}

unsigned char I2C_receive(){
  while (UCB0CTL1 & UCTXSTP);               // Ensure stop condition got sent
  UCB0CTL1 &= ~UCTR;                        // clear tx condition
  UCB0CTL1 |= UCTXSTT;                      // I2C start condition
  while(UCB0CTL1 & UCTXSTT);                // Start condition sent?
  for(int i=0; i<10; i++);
  RXData = UCB0RXBUF;
  UCB0CTL1 |= UCTXSTP;                      // stop condition
  return RXData;
  //LPM0;
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB0IV,12))
  {
  case  0: break;                           // Vector  0: No interrupts
  case  2: break;                           // Vector  2: ALIFG
  case  4: break;                           // Vector  4: NACKIFG
  case  6: break;                           // Vector  6: STTIFG
  case  8: break;                           // Vector  8: STPIFG
  case 10:
  // RXData = UCB0RXBUF;                       // Get RX data
  // UCB0CTL1 |= UCTXSTP;                      // stop condition
  // UCB0IFG &= ~UCRXIFG;                  // Clear USCI_B0 TX int flag
  // LPM0_EXIT;
  break;
  case 12:                                  // Vector 12: TXIFG
    if (TXByteCtr)                          // Check TX byte counter
    {
      UCB0TXBUF = *TI_transmit_field++;                   // Load TX buffer
      TXByteCtr--;                          // Decrement TX byte counter
    }
    else
    {
      UCB0CTL1 |= UCTXSTP;                  // I2C stop condition
      UCB0IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
      LPM0_EXIT;                            // Exit LPM0
    }
  break;
  default: break;
  }
}
