#include <msp430f5529.h>
#include <msp430.h>
#include "msprf24.h"
#include "nrf_userconfig.h"

void clock_setup();
void radio_setup();
void activate_rx_mode();
void send_msg();
void rcv_msg();

// for radio start
volatile unsigned int user;
char addr_tx[5];
char addr_rx[5];
//char addr_rx3[5];
char buf_r[14]; //rcvd msg 14 Bytes. See "Communication V2I I2X.docx" for details
char buf_s[14]; //sent msg 14 Bytes. See "Communication V2I I2X.docx" for detailschar sender_add = 3;
char road_no = 3;
char sender_add = 3;
char rcvd_msg_flag, send_msg_flag;
int p;

void main()
{
  WDTCTL = WDTPW + WDTHOLD;
  _EINT();

  P1DIR &= ~ BIT1;  // switch 2, emulate detection of a vehicle
  P1REN |= BIT1;    // internal resistance enabled
  P1OUT |= BIT1;    // pull-up resistance
  P1IFG &= ~BIT1;   // ifg cleared
  P1IE |= BIT1;     // enable interrupt for switch 2

//  clock_setup();
  rcvd_msg_flag = 0; // no msg rcvd yet
  send_msg_flag = 0;    // no need to send msg now
  radio_setup();
  activate_rx_mode();   // start radio in rx mode, change to tx mode when needed

  while(1)
  {
      if(send_msg_flag == 1)
      {
          send_msg_flag = 0;    // reset flag
          send_msg();
          activate_rx_mode();
      }
      rcv_msg();    // keep checking for rcvd msg.  take action when msg rcvd
      if(rcvd_msg_flag == 1)  // msg rcvd. see if any action needed
          if(buf_r[2] == 192) // if rcvd msg is type 3, send back the msg with own sender address
          {
              buf_s[0] = sender_add;
              for(p=1;p<14;p++)
                  buf_s[p] = buf_r[p];
              send_msg();
              activate_rx_mode();
          }
      rcvd_msg_flag = 0;
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
//          addr_rx3[0] = 0x52; addr_rx3[1] = 0x41; addr_rx3[2] = 0x44; addr_rx3[3] = 0x30; addr_rx3[4] = 0x01;
          w_tx_addr(addr_tx);
//          w_rx_addr(o, addr_tx);  // Pipe 0 receives auto-ack's, autoacks are sent back to the TX addr so the PTX node
                               // needs to listen to the TX addr on pipe#0 to receive them.
          w_rx_addr(0, addr_rx);
//          w_rx_addr(1, addr_rx3);
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
     LPM0;
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

// Port 1 interrupt service routine

#pragma vector=PORT1_VECTOR

__interrupt void Port_1(void)

{

    switch( __even_in_range( P1IV, P1IV_P1IFG7 )) {

    case P1IV_P1IFG1:   // Pin 1.1 (switch 2)
        buf_s[0] = sender_add;
        buf_s[1] = road_no;
        buf_s[2] = 50;
        buf_s[3] = 1;
        buf_s[4] = 1;
        buf_s[5] = 1;
        send_msg_flag = 1;
//        send_msg();   // send msg function has LPM0, wont work inside isr
//        activate_rx_mode;
//        P1IFG &= ~BIT1;
        LPM0_EXIT;
        break;

    default:   _never_executed();


    }
}
