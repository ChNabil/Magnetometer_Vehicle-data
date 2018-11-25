#include <msp430.h>
#include "msprf24.h"
#include "nrf_userconfig.h"
#include "stdint.h"

    uint8_t addr_rx[5];
//    uint8_t addr_rx3[5];
    uint8_t addr_tx[5];
    uint8_t buf_r[6]; //rcvd msg. 6 bits. 1.sender add. 2.Road no. 3.Speed. 4.Lane no. 5.Ice condition 6.Warning
    uint8_t buf_s[6]; //sent msg. 6 bits. 1.sender add. 2.Road no. 3.Speed. 4.Lane no. 5.Ice condition 6.Warning
    char sender_add, road_no, rcvd_msg_flag;

volatile unsigned int user;
void radio_setup();
void activate_rx_mode();
void rcv_msg();
void send_msg();

int main()
{
    WDTCTL = WDTHOLD | WDTPW;
    char temp;
    sender_add = 5; // 5 for all vehicles. 1,2,3,4 for 4 RSUs
    road_no = 3;    // assuming vehicle knows road no. Will try to figure out how vehicle can get that info
                    // cant just receive it from RSUs since vehicle may receive msg from all 4 RSUs
    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS_1;  // SMCLK = DCOCLK/2
    // SPI (USCI) uses SMCLK, prefer SMCLK < 10MHz (SPI speed limit for nRF24 = 10MHz)
//P1DIR |= 0x01;
//P1OUT |= 0x00;
    //user = 0xFE;
radio_setup();
activate_rx_mode();
rcvd_msg_flag = 0;  // no msg rcvd yet
int p;
//buf_s[0]=8;
//send_msg();

    while (1) {
        rcv_msg();
        if(rcvd_msg_flag == 1)   // new msg rcvd. see what action rqrd
        {
            if(buf_r[0] != 1 && buf_r[0] != 5)  // ignoring msg from RSU1 and vehicle 1 since we're assuming they are out of range from vehicle2
            {
                temp = 1;   // display msg here
                temp = 1;   // display msg here
            }
        if(buf_r[0] == buf_r[1] && buf_r[1] == road_no)  //when car rcvd a msg, only sent back msg if rcvd msg was sent by the RSU
        {                                                //of the same road(buf_r[0]) that the car is on and also the msg was for the same road(buf_r[1])
            buf_s[0] = sender_add;                               // sender address and road no is changed. rest of the msg is same since the car dont have any new info for now
            buf_s[1] = road_no;
            buf_s[2] = buf_r[2];
            buf_s[3] = buf_r[3];
            buf_s[4] = buf_r[4];
            buf_s[5] = buf_r[5];
            for(p=0;p<30000;p++);
            send_msg();
            activate_rx_mode(); // going back to rx mode after sending msg
        }
        }
    }
    return 0;
}

void rcv_msg(){
    if (rf_irq & RF24_IRQ_FLAGGED) {
        rf_irq &= ~RF24_IRQ_FLAGGED;
        msprf24_get_irq_reason();
    }
    if (rf_irq & RF24_IRQ_RX || msprf24_rx_pending()) {
        r_rx_payload(6, buf_r);
        msprf24_irq_clear(RF24_IRQ_RX);
//        P1OUT ^= 0x01;
        //user = 0xFE;
//        buf_r[2] = buf_r[2]*3.6; // converting m/s to Km/h
        rcvd_msg_flag = 1;  // rcvd new msg. see if any action required
    }
    else
        rcvd_msg_flag = 0;  // no new msg rcvd
}

void radio_setup(){
    /* Initial values for nRF24L01+ library config variables */
    rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
    rf_addr_width      = 5;
    rf_speed_power     = RF24_SPEED_1MBPS | RF24_POWER_0DBM;
    rf_channel         = 120;

    msprf24_init();
    msprf24_set_pipe_packetsize(0, 6);
    msprf24_open_pipe(0, 0);  // Open pipe#0 with Enhanced ShockBurst
//    msprf24_open_pipe(1, 0);

    // Set our TX and RX address
    addr_rx[0] = 0x52; addr_rx[1] = 0x41; addr_rx[2] = 0x44; addr_rx[3] = 0x30; addr_rx[4] = 0x01;  // RAD01 ascii
//    addr_rx3[0] = 0x52; addr_rx3[1] = 0x41; addr_rx3[2] = 0x44; addr_rx3[3] = 0x30; addr_rx3[4] = 0x03;  // RAD03 ascii
    addr_tx[0] = 0x52; addr_tx[1] = 0x41; addr_tx[2] = 0x44; addr_tx[3] = 0x30; addr_tx[4] = 0x01;  //RAD05 ascii
//    w_tx_addr(addr_rx);
    w_rx_addr(0, addr_rx);  // pipe 0, reading rad01
//    w_rx_addr(1, addr_rx3);  //pipe 1, reading rad03
    w_tx_addr(addr_tx);
//    w_rx_addr(0, addr_tx);
}

void activate_rx_mode(){
    // Receive mode
     if (!(RF24_QUEUE_RXEMPTY & msprf24_queue_state())) {
         flush_rx();
     }
     msprf24_activate_rx();
     LPM0;
}

void send_msg(){
    w_tx_payload(6, buf_s);
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

