#include <msp430f5529.h>
#include <math.h>
// for radio start
#include <msp430.h>
#include "msprf24.h"
#include "nrf_userconfig.h"
//#include "stdint.h"
// for radio end

void clock_setup();
void spi_setup();
void spi_com1();
void spi_com2();
void ADC_setup();
void UART_setup();

// for radio start
volatile unsigned int user;
// for radio end

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
//volatile unsigned int a,b,c,d,e,f,g,h,k1,k2;  // for debugging
volatile int x1, x2, y1, y2, z1, z2, mag1, mag2, mag_th_1, mag_th_2, count1, count2, count3, count4, count5, count6, count7, count8, count9, count10, count11, count12, count13, temp_data_1, temp_data_2, data_check_1, data_check_2;
int mag_rcnt_1[100];
int mag_rcnt_2[100];
volatile int mag1_1000[1000];
volatile int mag2_1000[1000];
volatile float speed, length, speed_th, length_th, dist_betwn_snsrs, sampling_rate;
volatile int store_1000_flag, highest_point_loc_1, highest_point_loc_2, ice_present_flag;

void main()
{
  WDTCTL = WDTPW + WDTHOLD;
  _EINT();

  // for radio start
  char addr[5];
  char buf[3];
  user = 0xFE;
  /* Initial values for nRF24L01+ library config variables */
    rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
    rf_addr_width      = 5;
    rf_speed_power     = RF24_SPEED_1MBPS | RF24_POWER_0DBM;
    rf_channel         = 120;
    msprf24_init();  // All RX pipes closed by default
    msprf24_set_pipe_packetsize(0, 3);
        msprf24_open_pipe(0, 1);  // Open pipe#0 with Enhanced ShockBurst enabled for receiving Auto-ACKs
            // Note: Pipe#0 is hardcoded in the transceiver hardware as the designated "pipe" for a TX node to receive
            // auto-ACKs.  This does not have to match the pipe# used on the RX side.

        // Transmit to 'rad01' (0x72 0x61 0x64 0x30 0x31)
        msprf24_standby();
        user = msprf24_current_state();
        addr[0] = 0xDE; addr[1] = 0xAD; addr[2] = 0xBE; addr[3] = 0xEF; addr[4] = 0x00;
        w_tx_addr(addr);
        w_rx_addr(0, addr);  // Pipe 0 receives auto-ack's, autoacks are sent back to the TX addr so the PTX node
                             // needs to listen to the TX addr on pipe#0 to receive them.
        // for radio end



  dist_betwn_snsrs = 0.125; // in m
  sampling_rate = 240;
  ice_present_flag = 0; // if there's ice, flag set to 1
  length_th = 4.1;  // 4.1m, length of compact car according us epa
  speed_th = 0.6; // 0.6 m/s, for demonstration, will be changed for later

  clock_setup();
  spi_setup();
  ADC_setup();
  UART_setup();
  j = 0;
  mag_th_1 = 0;
  mag_th_2 = 0;
  count1 = 0;   // count first 100 data to calculate threshold
  store_1000_flag = 0;
//  P8DIR |= BIT1;    // for debugging

  P1DIR |= BIT0;    // p1.0 led1 / warning
  P1OUT &= ~BIT0;   // warning off

  P1DIR &= ~ BIT1;  // switch 2, emulate ice
  P1REN |= BIT1;    // internal resistance enabled
  P1OUT |= BIT1;    // pull-up resistance
  P1IFG &= ~BIT1;   // ifg cleared
  P1IE |= BIT1;     // enable interrupt for switch 2

  P4DIR |= BIT7;    // p4.7 led2 / indicates ice
  P4OUT &= ~BIT7;   // no ice by default

  while(1)
  {
      spi_com1();
      spi_com2();
//      P8OUT ^= BIT1;    // to check sampling fr

      //    1 set of data is received for 2 magnetometer sensors
      x1 = (RXData_s1[0]*256)+RXData_s1[1];
      y1 = (RXData_s1[2]*256)+RXData_s1[3];
      z1 = (RXData_s1[4]*256)+RXData_s1[5];
      x2 = (RXData_s2[0]*256)+RXData_s2[1];
      y2 = (RXData_s2[2]*256)+RXData_s2[3];
      z2 = (RXData_s2[4]*256)+RXData_s2[5];
      mag1 = sqrt((x1*x1) + (y1*y1) + (z1*z1));
      mag2 = sqrt((x2*x2) + (y2*y2) + (z2*z2));

      if(count1<100)    // storing first 100 data and calculating threshold
          {
          mag_rcnt_1[count1] = mag1; // store 100 recent mag
          mag_rcnt_2[count1] = mag2;
          count1++;
          mag_th_1 += mag1;
          mag_th_2 += mag2;
          if(count1 == 100) // calculating threshold for first 100 data
                {
                    mag_th_1 = mag_th_1/100;
                    mag_th_2 = mag_th_2/100;
                }
          }

      else
      {
          for (count2=0;count2<99;count2++) // shifting 99 data points to the left to make room for most recent data at 100th place
          {
              mag_rcnt_1[count2] = mag_rcnt_1[count2+1];
              mag_rcnt_2[count2] = mag_rcnt_2[count2+1];
          }
          mag_rcnt_1[99] = mag1;    // add the most recent mag value to the array of recent 100 values
          mag_rcnt_2[99] = mag2;
          count3 = 0; // to check if last 10 data exceed threshold
          if (store_1000_flag == 0) // if this flag is 1, skip this stage and start storing
              {
              for (count11=90;count11<100;count11++)
          {
              if((mag_rcnt_1[count11]-mag_th_1) > 3 || (mag_rcnt_1[count11]-mag_th_1) < -3)
                  count3++;
          }
          if (count3 == 10)
          {
              store_1000_flag = 1;
              count4 = 100;
              for(count12=0;count12<100;count12++)  // storing first 100 data of 1000
              {
                  mag1_1000[count12] = mag_rcnt_1[count12];
                  mag2_1000[count12] = mag_rcnt_2[count12];
              }
          }
              }
      }

      if (store_1000_flag == 1 && count4<1000)
      {
          mag1_1000[count4] = mag1;
          mag2_1000[count4] = mag2;
          count4++;
      }

      if(count4 == 1000 && store_1000_flag == 1)    // 1000 data stored, time for calculation
      {
          P1OUT &= ~BIT0;   // warning sign off
          store_1000_flag = 0; // stop storing data and start calculations. will start checking recent 10 mag after calculations are done

// median filter for removing the spikes
          for(count10=1;count10<999;count10++)
          {
              if(mag1_1000[count10] > mag1_1000[count10-1] && mag1_1000[count10] > mag1_1000[count10+1])
                  mag1_1000[count10] = mag1_1000[count10+1];
              else if(mag1_1000[count10] < mag1_1000[count10-1] && mag1_1000[count10] < mag1_1000[count10+1])
                  mag1_1000[count10] = mag1_1000[count10+1];

              if(mag2_1000[count10] > mag2_1000[count10-1] && mag2_1000[count10] > mag2_1000[count10+1])
                  mag2_1000[count10] = mag2_1000[count10+1];
              else if(mag2_1000[count10] < mag2_1000[count10-1] && mag2_1000[count10] < mag2_1000[count10+1])
                  mag2_1000[count10] = mag2_1000[count10+1];
          }

//  moving avg filter. doesnt seem to be very useful

//          for(count6=0;count6<981;count6++) // moving avg filter, only valid first 900 data
//          {
//              temp_data_1 = 0;
//              temp_data_2 = 0;
//              for(count5=0;count5<20;count5++)
//              {
//                  temp_data_1 += mag1_1000[count6+count5];
//                  temp_data_2 += mag2_1000[count6+count5];
//              }
//              mag1_1000[count6] = temp_data_1/20;
//              mag2_1000[count6] = temp_data_2/20;
//          }

          // removing mag threshold so that squaring wont cause any prob, works like removing mean but dont have to calculate mean
          for(count8=0;count8<1000;count8++)
          {
              mag1_1000[count8] = mag1_1000[count8] - mag_th_1;
              mag2_1000[count8] = mag2_1000[count8] - mag_th_2;
              mag1_1000[count8] = mag1_1000[count8] * mag1_1000[count8];
              mag2_1000[count8] = mag2_1000[count8] * mag2_1000[count8];
          }

          // finding time difference between 2 sensors data
          data_check_1 = mag1_1000[0];
          highest_point_loc_1 = 0;
          data_check_2 = mag2_1000[0];
          highest_point_loc_2 = 0;
          for(count7=1;count7<1000;count7++)
          {
              if(mag1_1000[count7]>data_check_1)
              {
                  data_check_1 = mag1_1000[count7]; // highest point of the graph
                  highest_point_loc_1 = count7;  // index of the highest point
              }
              if(mag2_1000[count7]>data_check_2)
              {
                  data_check_2 = mag2_1000[count7];
                  highest_point_loc_2 = count7;
              }
          }
// get wrong index sometimes, need to check [fixed]


          speed = dist_betwn_snsrs/((highest_point_loc_2 - highest_point_loc_1)/sampling_rate);   // speed in m/s

// finding length. We know max point is "highest_point_loc_1", check when the mag becomes same as threshold after that.
// since we are removing threshold, need to find when mag becomes close to 0. checking 2 consecutive samples to be sure.
// stopping the for loop when we find that point
          for(count9=highest_point_loc_1;count9<999;count9++)
          {
              if(mag1_1000[count9]<10 && mag1_1000[count9+1]<10)
              {
                  length = (count9 - 89)/sampling_rate;    //89+20=109, 89 data before detecting car, 20 for moving avg filter(not using)
                  length = length * speed;  //   length in m
                  count9 = 1000;    // stopping the for loop right here
              }
          }

          speed_th = 0.6; // speed threshold reset fpr each vehicle

          if(ice_present_flag == 1) // if ice present, threshold speed reduced by 50%
              speed_th = 0.3; // if write speed_th/2, will divide by 2 in every loop
// next speed threshold calculation showing error. need to check
          //if(length > length_th)    // if length of the car is longer than avg
              //speed_th = speed_th - (speed_th * ((int(length - length_th)) * 0.1));   // 10% reduction in speed threshold for each meter increase in length from avg length

          if(speed > speed_th)
          {
              P1OUT |= BIT0;    // warning sign on, will remain on until another car detected

              // for radio start
              buf[0] = (int)(speed);    // sending speed
              if(distance<300)  // if the car is less than 300 cm away, lane 1, else lane 2
                  buf[1] = 1;
              else
                  buf[1] = 2;

              if(ice_present_flag == 1) // if ice on road, send 1, else 0
                  buf[2] = 1;
              else
                  buf[2] = 0;

              w_tx_payload(3, buf);
                      msprf24_activate_tx();
                      LPM0;

                      if (rf_irq & RF24_IRQ_FLAGGED) {
                          rf_irq &= ~RF24_IRQ_FLAGGED;

                          msprf24_get_irq_reason();
                          if (rf_irq & RF24_IRQ_TX){
                              //P1OUT &= ~BIT0; // Red LED off
                              //P4OUT |= BIT7;  // Green LED on
                          }
                          if (rf_irq & RF24_IRQ_TXFAILED){
                              //P4OUT &= ~BIT7; // Green LED off
                              //P1OUT |= BIT0;  // Red LED on
                          }

                          msprf24_irq_clear(rf_irq);
                          user = msprf24_get_last_retransmits();
                      }
                      // for radio end
          }

      }


      //////////////////////////////////////////////////////////

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

// Port 1 interrupt service routine

#pragma vector=PORT1_VECTOR

__interrupt void Port_1(void)

{

    switch( __even_in_range( P1IV, P1IV_P1IFG7 )) {

    case P1IV_P1IFG1:   // Pin 1.1 (switch 2)
        if(ice_present_flag == 0)
        {
            ice_present_flag = 1;   // ice present
            P4OUT |= BIT7;  // ice led on
        }
        else
        {
            ice_present_flag = 0;   // if pressed again, ice not present
            P4OUT &= ~BIT7; // ice led off
        }
        break;

    default:   _never_executed();


    }
}
