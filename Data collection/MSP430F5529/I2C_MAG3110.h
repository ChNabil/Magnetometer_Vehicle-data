#ifndef I2C_MAG3110_H_
#define I2C_MAG3110_H_

#include <msp430f5529.h>
//#include <msp430g2553.h>

int MAG3110_add = 0x0E;  // Slave address for MAG3110
int WHO_AM_I = 0x07; // device identification number, set to 0xC4

void I2C_config(void){
    //P1SEL |= BIT6 + BIT7; // assign pin 1.6(scl) and 1.7(sda) for I2C
    //P1SEL2 |= BIT6 + BIT7;  // assign pin 1.6(scl) and 1.7(sda) for I2C
    P3SEL |= BIT0 + BIT1;
    UCB0CTL1 |= UCSWRST;    // reset enable
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;   // master, i2c and sync mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;  // Use SMCLK(default 1MHz), keep SW reset
    UCB0BR0 = 10;   // fSCL = 1MHz/10 = ~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = MAG3110_add;    // set slave address
    UCB0CTL1 &= ~UCSWRST;   // reset disable
    UCB0IE |= UCRXIE + UCTXIE; // RX & TX interrupt enable
}

void I2C_transmit(int registerAddr, int value){
    while (UCB0CTL1 & UCTXSTP); // wait for stop condition to finish
    UCB0CTL1 |= UCTR + UCTXSTT; // Transmit mode + Start condition(this will set UCB0TXIFG)
    while((UCB0IFG & UCTXIFG) == 0); // wait for UCBOTXIFG to set
    UCB0TXBUF = registerAddr;   // address of the control register
    while((UCB0IFG & UCTXIFG) == 0); // wait for transmission to finish, UCB0TXIFG will be set then
    UCB0TXBUF = value;   // set the value to be sent in control register
    while((UCB0IFG & UCTXIFG) == 0); // // wait for transmission to finish, UCB0TXIFG will be set then
    UCB0CTL1 |= UCTXSTP;    // stop condition
    UCB0IFG &= ~UCTXIFG; // Clear Tx flag
}

int I2C_receive(int registerAddr){
    int receivedData;
    while (UCB0CTL1 & UCTXSTP); // wait for stop condition to finish
    UCB0CTL1 |= UCTR + UCTXSTT; // Transmit mode + Start condition(this will set UCB0TXIFG)
    while((UCB0IFG & UCTXIFG) == 0); // wait for UCBOTXIFG to set
    UCB0TXBUF = registerAddr;   // address of the register that needs to be read
    while((UCB0IFG & UCTXIFG) == 0); // wait for transmission to finish
    UCB0CTL1 &= ~UCTR ; // Rx mode
    UCB0CTL1 |= UCTXSTT + UCTXNACK; // start condition with not-acknowledge to read a single byte
    while (UCB0CTL1 & UCTXSTT); // wait for start condition to finish
    __delay_cycles(100);    // RXBUF is ready but can't copy the data from it without this delay, need to check why
    receivedData = UCB0RXBUF;   // read data
    UCB0CTL1 |= UCTXSTP;    // stop condition
    return receivedData;    // return data
}


#endif
