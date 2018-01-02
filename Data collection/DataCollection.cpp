#include <msp430f5529.h>
//#include <math.h>

void Clock_setup();   // using default SMCLK ~1MHz for now
void I2C_setup_s1();  // I2C setup for sensor 1
void I2C_setup_s2();  // I2C setup for sensor 2
void MAG3110_setup_s1(void);  // Sensor 1 configuration setup
void MAG3110_setup_s2(void);  // Sensor 2 configuration setup
void I2C_receive_s1();  // receive 6 bytes from sensor 1
void I2C_receive_s2();  // receive 6 bytes from sensor 2
void I2C_transmit_s1(unsigned char NoOfBytes,unsigned char *transmit_data);  // transmit data to sensor 1
void I2C_transmit_s2(unsigned char NoOfBytes,unsigned char *transmit_data);  // transmit data to sensor 2
void ADC_setup();
void UART_setup();

unsigned char TXConfigData[4]={0x10,0x01,0x11,0x80};  // same config for both sensors
unsigned char RegAddData[1]={0x1};  // same address for data in both sensors
int x_s1; // x value from sensor 1
int y_s1; // y value from sensor 1
int z_s1; // z value from sensor 1
int x_s2; // x value from sensor 2
int y_s2; // y value from sensor 2
int z_s2; // z value from sensor 2
int mag_s1;
int mag_s2;
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
char three_digit_flag;  // sending 3 digits of each bytes separately
char i; // for each 5 magnetometer data, 1 distance will be measured. cause magnetometer data fr is 80Hz, ultrasonic data fr 20Hz

void main(){
  WDTCTL = WDTPW + WDTHOLD;
  P2DIR &= ~BIT5; // will be used to check if new magnetmtr data are available
  //Clock_setup();
  I2C_setup_s1();
  //I2C_setup_s2();
  _EINT();
  MAG3110_setup_s1();
  //MAG3110_setup_s2();
  ADC_setup();
  UART_setup();
  i = 0;
  while(1){
    if(P2IN & BIT5){  // checking magntmtr int pin, so all 3 sensors data will have same sampling fr(80Hz)
      I2C_transmit_s1(1,RegAddData);
      I2C_receive_s1();
      //I2C_transmit_s2(1,RegAddData);
      //I2C_receive_s2();
      i++;
      if (i==5){
          ADC12IE = 0x01;   // enable adc interrupt for channel 0
          LPM0;
          i = 0;
      }
//      x_s1 = (RXData_s1[0] << 8) | RXData_s1[1];
//      y_s1 = (RXData_s1[2] << 8) | RXData_s1[3];
//      z_s1 = (RXData_s1[4] << 8) | RXData_s1[5];
//      x_s2 = (RXData_s2[0] << 8) | RXData_s2[1];
//      y_s2 = (RXData_s2[2] << 8) | RXData_s2[3];
//      z_s2 = (RXData_s2[4] << 8) | RXData_s2[5];
      // will do this in data processing, taking raw data (x,y,z) for now
      // mag_s1 = sqrt(powf(x_s1,2)+powf(y_s1,2)+powf(z_s1,2));  // calculate magnitude
      // mag_s1 = mag_s1*2000/65536; // convert magnitude into uT
      // mag_s2 = sqrt(powf(x_s2,2)+powf(y_s2,2)+powf(z_s2,2));  // calculate magnitude
      // mag_s2 = mag_s2*2000/65536; // convert magnitude into uT
      UART_sensor_flag = 1;  // sensor 1 is sending data
      UART_byte = 0;  // for first datum from sensor 1
      three_digit_flag = 1; // sending first digit
      UCA1IE |= UCTXIE; // Enable USCI_A1 TX interrupt
      LPM0;
      UART_sensor_flag = 2;  // sensor 2 is sending data
      UART_byte = 0;  // for first datum from sensor 2
      three_digit_flag = 1; // sending first digit
      UCA1IE |= UCTXIE; // Enable USCI_A1 TX interrupt
      LPM0;
      UART_sensor_flag = 3;  // sensor 3 (ultrasonic) is sending data
      three_digit_flag = 1; // sending first digit
      UCA1IE |= UCTXIE; // Enable USCI_A1 TX interrupt
      LPM0;
    }
  }
}

void Clock_setup(){
    P2DIR |= BIT2;    // check smclk, 1MHz default
    P2SEL |= BIT2;    // check smclk, 1MHz default
    P1DIR |= BIT0;    // check aclk, 32.8KHz default
    P1SEL |= BIT0;    // check aclk, 32.8KHz default
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

// Ref voltage is AVCC = 3.3v, range of sensor is 648cm, 648/2048=0.316
void ADC_setup(){
    P6SEL |= 0x01;                            // Enable A/D channel A0
//    REFCTL0 = REFMSTR + REFVSEL_0 + REFON;
    ADC12CTL0 = ADC12ON+ADC12SHT0_8+ADC12MSC;   // Turn on ADC12, set sampling time
    ADC12CTL1 = ADC12SSEL_3+ADC12SHP+ADC12CONSEQ_2;       // Use sampling timer, set mode
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
      if (three_digit_flag == 1){
        UCA1TXBUF = ((int) (RXData_s1[UART_byte]/100))+48;
        three_digit_flag++;
      }
        else if (three_digit_flag == 2){
          UCA1TXBUF = ((RXData_s1[UART_byte] - (((int) (RXData_s1[UART_byte]/100))*100))/10)+48;
          three_digit_flag++;
        }
        else if (three_digit_flag == 3){
          UCA1TXBUF = ((RXData_s1[UART_byte] - (((int) (RXData_s1[UART_byte]/100))*100))%10)+48;
          three_digit_flag = 1;
          UART_byte++;
        }
      if(UART_byte == 6)
      {
        UCA1IE &= ~UCTXIE;
        LPM0_EXIT;
      }
    }
    else if (UART_sensor_flag == 2){
      if (three_digit_flag == 1){
        UCA1TXBUF = ((int) (RXData_s2[UART_byte]/100))+48;
        three_digit_flag++;
      }
        else if (three_digit_flag == 2){
          UCA1TXBUF = (RXData_s2[UART_byte] - ((((int) (RXData_s2[UART_byte]/100))*100))/10)+48;
          three_digit_flag++;
        }
        else if (three_digit_flag == 3){
          UCA1TXBUF = ((RXData_s2[UART_byte] - (((int) (RXData_s2[UART_byte]/100))*100))%10)+48;
          three_digit_flag = 1;
          UART_byte++;
        }
      if(UART_byte == 6)
      {
          UCA1IE &= ~UCTXIE;
          LPM0_EXIT;
      }
    }
    else if (UART_sensor_flag == 3){
        if (three_digit_flag == 1){
           UCA1TXBUF = ((int) (distance/100))+48;
           three_digit_flag++;
         }
           else if (three_digit_flag == 2){
               temp = ((int) (distance/100))*100;
               temp = (distance - temp)/10;
               UCA1TXBUF = temp +48;
//             UCA1TXBUF = (distance - ((((int) (distance/100))*100))/10)+48;
             three_digit_flag++;
           }
           else if (three_digit_flag == 3){
             UCA1TXBUF = ((distance - (((int) (distance/100))*100))%10)+48;
             three_digit_flag++;
           }
           else if (three_digit_flag == 4){
               UCA1TXBUF = '\n'; // newline
               three_digit_flag++;
               }
           else if (three_digit_flag == 5){
               UCA1TXBUF = '\r'; // starting position of newline
               UCA1IE &= ~UCTXIE;
               LPM0_EXIT;
               }
    }
  break;                             // Vector 4 - TXIFG
  default: break;
  }
}