#include <msp430f5529.h>

void clock_setup();
void spi_setup();
void spi_com1();
void spi_com2();
void ADC_setup();
void UART_setup();

volatile unsigned char RXData_s1[6];  // received 6 bytes from sensor 1
volatile unsigned char RXData_s2[6];  // received 6 bytes from sensor 2
volatile unsigned int distance;  // distance in cm
int temp;
unsigned char *PRXData_s1; // pointer for sensor 1 data
unsigned char *PRXData_s2; // pointer for sensor 2 data
unsigned char RXByteCtr;  // same for both sensors
unsigned char TXByteCtr;  // same for both sensors
unsigned char *TI_transmit_field; // same for both sensors
char UART_byte; // number of byte to be sent
char UART_sensor_flag;  // which sensor is sending data via uart
char j,j1; // for each 100 magnetometer data, 1 distance will be measured. cause magnetometer data fr is 2000Hz, ultrasonic data fr 20Hz
int sensor_select; // received data from which sensor
volatile unsigned int i;
volatile unsigned int a,b,c,d,e,f,g,h,k1,k2;  // for debugging

void main()
{
  WDTCTL = WDTPW + WDTHOLD;
  _EINT();
  clock_setup();
  spi_setup();
  ADC_setup();
  UART_setup();
  j = 0;
//  P8DIR |= BIT1;    // for debugging

  while(1)
  {
      spi_com1();
      spi_com2();
      //      distance = 85;  // for debugging
      j++;
//      P8OUT ^= BIT1;    // for debugging
      if (j==12){
                ADC12IE = 0x01;   // enable adc interrupt for channel 0
                LPM0;
                j = 0;
            }
//      a=TA0R;
            UART_sensor_flag = 1;  // sensor 1 is sending data
            UART_byte = 0;  // for first datum from sensor 1
            //three_digit_flag = 1; // sending first digit
            UCA1IE |= UCTXIE; // Enable USCI_A1 TX interrupt
            LPM0;
            UART_sensor_flag = 2;  // sensor 2 is sending data
            UART_byte = 0;  // for first datum from sensor 2
            //three_digit_flag = 1; // sending first digit
            UCA1IE |= UCTXIE; // Enable USCI_A1 TX interrupt
            LPM0;
            UART_sensor_flag = 3;  // sensor 3 (ultrasonic) is sending data
            UART_byte = 0;
            //three_digit_flag = 1; // sending first digit
            UCA1IE |= UCTXIE; // Enable USCI_A1 TX interrupt
            LPM0;
//            b=TA0R;
  }
}

void clock_setup(){
    UCSCTL3 = SELREF_2;
    UCSCTL4 = SELA_2 + SELS_4 + SELM_3;
    UCSCTL0 = 0x0000;
    UCSCTL1 = DCORSEL_5;
    UCSCTL2 |= FLLD_1 + 249;
    UCSCTL5 |= DIVS__8;
//    P2DIR |= BIT2;    // check smclk, 1MHz default
//    P2SEL |= BIT2;    // check smclk, 1MHz default
//    P1DIR |= BIT0;    // check aclk, 32.8KHz default
//    P1SEL |= BIT0;    // check aclk, 32.8KHz default
//    P7DIR |= BIT7;    // check mclk, 1MHz default/ 16.8MHz
//    P7SEL |= BIT7;    // check mclk, 1MHz default
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
//  P8OUT ^= BIT1;    // for debugging
//    TA0CCR0 = 50000;    // for debugging
//    TA0CTL = TASSEL_2 + MC_1 + TACLR; // for debugging
  sensor_select = 1;
  P1OUT &= ~BIT4; // select sensor 1
  i=0;
  PRXData_s1 = (unsigned char *)RXData_s1;  // pointer for RXData_s1
  P1OUT |= BIT2; // sensor 1 reset
  P1OUT &= ~BIT2; // sensor 1 reset
  // sending 1 command byte followed by 2 dummy byte to receive 2 byte data for each axis
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 1; // command to get x axis data
  LPM0;
//  a=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get x axis data
  LPM0;
//  b=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get x axis data
  LPM0;
//  c=TA0R;   // for debugging
  P1OUT &= ~BIT4; // select sensor 1
  P1OUT |= BIT2; // sensor 1 reset
  P1OUT &= ~BIT2; // sensor 1 reset
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 2; // command to get y axis data
  LPM0;
//  d=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get y axis data
  LPM0;
//  e=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get y axis data
  LPM0;
//  f=TA0R;   // for debugging
  P1OUT &= ~BIT4; // select sensor 1
  P1OUT |= BIT2; // sensor 1 reset
  P1OUT &= ~BIT2; // sensor 1 reset
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 3; // command to get z axis data
  LPM0;
//  g=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get z axis data
  LPM0;
//  h=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get z axis data
  LPM0;
  P1OUT |= BIT4;    // sensor 1 inactive
//  k1=TA0R;   // for debugging
}

void spi_com2()
{
//  P8OUT ^= BIT1;    // for debugging
//    TA0CCR0 = 50000;    // for debugging
//      TA0CTL = TASSEL_2 + MC_1 + TACLR; // for debugging
  sensor_select = 2;
  P1OUT &= ~BIT5; // select sensor 2
  i=0;
  PRXData_s2 = (unsigned char *)RXData_s2;  // pointer for RXData_s2
  P1OUT |= BIT3; // sensor 2 reset
  P1OUT &= ~BIT3; // sensor 2 reset
  // sending 1 command byte followed by 2 dummy byte to receive 2 byte data for each axis
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 1; // command to get x axis data
  LPM0;
//  a=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get x axis data
  LPM0;
//  b=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get x axis data
  LPM0;
//  c=TA0R;   // for debugging
  P1OUT &= ~BIT5; // select sensor 2
  P1OUT |= BIT3; // sensor 2 reset
  P1OUT &= ~BIT3; // sensor 2 reset
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 2; // command to get y axis data
  LPM0;
//  d=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get y axis data
  LPM0;
//  e=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get y axis data
  LPM0;
//  f=TA0R;   // for debugging
  P1OUT &= ~BIT5; // select sensor 1
  P1OUT |= BIT3; // sensor 1 reset
  P1OUT &= ~BIT3; // sensor 1 reset
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 3; // command to get z axis data
  LPM0;
//  g=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get z axis data
  LPM0;
//  h=TA0R;   // for debugging
  while (!(UCB0IFG&UCTXIFG)); // USCIB0 TX buffer ready?
  UCB0TXBUF = 0x00; // command to get z axis data
  LPM0;
  P1OUT |= BIT5;    // sensor 2 inactive
//  k2=TA0R;   // for debugging
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
    case 2: // Vector 2 - RXIFG
//      for(j1=1;j1<50;j1++);    // without this delay, sensor 2 data skips one bit, need to check, this is the min delay that works
      while (!(UCB0IFG&UCTXIFG));           // USCIB0 TX buffer ready?
      if(i!=0 && i!=3 && i!=6)  // data received while sending command byte is ignored, storing data received while sending dummy byte
      {
          if(sensor_select == 1)    // checking which sensor is active to determine where to store the data
              *PRXData_s1++ = UCB0RXBUF;               // Move RX data to address PRxData
          else
              *PRXData_s2++ = UCB0RXBUF;               // Move RX data to address PRxData
      }
      else  // wait for new data only after sending command byte
          {
          if(sensor_select == 1)
              while (!(P2IN&BIT4));           // check DRDY pin if new data is ready in sensor 1
          else
              while (!(P2IN&BIT5));           // check DRDY pin if new data is ready in sensor 2
          }
      LPM0_EXIT;
      i++;
      break;
    case 4: break;                          // Vector 4 - TXIFG
    default: break;
  }
}

void ADC_setup(){
    P6SEL |= 0x01;                            // Enable A/D channel A0
//    REFCTL0 = REFMSTR + REFVSEL_0 + REFON;
    ADC12CTL0 = ADC12ON+ADC12SHT0_8+ADC12MSC;   // Turn on ADC12, set sampling time
    ADC12CTL1 = ADC12SSEL_3+ADC12SHP+ADC12CONSEQ_2;       // Use sampling timer, set mode, smclk
//    ADC12IE = 0x01;                           // Enable ADC12IFG.0
//    ADC12MCTL0 = ADC12SREF_1;
      ADC12CTL0 |= ADC12ENC;                    // Enable conversions
      ADC12CTL0 |= ADC12SC;                     // Start conversion
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC12_VECTOR))) ADC12ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(ADC12IV,34))
  {
  case  0: break;                           // Vector  0:  No interrupt
  case  2: break;                           // Vector  2:  ADC overflow
  case  4: break;                           // Vector  4:  ADC timing overflow
  case  6:                                  // Vector  6:  ADC12IFG0
    distance = ADC12MEM0;             // Move results
    distance = distance *0.316;   // convert into cm
    ADC12IE = 0x00; // disable interrupt for adc channel 0
    LPM0_EXIT;
    break;
  case  8: break;                           // Vector  8:  ADC12IFG1
  case 10: break;                           // Vector 10:  ADC12IFG2
  case 12: break;                           // Vector 12:  ADC12IFG3
  case 14: break;                           // Vector 14:  ADC12IFG4
  case 16: break;                           // Vector 16:  ADC12IFG5
  case 18: break;                           // Vector 18:  ADC12IFG6
  case 20: break;                           // Vector 20:  ADC12IFG7
  case 22: break;                           // Vector 22:  ADC12IFG8
  case 24: break;                           // Vector 24:  ADC12IFG9
  case 26: break;                           // Vector 26:  ADC12IFG10
  case 28: break;                           // Vector 28:  ADC12IFG11
  case 30: break;                           // Vector 30:  ADC12IFG12
  case 32: break;                           // Vector 32:  ADC12IFG13
  case 34: break;                           // Vector 34:  ADC12IFG14
  default: break;
  }
}



void UART_setup(){
  P4SEL |= BIT5+BIT4; // UCA1TXD and UCA1RXD
  UCA1CTL1 |= UCSWRST;  // reset enabled
  UCA1CTL1 |= UCSSEL_2; // SMCLK
  UCA1BR0 = 9;  // 1MHz/9 = 115200
  UCA1BR1 = 0;  // 1MHz/9 = 115200
  UCA1CTL1 &= ~UCSWRST; // reset disabled
}


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA1IV,4))
  {
  case 0:break;
  case 2:break;
  case 4:
    if (UART_sensor_flag == 1){
        UCA1TXBUF = RXData_s1[UART_byte];
        UART_byte++;
      if(UART_byte == 6)
      {
        UCA1IE &= ~UCTXIE;
        LPM0_EXIT;
      }
    }
    else if (UART_sensor_flag == 2){
        UCA1TXBUF = RXData_s2[UART_byte];
        UART_byte++;
      if(UART_byte == 6)
      {
          UCA1IE &= ~UCTXIE;
          LPM0_EXIT;
      }
    }
    else if (UART_sensor_flag == 3){
      if (UART_byte == 0)
           UCA1TXBUF = (int)(distance/256);
           else if (UART_byte == 1)
           {
                UCA1TXBUF = distance - (256*((int)(distance/256)));
                  //UCA1IE &= ~UCTXIE;
                  //LPM0_EXIT;
                }
                else if (UART_byte == 2)
                {
                     UCA1TXBUF = 0x7F;  // sending 2 extra dummy bytes for formating in terminal app
                       //UCA1IE &= ~UCTXIE;
                       //LPM0_EXIT;
                     }
                     else if (UART_byte == 3)
                     {
                          UCA1TXBUF = 0xFF; // sending 2 extra dummy bytes for formating in terminal app
                            UCA1IE &= ~UCTXIE;
                            LPM0_EXIT;
                          }
                UART_byte++;
       }
  break;                             // Vector 4 - TXIFG
  default: break;
  }
}
