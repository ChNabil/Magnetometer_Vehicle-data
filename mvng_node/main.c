#include <msp430.h>
#include "msprf24.h"
#include "nrf_userconfig.h"
#include "stdint.h"

    uint8_t addr[5];
    uint8_t buf[3];

volatile unsigned int user;
char string[13];
const char string1[6] = {"Speed "};
char string2[3];
const char string3[13] = {" Kmph   Lane "};
const char string4[7] = {"   Ice "};
const char string5[5] = {" No\n\r"};
const char string6[6] = {" Yes\n\r"};
int i; // counter
int string_size;

void uart_setup();
void int_to_char();

int main()
{


    WDTCTL = WDTHOLD | WDTPW;
    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS_1;  // SMCLK = DCOCLK/2
    // SPI (USCI) uses SMCLK, prefer SMCLK < 10MHz (SPI speed limit for nRF24 = 10MHz)
//    uart_setup();
P1DIR |= 0x01;
P1OUT |= 0x00;
    //user = 0xFE;

    /* Initial values for nRF24L01+ library config variables */
    rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
    rf_addr_width      = 5;
    rf_speed_power     = RF24_SPEED_1MBPS | RF24_POWER_0DBM;
    rf_channel         = 120;

    msprf24_init();
    msprf24_set_pipe_packetsize(0, 3);
    msprf24_open_pipe(0, 1);  // Open pipe#0 with Enhanced ShockBurst

    // Set our RX address
    addr[0] = 0xDE; addr[1] = 0xAD; addr[2] = 0xBE; addr[3] = 0xEF; addr[4] = 0x00;
    w_rx_addr(0, addr);

    // Receive mode
    if (!(RF24_QUEUE_RXEMPTY & msprf24_queue_state())) {
        flush_rx();
    }
    msprf24_activate_rx();
    LPM0;

    while (1) {
        if (rf_irq & RF24_IRQ_FLAGGED) {
            rf_irq &= ~RF24_IRQ_FLAGGED;
            msprf24_get_irq_reason();
        }
        if (rf_irq & RF24_IRQ_RX || msprf24_rx_pending()) {
            r_rx_payload(3, buf);
            msprf24_irq_clear(RF24_IRQ_RX);
            P1OUT ^= 0x01;
            //user = 0xFE;
            buf[0] = buf[0]*3.6; // converting m/s to Km/h

     /*/ uart start
            for (i=0;i<6;i++)
            string[i] = string1[i];
            i = 0;
            string_size = 6;
            UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
            LPM0;

            int_to_char();
            for (i=0;i<3;i++)
            string[i] = string2[i];
            i = 0;
            string_size = 3;
            UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
            LPM0;

            for (i=0;i<13;i++)
            string[i] = string3[i];
            i = 0;
            string_size = 13;
            UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
            LPM0;

            if(buf[1] == 1) // check the lane number
                string[0] = 1;
            else
                string[0] = 2;

            i = 0;
            string_size = 1;
            UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
            LPM0;

            for (i=0;i<7;i++)
            string[i] = string4[i];
            i = 0;
            string_size = 7;
            UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
            LPM0;

            if(buf[2] == 0)
            {
                for (i=0;i<5;i++)
                           string[i] = string5[i];
                           i = 0;
                           string_size = 5;
                           UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
                           LPM0;
            }
            else
            {
                for (i=0;i<6;i++)
                           string[i] = string6[i];
                           i = 0;
                           string_size = 7;
                           UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
                           LPM0;
            }
            LPM0;

     // uart end */

        } else {
            //user = 0xFF;
        }
    }
    return 0;
}

void uart_setup()
{
    P1SEL |= BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
       P1SEL2 |= BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
       UCA0CTL1 |= UCSSEL_2; // SMCLK
          UCA0BR0 = 65; // 8MHz 9600    // max supported baud rate
          UCA0BR1 = 3; // 8MHz 9600
          UCA0MCTL = UCBRS2 + UCBRS0; // Modulation UCBRSx = 5
          UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
   //P1OUT |= TXLED;
     UCA0TXBUF = string[i++]; // TX next character
    if (i == string_size - 1) // TX over?
    {
       UC0IE &= ~UCA0TXIE; // Disable USCI_A0 TX interrupt
    //P1OUT &= ~TXLED;
       LPM0_EXIT;
    }
}

void int_to_char()
{
    string2[0] = buf[0]%100; // buf[0] is the speed
    string2[1] = (buf[0] - (string2[0]*100))%10; // buf[0] is the speed
    string2[2] = (buf[0] - ((string2[0]*100) + (string2[1]*10))); // buf[0] is the speed
}
