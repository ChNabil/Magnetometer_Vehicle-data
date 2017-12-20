//#include <msp430.h>
#include <msp430g2553.h>
#define MAG3110_add 0x0E  // Slave address for MAG3110
#define WHO_AM_I 0x07 // device identification number, set to 0xC4
#define CTRL_REG1 0x10  // MAG3110 ctl register1
#define CTRL_REG2 0x11  // MAG3110 ctl register2
#define OUT_X_MSB 0x01  // X axis MSB address
#define OUT_X_LSB 0x02  // X axis LSB address
#define OUT_Y_MSB 0x03  // X axis MSB address
#define OUT_Y_LSB 0x04  // X axis LSB address
#define OUT_Z_MSB 0x05  // X axis MSB address
#define OUT_Z_LSB 0x06  // X axis LSB address

int Rx = 0; // flag for TX mode
int ctrl_reg = 1; // flag to select which control reg is being written
int i = 2;  // number of byte needs to be sent
int j = 1;  // will be used for loop to send 6 output register address and read 6 output data
unsigned char mag_x_msb, mag_x_lsb, mag_y_msb, mag_y_lsb, mag_z_msb, mag_z_lsb; // received data
unsigned char mag_X, mag_Y, mag_Z;  // received data

void CLOCK_config(void);
void I2C_config(void);
void I2C_transmit(void);
void I2C_receive(void);
void MAG3110_config(void);
void UART_config(void);

void main(void){
  WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
  _EINT();
  CLOCK_config();
  I2C_config();
  MAG3110_config();
  while(1){
    for (j = 1; j < 7; j++) {
          Rx = 0; // flag for TX mode
          I2C_transmit();
          Rx = 1; // flag set for RX mode
          I2C_receive();
    }
    //mag_X =
    //mag_Y =
    //mag_Z =
    j = 1;  // reseting the loop
  }

}

void CLOCK_config(void){
  //default clock settings for now
  P1DIR |= BIT4;  // to check SMCLK
  P1SEL |= BIT4;  // to check SMCLK

}

void I2C_config(void){
  P1SEL |= BIT6 + BIT7; // assign pin 1.6(scl) and 1.7(sda) for I2C
  P1SEL2 |= BIT6 + BIT7;  // assign pin 1.6(scl) and 1.7(sda) for I2C
  UCB0CTL1 |= UCSWRST;  // reset enable
  UCB0CTL0 |= UCMST + UCMODE_3 + UCSYNC;  // master, i2c and sync mode
  UCB0CTL1 |= UCSSEL_2 + UCSWRST; // SMCLK + reset
  // UCB0BR0 |=
  // UCB0BR1 |=
  UCB0I2CSA = MAG3110_add;  // set slave address
  UCB0CTL1 &= ~UCSWRST;  // reset disable
  IE2 |= UCB0RXIE + UCB0TXIE; // RX & TX interrupt enable

}

void I2C_transmit(void){
  while (UCB0CTL1 & UCTXSTP); // wait till stop condition is sent
  UCB0CTL1 |= UCTR + UCTXSTT; // I2C TX mode + start condition
  LPM0;

}

void I2C_receive(void){
  while (UCB0CTL1 & UCTXSTP); // Ensure stop condition got sent
  UCB0CTL1 &= ~UCTR;  // RX mode
  UCB0CTL1 |= UCTXSTT;  // I2C start condition
  while (UCB0CTL1 & UCTXSTT); // Start condition sent?
  UCB0CTL1 |= UCTXSTP;  // I2C stop condition
  LPM0;

}

void MAG3110_config(void){
  Rx = 0;
  ctrl_reg = 1; // seting flag for control register 1
  I2C_transmit();
  ctrl_reg = 2; // seting flag for control register 2
  I2C_transmit();
  ctrl_reg = 0; // no more control register. send next address for output register. putting it in while loop will be same except 1 more command in each loop

}

void UART_config(void){

}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  if (Rx == 0){ // TX mode
    if (ctrl_reg == 1){ // writting control register 1
      if (i == 2){  // number of byte needs to be sent for control register 1
        UCB0TXBUF = CTRL_REG1;  // transmitting address for control register 1
        i--;

      }
      else{
        UCB0TXBUF = 0x01; // set active mode using control register 1
        i = 2;  // number of byte needs to be sent for control register 2
        UCB0CTL1 |= UCTXSTP;  // I2C stop condition
        IFG2 &= ~UCB0TXIFG; // Clear USCI_B0 TX flag
        LPM0_EXIT;
      }
    }
    else if (ctrl_reg == 2){ // writting control register 2
      if (i == 2){  // number of byte needs to be sent for control register 2
        UCB0TXBUF = CTRL_REG2;  // transmitting address for control register 2
        i--;

      }
      else{
        UCB0TXBUF = 0x80; // enable auto reset using control register 2
        UCB0CTL1 |= UCTXSTP;  // I2C stop condition
        IFG2 &= ~UCB0TXIFG; // Clear USCI_B0 TX flag
        LPM0_EXIT;
      }
    }
    else if (ctrl_reg == 0){ // dealing with output register, not control register
      if (j == 1) {  // selecting 1 of 6 output register
        UCB0TXBUF = 0x01; // register address for mag_x_msb
        UCB0CTL1 |= UCTXSTP;  // I2C stop condition
        IFG2 &= ~UCB0TXIFG; // Clear USCI_B0 TX flag
        LPM0_EXIT;
      }
        else if (j == 2){
        UCB0TXBUF = 0x02; // register address for mag_x_lsb
        UCB0CTL1 |= UCTXSTP;  // I2C stop condition
        IFG2 &= ~UCB0TXIFG; // Clear USCI_B0 TX flag
        LPM0_EXIT;
      }
        else if (j == 3){
        UCB0TXBUF = 0x03; // register address for mag_y_msb
        UCB0CTL1 |= UCTXSTP;  // I2C stop condition
        IFG2 &= ~UCB0TXIFG; // Clear USCI_B0 TX flag
        LPM0_EXIT;
      }
      else if (j == 4){
        UCB0TXBUF = 0x04; // register address for mag_y_lsb
        UCB0CTL1 |= UCTXSTP;  // I2C stop condition
        IFG2 &= ~UCB0TXIFG; // Clear USCI_B0 TX flag
        LPM0_EXIT;
      }
      else if(j == 5){
        UCB0TXBUF = 0x05; // register address for mag_z_msb
        UCB0CTL1 |= UCTXSTP;  // I2C stop condition
        IFG2 &= ~UCB0TXIFG; // Clear USCI_B0 TX flag
        LPM0_EXIT;
      }
      else if (j == 6){
        UCB0TXBUF = 0x06; // register address for mag_z_lsb
        UCB0CTL1 |= UCTXSTP;  // I2C stop condition
        IFG2 &= ~UCB0TXIFG; // Clear USCI_B0 TX flag
        LPM0_EXIT;
      }
      }
    }
  
  else if (Rx == 1)
  {
    switch (j) {
      case 1: mag_x_msb = UCB0RXBUF;  // read mag_x_msb from corresponding register
      LPM0_EXIT;
      break;
      case 2: mag_x_lsb = UCB0RXBUF;  // read mag_x_lsb from corresponding register
      LPM0_EXIT;
      break;
      case 3: mag_y_msb = UCB0RXBUF;  // read mag_y_msb from corresponding register
      LPM0_EXIT;
      break;
      case 4: mag_y_lsb = UCB0RXBUF;  // read mag_y_lsb from corresponding register
      LPM0_EXIT;
      break;
      case 5: mag_z_msb = UCB0RXBUF;  // read mag_z_msb from corresponding register
      LPM0_EXIT;
      break;
      case 6: mag_z_lsb = UCB0RXBUF;  // read mag_z_lsb from corresponding register
      LPM0_EXIT;
      break;
    }

  }

}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
  // never receiving more than one byte

}
