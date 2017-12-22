//#include <msp430g2553.h>
#include <msp430f5529.h>
#include "I2C_MAG3110.h"

int mag_x_msb=0, mag_x_lsb=0, mag_y_msb=0, mag_y_lsb=0, mag_z_msb=0, mag_z_lsb=0; // received data
int mag_X, mag_Y, mag_Z;  // received data

void CLOCK_config(void);
void MAG3110_config(void);

void main(void){
  WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
  CLOCK_config();
  I2C_config();
  MAG3110_config();
  while(1){
      mag_x_msb = I2C_receive(0x07);
      mag_x_lsb = I2C_receive(0x08);
      mag_y_msb = I2C_receive(0x02);
      mag_y_lsb = I2C_receive(0x04);
      mag_z_msb = I2C_receive(0x06);
      mag_z_lsb = I2C_receive(0x03);

  }

}


void CLOCK_config(void){
  //default clock settings for now(SMCLK 1MHz)
  //P1DIR |= BIT4;  // to check SMCLK
  //P1SEL |= BIT4;  // to check SMCLK

}


void MAG3110_config(void){
  I2C_transmit(0x10, 0x01); // active mode
  I2C_transmit(0x11, 0x80); // auto reset

}


#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
}


#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{

}
