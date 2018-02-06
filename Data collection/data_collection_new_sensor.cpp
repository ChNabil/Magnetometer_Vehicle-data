#include <msp430f5529.h>

void clock_setup();
void spi_setup();
void spi_com1();

// unsigned char temp1;
// unsigned char temp2;
// unsigned int x;
// unsigned int y;
volatile unsigned char RXData_s1[6];  // received 6 bytes from sensor 1
volatile unsigned char RXData_s2[6];  // received 6 bytes from sensor 2
unsigned char *PRXData_s1; // pointer for sensor 1 data
unsigned char *PRXData_s2; // pointer for sensor 2 data
volatile unsigned int j;
volatile unsigned int i;
void main()
{
  WDTCTL = WDTPW + WDTHOLD;
  _EINT();
  // clock_setup();
  spi_setup();
  while(1)
  {
      spi_com1();
  }
}

void clock_setup(){
    P2DIR |= BIT2;    // check smclk, 1MHz default
    P2SEL |= BIT2;    // check smclk, 1MHz default
    P1DIR |= BIT0;    // check aclk, 32.8KHz default
    P1SEL |= BIT0;    // check aclk, 32.8KHz default
}

void spi_setup()
{
  P3SEL |= BIT0 + BIT1 + BIT2;  // USCB0SIMO, UCB0SOMI, UCB0CLK select
  P1DIR |= BIT2 + BIT3 + BIT4 + BIT5;  // p1.2 and 1.3 for reset, p1.4 and 1.5 for selecting slave
  P2DIR &= ~ BIT4; // DRDY pin sensor 1
  P2DIR &= ~ BIT5; // DRDY pin sensor 2
  P1OUT &= ~BIT2; // sensor 1 reset
  P1OUT &= ~BIT3; // sensor 2 reset
  UCB0CTL1 |= UCSWRST;  // reset
  UCB0CTL0 |= UCMST+UCSYNC+UCCKPL+UCMSB;  // master mode, synchronous, inactive clock high, msb first
  UCB0CTL1 |= UCSSEL_2; // using smclk
  UCB0CTL1 &= ~UCSWRST; // reset disabled
  UCB0IE |= UCRXIE; // rx interrupt
}

void spi_com1()
{
  P1OUT &= ~BIT4; // select sensor 1
  i=0;
  PRXData_s1 = (unsigned char *)RXData_s1;  // pointer for RXData_s1
  P1OUT |= BIT2; // sensor 1 reset
  P1OUT &= ~BIT2; // sensor 1 reset
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x01; // command to get x axis data
  LPM0;
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get x axis data
  LPM0;
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get x axis data
  LPM0;
  P1OUT &= ~BIT4; // select sensor 1
  P1OUT |= BIT2; // sensor 1 reset
  P1OUT &= ~BIT2; // sensor 1 reset
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x02; // command to get y axis data
  LPM0;
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get y axis data
  LPM0;
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get y axis data
  LPM0;
  P1OUT &= ~BIT4; // select sensor 1
  P1OUT |= BIT2; // sensor 1 reset
  P1OUT &= ~BIT2; // sensor 1 reset
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x03; // command to get z axis data
  LPM0;
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get z axis data
  LPM0;
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get z axis data
  LPM0;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB0IV,4))
  {
    case 0: break;
    case 2:
    i++;                               // Vector 2 - RXIFG
      while (!(UCB0IFG&UCTXIFG));           // USCIB0 TX buffer ready?
      while (!(P2IN&BIT4));           //
      if(i!=1 && i!=4 && i!=7)
          *PRXData_s1++ = UCB0RXBUF;               // Move RX data to address PRxData
      LPM0_EXIT;
      break;
    case 4: break;                          // Vector 4 - TXIFG
    default: break;
  }
}
