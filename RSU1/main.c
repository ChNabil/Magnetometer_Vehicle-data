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
void spi_setup_acclmtr();
void spi_com_acclmtr();
void acclmtr_config();
void radio_setup();
void activate_rx_mode();
void send_msg();
void rcv_msg();

// for radio start
volatile unsigned int user;
// for radio end

unsigned char RXData_s1[6];  // received 6 bytes from sensor 1
unsigned char RXData_s2[6];  // received 6 bytes from sensor 2
unsigned int distance;  // distance in cm
//int temp;
unsigned char *PRXData_s1; // pointer for sensor 1 data
unsigned char *PRXData_s2; // pointer for sensor 2 data
unsigned char RXByteCtr;  // same for both sensors
unsigned char TXByteCtr;  // same for both sensors
unsigned char *TI_transmit_field; // same for both sensors
char UART_byte; // number of byte to be sent
char UART_sensor_flag;  // which sensor is sending data via uart
char j,j1; // for each 100 magnetometer data, 1 distance will be measured. cause magnetometer data fr is 2000Hz, ultrasonic data fr 20Hz
int sensor_select; // received data from which sensor
unsigned int i, length_int, speed_int;
int size;
long xcorr_result;
long temp = 0;
char mag2_dtct_flag;
int ii, lag;
int peak = 0;
int index = 0;
int indexx = 0;
//volatile unsigned int a,b,c,d,e,f,g,h,k1,k2;  // for debugging
volatile int x1, x2, y1, y2, z1, z2, mag1, mag2, mag_th_1, mag_th_2, count1, count2, count3, count4, count5, count6, count7, count8, count9, count10, count11, count12, count13, temp_data_1, temp_data_2, data_check_1, data_check_2;
int mag_rcnt_1[100];
int mag_rcnt_2[100];
int mag1_1000[1000];
int mag2_1000[1000];
float speed, speed1, length, speed_th, length_th, dist_betwn_snsrs, sampling_rate;
int store_1000_flag, highest_point_loc_1, highest_point_loc_2, ice_present_flag;
unsigned char acclmtr_data[2];  // received 2 bytes from acclmtr z axis
unsigned char *Pacclmtr_data; // pointer for acclmtr data
int z_acclmtr[1000];
char acclmtr_config_flag;
char i_acclmtr;
// for radio start
char addr_tx[5];
char addr_rx[5];
//char addr_rx3[5];
char buf_s[14];//sent msg 14 Bytes. See "Communication V2I I2X.docx" for details
char buf_r[14];//rcvd msg 14 Bytes. See "Communication V2I I2X.docx" for details
char sender_add = 1;
char road_no = 1;
char rcvd_msg_flag;
int temp_id;    // temp ID, will increase this every time a vehicle is detected

void main()
{
  WDTCTL = WDTPW + WDTHOLD;
  _EINT();

  // to check sampling rate
  P8DIR |= BIT1;
  user = 0xFE;  // for radio

  P6DIR |= BIT5;    // for checking processing time
  P6OUT &= ~BIT5;

  radio_setup();
  activate_rx_mode();   // start radio in rx mode, change to tx mode when needed
  rcvd_msg_flag = 0; // no msg rcvd yet

  dist_betwn_snsrs = 0.23; // in m
  sampling_rate = 200;  // increased from 200 after stopped sending raw data through UART
  ice_present_flag = 0; // if there's ice, flag set to 1
  length_th = 4.1;  // 4.1m, length of compact car according us epa
  speed_th = 0.6; // 0.6 m/s, for demonstration, will be changed for later

  clock_setup();
  spi_setup();
  ADC_setup();
  UART_setup();
  spi_setup_acclmtr();
  acclmtr_config();
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

  temp_id = 8968; // temp ID, will increase this every time a vehicle is detected
  mag2_dtct_flag = 0;

  while(1)
  {
      spi_com1();
      spi_com2();
      spi_com_acclmtr();
      P8OUT ^= BIT1;    // to check sampling fr

      rcv_msg();    // keep checking for rcvd msg.  take action when msg rcvd
      if(rcvd_msg_flag == 1)  // msg rcvd. see if any action needed
      {
          if(buf_r[2] == 64)    // type 1 msg rcvd
          {
              buf_s[2] = 128;   // will send type 2 msg
              buf_s[6] = buf_r[6];  // copy info rcvd from vhcl to the msg to be sent
              buf_s[7] = buf_r[7];  // copy info rcvd from vhcl to the msg to be sent
              buf_s[8] = buf_r[8];  // copy info rcvd from vhcl to the msg to be sent
              buf_s[10] = buf_r[10];    // copy info rcvd from vhcl to the msg to be sent
              buf_s[11] = buf_r[11];    // copy the random ID vehicle sent to combine with the ID RSU sent in msg 0
              send_msg();
              activate_rx_mode();
          }
          rcvd_msg_flag = 0;
      }

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

          if (count1 != 0)  // sometimes first data is garbage, check why
          {
          mag_th_1 += mag1;
          mag_th_2 += mag2;
          }
          count1++;
          if(count1 == 100) // calculating threshold for first 100 data
                {
              mag_rcnt_1[0] = mag_rcnt_1[1]; // sometimes first data is garbage, check why
              mag_rcnt_2[0] = mag_rcnt_2[1]; // sometimes first data is garbage, check why
              mag_th_1 = mag_th_1;
              mag_th_2 = mag_th_2;
                    mag_th_1 = mag_th_1/99;
                    mag_th_2 = mag_th_2/99;
                }
          }

      else
      {
          for (count2=90;count2<99;count2++) // shifting 99 data points to the left to make room for most recent data at 100th place
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
              if((mag_rcnt_1[count11]-mag_th_1) > 3)
                  count3++;
          }
          if (count3 == 10)
          {
              store_1000_flag = 1;
              mag2_dtct_flag = 1;
              count4 = 100;
              for(count12=0;count12<100;count12++)  // storing first 100 data of 1000
              {
                  mag1_1000[count12] = mag_rcnt_1[count12];
                  mag2_1000[count12] = mag_rcnt_2[count12];
                  //only storing acclmtr data when vehicle is detected by mgntmtr.
                  //for +-16 g range, RX_data/3=accelaration in m/s^2. (RX_data/32)*9.8=RX_data/3.
                  z_acclmtr[count12] = ((acclmtr_data[1]*256)+acclmtr_data[0])/3; // acclmtr data z axis
              }
          }
              }
      }

      if (store_1000_flag == 1 && count4<1000)
      {
          mag1_1000[count4] = mag1;
          mag2_1000[count4] = mag2;
          //only storing acclmtr data when vehicle is detected by mgntmtr.
          //for +-16 g range, RX_data/3=accelaration in m/s^2. (RX_data/32)*9.8=RX_data/3.
          z_acclmtr[count4] = ((acclmtr_data[1]*256)+acclmtr_data[0])/3; // acclmtr data z axis

      // checking when 2nd magnetometer detects the vehicle

  if(mag2_dtct_flag == 1)   // stop checking once vehicle dtctd
  {
      count3 = 0; // to check if last 10 data exceed threshold
          for (count11=0;count11<10;count11++)
      {
          if((mag2_1000[count4-count11]-mag_th_2) > 3)
              count3++;
      }
      if (count3 == 10)
      {
          mag2_dtct_flag = 0;   // stop checking, already detected the vehicle
        indexx = count4-99;// bcz 100th sample is where mag1 detected the vehicles
      }
   }
  count4++;
      }
      if(count4 == 1000 && store_1000_flag == 1)    // 1000 data stored, time for calculation
      {
          P6OUT |= BIT5;
          P1OUT &= ~BIT0;   // warning sign off
          store_1000_flag = 0; // stop storing data and start calculations. will start checking recent 10 mag after calculations are done

// median filter for removing the spikes
   /*       for(count10=1;count10<999;count10++)
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
*/
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

          // removing mag threshold for cross-correlation
          for(count8=0;count8<1000;count8++)
          {
              mag1_1000[count8] = mag1_1000[count8] - mag_th_1;
              mag2_1000[count8] = mag2_1000[count8] - mag_th_2;
            //  mag1_1000[count8] = mag1_1000[count8] * mag1_1000[count8];
            //  mag2_1000[count8] = mag2_1000[count8] * mag2_1000[count8];
          }

          // finding time difference between 2 sensors data
    /*      data_check_1 = mag1_1000[0];
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
          }*/ // get wrong index sometimes, need to check [fixed]
          // cross correlation to find time difference between 2 sensors
          size = 1000;
// takes too much time (208.5 ms). try another method
          xcorr_result = 0;
          for(lag=0; lag<size; lag++)
          {
            temp = 0;
            for(ii=0; ii<size-lag; ii++)
            {
              temp = temp + (mag1_1000[ii] * mag2_1000[ii+lag]);
            }
            if(temp >= xcorr_result)
                        {
                          xcorr_result = temp;
                          index = lag;
                        }
          }
//          peak = xcorr_result[0];
//          for(ii=0; ii<size; ii++)
//          {
//            if(xcorr_result[ii] >= peak)
//            {
//              peak = xcorr_result[ii];
//              index = ii;
//            }
//          }
          //

          speed = dist_betwn_snsrs/(index/sampling_rate);   // speed in m/s (xcorr)
          speed = speed * 3.6;  // in Km/h
          speed_int = speed;    // only using for UART checking, will remove in final sw

          speed1 = dist_betwn_snsrs/(indexx/sampling_rate);   // speed in m/s (not xcorr)
          speed1 = speed1 * 3.6;    // in Km/h

// finding length. We know max point is "highest_point_loc_1", check when the mag becomes same as threshold after that.
// since we are removing threshold, need to find when mag becomes close to 0. checking 2 consecutive samples to be sure.
// stopping the for loop when we find that point
          for(count9=100;count9<997;count9++)
          {
              if(mag1_1000[count9]<6 && mag1_1000[count9+1]<3 && mag1_1000[count9+2]<6)
              {
                  length = (count9 - 89)/sampling_rate;    //89+20=109, 89 data before detecting car, 20 for moving avg filter(not using)
                  length = (length * speed)/3.6;  //   length in m
                  length_int = (length * 100);  //   length in cm, will comment out this line later
                  count9 = 1000;    // stopping the for loop right here
              }
          }

          speed_th = 4; // speed threshold reset fpr each vehicle

          if(ice_present_flag == 1) // if ice present, threshold speed reduced by 50%
              speed_th = 2; // if write speed_th/2, will divide by 2 in every loop
// next speed threshold calculation showing error. need to check
          //if(length > length_th)    // if length of the car is longer than avg
              //speed_th = speed_th - (speed_th * ((int(length - length_th)) * 0.1));   // 10% reduction in speed threshold for each meter increase in length from avg length

//          if(speed > 0) // sends message whenever detects vehicle
//          {
              // for radio start
          /* ignoring for demonstration
              buf_s[0] = sender_add;
              buf_s[1] = road_no;
              buf_s[2] = (int)(speed);    // sending speed
              if(distance<300)  // if the car is less than 300 cm away, lane 1, else lane 2
                  buf_s[3] = 1;
              else
                  buf_s[3] = 2;

              if(ice_present_flag == 0) // if ice on road, send 1, else 0
                  buf_s[4] = 0;
              else
                  buf_s[4] = 1;

              if(speed > speed_th)
              {
                  buf_s[5] = 1; // warning
                  P1OUT |= BIT0;    // warning sign on, will remain on until another car detected
              }
              else
                  buf_s[5] = 0;
    */
          if(speed > speed_th)
                        {
                            P1OUT |= BIT0;    // warning sign on, will remain on until another car detected
                        }
          P6OUT &= ~BIT5;
          // for demonstration only
          buf_s[0] = 1; // intersection add 0, sender add 1
          buf_s[1] = 0x14;  // road number 1, total road at intersection 4
          buf_s[2] = 0; // reserved
          buf_s[3] = speed; // sending the calculated speed
//          buf_s[3] = 0x32;  // speed 50Mph
          buf_s[4] = 0x52;  // lane 2, length 4m, class 2
          buf_s[5] = 0;  // reserved
          buf_s[6] = 0; // vehicle will send this
          buf_s[7] = 0; // vehicle will send this
          buf_s[8] = 0; // vehicle will send this
          buf_s[9] = 0x10;  // warning low visibility, no speeding warning
          if (speed > speed_th)
              buf_s[9] = buf_s[9] | 0x80;   // setting speeding warning bit if applicable
          buf_s[10] = 0;    // reserved
          buf_s[11] = 0;    // temp ID, will rcv from vchl in type 1 msg
          temp_id++;    // temp ID, will increase this every time a vehicle is detected
          buf_s[12] = temp_id/256;    // temp ID
          buf_s[13] = temp_id%256; // temp ID

          //end
              send_msg();
              activate_rx_mode();   // after sending msg, keep radio in rx mode and keep checking for rcvd msg until a new car is detected
//          }                       // will only go to tx mode when a new car is detected or a msg is rcvd (dpndng on the msg)

/*
              while (!(UCA1IFG&UCTXIFG));             // USCI_A1 TX buffer ready?
              UCA1TXBUF = speed_int;
              while (!(UCA1IFG&UCTXIFG));             // USCI_A1 TX buffer ready?
  */            UCA1TXBUF = length_int;

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
      //    dont need to send raw data anymore

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
      //    not sending raw data anymore

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
           //UCA1TXBUF = (int)(distance/256); // sending acclmtr data instead distance
           UCA1TXBUF = ((acclmtr_data[1]*256)+acclmtr_data[0])/3;
           else if (UART_byte == 1)
           {
                //UCA1TXBUF = distance - (256*((int)(distance/256)));
               UCA1TXBUF = index;
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

void spi_setup_acclmtr()
{
  P4SEL |= BIT1 + BIT2 + BIT3;  // USCB1SIMO, UCB1SOMI, UCB1CLK select
  P8DIR |= BIT2; //8.2 for selecting slave
  UCB1CTL1 |= UCSWRST;  // reset
  UCB1CTL0 |= UCMST+UCSYNC+UCCKPL+UCMSB;  // master mode, synchronous, inactive clock high, msb first
  UCB1CTL1 |= UCSSEL_2; // using smclk
  UCB1CTL1 &= ~UCSWRST; // reset disabled
  UCB1IE |= UCRXIE; // rx interrupt
}

void acclmtr_config()
{
    acclmtr_config_flag = 1;
    P8OUT &= ~BIT2; // slave select
    i_acclmtr = 0;  // will send 4 bytes, 2 address and 2 value
    while (!(UCB1IFG&UCTXIFG)); // USCIB1 TX buffer ready?
      UCB1TXBUF = 0x2C; // Address of BW_rate register
      LPM0;
      while (!(UCB1IFG&UCTXIFG)); // USCIB1 TX buffer ready?
        UCB1TXBUF = 0x0B; // Set value to "0x0B" for 400Hz data rate
        LPM0;
        while (!(UCB1IFG&UCTXIFG)); // USCIB1 TX buffer ready?
          UCB1TXBUF = 0x31; // Address of Data_format register
          LPM0;
          while (!(UCB1IFG&UCTXIFG)); // USCIB0 TX buffer ready?
            UCB1TXBUF = 0x03; // 4 wire SPI, +-16g range. will change the range if needed, 10 bit mode
            LPM0;
            while (!(UCB1IFG&UCTXIFG)); // USCIB1 TX buffer ready?
                  UCB1TXBUF = 0x2D; // Address of Power_ctl register
                  LPM0;
                  while (!(UCB1IFG&UCTXIFG)); // USCIB1 TX buffer ready?
                    UCB1TXBUF = 0x08; // Set value to "0x08" to start measuring
                    LPM0;

                    P8OUT |= BIT2;    // acclmtr inactive

                    acclmtr_config_flag = 0; // next time in ISR, i will start saving data, config is done
}

void spi_com_acclmtr()
{
  P8OUT &= ~BIT2; // slave select
  i_acclmtr=0;
  Pacclmtr_data = (unsigned char *)acclmtr_data;  // pointer for acclmtr_data
  // sending 1 command byte followed by 2 dummy byte to receive 2 byte data for z axis
  while (!(UCB1IFG&UCTXIFG)); // USCIB1 TX buffer ready?
  UCB1TXBUF = 0xF6; // reg addrs for z0 data is 0x36, to read bit 8 is 1, for multiple bytes read bit7 is 1. so 0x36|0x80|0x40=0xF6
  LPM0;
  while (!(UCB1IFG&UCTXIFG)); // USCIB1 TX buffer ready?
  UCB1TXBUF = 0x00; // command to get z axis data
  LPM0;
  while (!(UCB1IFG&UCTXIFG)); // USCIB1 TX buffer ready?
  UCB1TXBUF = 0x00; // command to get z axis data
  LPM0;
  P8OUT |= BIT2;    // acclmtr inactive
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B1_VECTOR))) USCI_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB1IV,4))
  {
    case 0: break;
    case 2:
        while (!(UCB1IFG&UCTXIFG));           // USCIB1 TX buffer ready?
        if(acclmtr_config_flag == 0){   // config is done
      if(i_acclmtr!=0)  // data received while sending command byte is ignored, storing data received while sending dummy byte
      {
              *Pacclmtr_data++ = UCB1RXBUF;               // Move RX data to address PRxData
      }
        }

      LPM0_EXIT;
      i_acclmtr++;
      break;
    case 4: break;                          // Vector 4 - TXIFG
    default: break;
  }
}

void radio_setup(){
    /* Initial values for nRF24L01+ library config variables */
      rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
      rf_addr_width      = 5;
      rf_speed_power     = RF24_SPEED_1MBPS | RF24_POWER_0DBM;
      rf_channel         = 120;
      msprf24_init();  // All RX pipes closed by default
      msprf24_set_pipe_packetsize(0, 14);
//          msprf24_open_pipe(0, 0);  // Open pipe#0 with Enhanced ShockBurst enabled for receiving Auto-ACKs
              // Note: Pipe#0 is hardcoded in the transceiver hardware as the designated "pipe" for a TX node to receive
              // auto-ACKs.  This does not have to match the pipe# used on the RX side.
          msprf24_open_pipe(0, 0);
//          msprf24_open_pipe(1, 0);
          // Transmit to 'rad01' (0x72 0x61 0x64 0x30 0x31)
          msprf24_standby();
          user = msprf24_current_state();
          addr_tx[0] = 0x52; addr_tx[1] = 0x41; addr_tx[2] = 0x44; addr_tx[3] = 0x30; addr_tx[4] = 0x01;
          addr_rx[0] = 0x52; addr_rx[1] = 0x41; addr_rx[2] = 0x44; addr_rx[3] = 0x30; addr_rx[4] = 0x01;
//          addr_rx3[0] = 0x52; addr_rx3[1] = 0x41; addr_rx3[2] = 0x44; addr_rx3[3] = 0x30; addr_rx3[4] = 0x03;
          w_tx_addr(addr_tx);
//          w_rx_addr(o, addr_tx);  // Pipe 0 receives auto-ack's, autoacks are sent back to the TX addr so the PTX node
                               // needs to listen to the TX addr on pipe#0 to receive them.
          w_rx_addr(0, addr_rx);
//          w_rx_addr(3, addr_rx3);
          // for radio end
}

void send_msg(){
    w_tx_payload(14, buf_s);
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

void activate_rx_mode(){
    // Receive mode
     if (!(RF24_QUEUE_RXEMPTY & msprf24_queue_state())) {
         flush_rx();
     }
     msprf24_activate_rx();
//     LPM0;
}

void rcv_msg(){
    if (rf_irq & RF24_IRQ_FLAGGED) {
        rf_irq &= ~RF24_IRQ_FLAGGED;
        msprf24_get_irq_reason();
    }
    if (rf_irq & RF24_IRQ_RX || msprf24_rx_pending()) {
        r_rx_payload(14, buf_r);
        msprf24_irq_clear(RF24_IRQ_RX);
//        P1OUT ^= 0x01;
        //user = 0xFE;
//        buf_r[2] = buf_r[2]*3.6; // converting m/s to Km/h
        rcvd_msg_flag = 1;  // msg rcvd. chaeck if any action needed

    }
    else
        rcvd_msg_flag = 0;  // indicating no msg rcvd
}
