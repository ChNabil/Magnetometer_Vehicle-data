#include<msp430f5529.h>

//void Clock_setup(void);
void I2C_setup_s1();  // I2C setup for sensor 1
void I2C_setup_s2();  // I2C setup for sensor 2
void MAG3110_setup_s1(void);  // Sensor 1 configuration setup
void MAG3110_setup_s2(void);  // Sensor 2 configuration setup
void I2C_receive_s1();  // receive 6 bytes from sensor 1
void I2C_receive_s2();  // receive 6 bytes from sensor 2
void I2C_transmit_s1(unsigned char NoOfBytes,unsigned char *transmit_data);  // transmit data to sensor 1
void I2C_transmit_s2(unsigned char NoOfBytes,unsigned char *transmit_data);  // transmit data to sensor 2

unsigned char TXConfigData[4]={0x10,0x01,0x11,0x80};  // same config for both sensors
unsigned char RegAddData[1]={0x1};  // same address for data in both sensors
int x_s1; // x value from sensor 1
int y_s1; // y value from sensor 1
int z_s1; // z value from sensor 1
int x_s2; // x value from sensor 2
int y_s2; // y value from sensor 2
int z_s2; // z value from sensor 2
volatile unsigned char RXData_s1[6];  // received 6 bytes from sensor 1
volatile unsigned char RXData_s2[6];  // received 6 bytes from sensor 2
unsigned char *PRXData_s1; // pointer for sensor 1 data
unsigned char *PRXData_s2; // pointer for sensor 2 data
unsigned char RXByteCtr;  // same for both sensors
unsigned char TXByteCtr;  // same for both sensors
unsigned char *TI_transmit_field; // same for both sensors

void main(){
  WDTCTL = WDTPW + WDTHOLD;
  I2C_setup_s1();
  I2C_setup_s2();
  _EINT();
  MAG3110_setup_s1();
  MAG3110_setup_s2();
  while(1){
   I2C_transmit_s1(1,RegAddData);
   I2C_receive_s1();
   I2C_transmit_s2(1,RegAddData);
   I2C_receive_s2();
    x_s1 = (RXData_s1[0] << 8) | RXData_s1[1];
    y_s1 = (RXData_s1[2] << 8) | RXData_s1[3];
    z_s1 = (RXData_s1[4] << 8) | RXData_s1[5];
    x_s2 = (RXData_s2[0] << 8) | RXData_s2[1];
    y_s2 = (RXData_s2[2] << 8) | RXData_s2[3];
    z_s2 = (RXData_s2[4] << 8) | RXData_s2[5];
  }


}

void I2C_setup_s1() {
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


void I2C_setup_s2() {
  P4SEL |= 0x06;                            //  P4.1(UCB1_SDA), P4.2(UCB1_SCL)
  UCB1CTL1 |= UCSWRST;                      // reset enable
  UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;     // master + I2C mode + Sync
  UCB1CTL1 = UCSSEL_2 + UCSWRST;            //use SMCLK + still reset
  UCB1BR0 = 4;                              // default SMCLK 1M/4 = 250KHz
  UCB1BR1 = 0;                              //
  UCB1I2CSA = 0x0E;                         // MAG3110 address 0x0E
  UCB1CTL1 &= ~UCSWRST;                     // reset clear
  UCB1IE |= UCTXIE + UCRXIE;             //TX and RX interrupt enabled
}


void MAG3110_setup_s1(void){
    I2C_transmit_s1(4,TXConfigData);
}


void MAG3110_setup_s2(void){
    I2C_transmit_s2(4,TXConfigData);
}


void I2C_transmit_s1(unsigned char NoOfBytes, unsigned char *transmit_data){
  TXByteCtr = NoOfBytes;                            // never sending more than 1 byte
  TI_transmit_field = (unsigned char *)transmit_data;                   //
  while (UCB0CTL1 & UCTXSTP);               // wait untill stop condition is sent
  UCB0CTL1 |= UCTR + UCTXSTT;               // TX mode + start condition
  LPM0;
}


void I2C_transmit_s2(unsigned char NoOfBytes, unsigned char *transmit_data){
  TXByteCtr = NoOfBytes;                            // never sending more than 1 byte
  TI_transmit_field = (unsigned char *)transmit_data;                   //
  while (UCB1CTL1 & UCTXSTP);               // wait untill stop condition is sent
  UCB1CTL1 |= UCTR + UCTXSTT;               // TX mode + start condition
  LPM0;
}


void I2C_receive_s1(){
  PRXData_s1 = (unsigned char *)RXData_s1;
  RXByteCtr = 6;
  while (UCB0CTL1 & UCTXSTP);               // Ensure stop condition got sent
  UCB0CTL1 &= ~UCTR;                        // clear tx condition
  UCB0CTL1 |= UCTXSTT;                      // I2C start condition
  LPM0;
}


void I2C_receive_s2(){
  PRXData_s2 = (unsigned char *)RXData_s2;
  RXByteCtr = 6;
  while (UCB1CTL1 & UCTXSTP);               // Ensure stop condition got sent
  UCB1CTL1 &= ~UCTR;                        // clear tx condition
  UCB1CTL1 |= UCTXSTT;                      // I2C start condition
  LPM0;
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
  RXByteCtr--;                            // Decrement RX byte counter
  if (RXByteCtr)
  {
    *PRXData_s1++ = UCB0RXBUF;               // Move RX data to address PRxData
    if (RXByteCtr == 1)                   // Only one byte left?
      UCB0CTL1 |= UCTXSTP;                // Generate I2C stop condition
  }
  else
  {
    *PRXData_s1 = UCB0RXBUF;                 // Move final RX data to PRxData
    LPM0_EXIT; // Exit active CPU
  }
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


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B1_VECTOR))) USCI_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB1IV,12))
  {
  case  0: break;                           // Vector  0: No interrupts
  case  2: break;                           // Vector  2: ALIFG
  case  4: break;                           // Vector  4: NACKIFG
  case  6: break;                           // Vector  6: STTIFG
  case  8: break;                           // Vector  8: STPIFG
  case 10:
  RXByteCtr--;                            // Decrement RX byte counter
  if (RXByteCtr)
  {
    *PRXData_s2++ = UCB1RXBUF;               // Move RX data to address PRxData
    if (RXByteCtr == 1)                   // Only one byte left?
      UCB1CTL1 |= UCTXSTP;                // Generate I2C stop condition
  }
  else
  {
    *PRXData_s2 = UCB1RXBUF;                 // Move final RX data to PRxData
    LPM0_EXIT; // Exit active CPU
  }
  break;
  case 12:                                  // Vector 12: TXIFG
    if (TXByteCtr)                          // Check TX byte counter
    {
      UCB1TXBUF = *TI_transmit_field++;                   // Load TX buffer
      TXByteCtr--;                          // Decrement TX byte counter
    }
    else
    {
      UCB1CTL1 |= UCTXSTP;                  // I2C stop condition
      UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
      LPM0_EXIT;                            // Exit LPM0
    }
  break;
  default: break;
  }
}
