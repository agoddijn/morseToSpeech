#include "msp430.h"

#define BUTTON BIT0 // Button at pin P2.0
#define TXD BIT1 // TXD at pin P1.1
#define RXD BIT2 // RXD at pin P1.2
#define LED1 BIT0
#define LED2 BIT6

#define MAX_BUFFER 50

/* 9600 Baud (bit/s) SW UART, SMCLK = 1MHz */

#define bitTime 13 // duration of 1 bit in timer A clock cycles (uses ID_3 (/8))

volatile unsigned char BitCnt;
volatile int TXByte;

/* END UART Stuff */

/* For Morse stuff */
volatile unsigned long unitTime; // duration of unit time in clock cycles (dynamic)
volatile unsigned int startTime = 0; // start time measured by clock
volatile unsigned long curTime; // end time measured by clock
volatile unsigned long time; // end time measured by clock
volatile unsigned int letter = 0; // to store lettercurrently stored in letter
volatile unsigned int overflowCount = 0; // number of timer overflows
volatile unsigned int debounce = 1000;

// For calibration
volatile char calibrateMode = 1;
volatile unsigned char calibrateCount = 0; // counts number of interrupts
volatile unsigned long calibrateTime = 0;

/* END Morse stuff */

void calibrateClock(void);
void configurePins(void);
void transmit(void);
void setupTimerUART(void);
void setupTimerMorse(void);
unsigned char getLetter(void);
void resetLetter(void);
void displayLetter(void);
void sendLong(long toSend, char hi);

void main(void) {
  WDTCTL = WDTPW + WDTHOLD; // Stop WDT
  calibrateClock();
  configurePins();
  // unitTime = 65535; // Set unit time to be 0.15s
  setupTimerMorse();
  __enable_interrupt();

  P1OUT &= ~(LED1 + LED2);

  while(1) {
    __bis_SR_register(LPM0_bits + GIE);  // LPM0 with interrupts enabled  turns cpu off.
    // For dynamic calibration
    if (calibrateMode) {

      if (calibrateCount != 0) {
        calibrateTime += time;
      }

      if (calibrateCount >= 16) { // Number of units in 8 dots and 8 gaps
        unitTime = (unsigned long) (calibrateTime / 16);
        // sendLong(unitTime,1);
        calibrateMode = 0;
      }

      calibrateCount++;

    // Main part
    } else {

      if (TA1CCTL0 & CCI) { // if previous one was low (button was pushed)

        if (time < (2 * unitTime)) { // dot
          letter = letter << 2;
          letter |= 1;
          // DEBUG
          P1OUT ^= LED2;

        } else if (time > (2 * unitTime)) { // dash
          letter = letter << 3;
          letter |= 3;
          // DEBUG
          P1OUT ^= LED1;
        }

      } else { // if previous one was high (button was not pushed)

        if (time > (2 * unitTime)) { // end of letter
          setupTimerUART();
          TXByte = getLetter();
          transmit();
          resetLetter();
        }

        if (time > (5 * unitTime)) { // end of word
          setupTimerUART();
          TXByte = (unsigned char) 0;
          transmit();
        }

        if (time > (10 * unitTime)) { // end of scentence
          setupTimerUART();
          TXByte = (unsigned char) 0b11111111;
          transmit();
        }

      }
    }
  }

}

void resetLetter(void) {
  letter = 0;
}

unsigned char getLetter(void) {
  switch (letter) {
    case 11:
      return 65;
    case 213:
      return 66;
    case 429:
      return 67;
    case 53:
      return 68;
    case 1:
      return 69;
    case 173:
      return 70;
    case 109:
      return 71;
    case 85:
      return 72;
    case 5:
      return 73;
    case 731:
      return 74;
    case 107:
      return 75;
    case 181:
      return 76;
    case 27:
      return 77;
    case 13:
      return 78;
    case 219:
      return 79;
    case 365:
      return 80;
    case 875:
      return 81;
    case 45:
      return 82;
    case 21:
      return 83;
    case 3:
      return 84;
    case 43:
      return 85;
    case 171:
      return 86;
    case 91:
      return 87;
    case 427:
      return 88;
    case 859:
      return 89;
    case 437:
      return 90;
    default:
      return 33;
  }
}

void sendLong(long toSend, char hi) {
  if (hi) {
    setupTimerUART();
    TXByte = (unsigned char) 0b01111111;
    transmit();
  } else {
    setupTimerUART();
    TXByte = (unsigned char) 0b11111110;
    transmit();
  }

  setupTimerUART();
  TXByte = (unsigned char) ((toSend >> 24) & 255);
  transmit();
  setupTimerUART();
  TXByte = (unsigned char) ((toSend >> 16) & 255);
  transmit();
  setupTimerUART();
  TXByte = (unsigned char) ((toSend >> 8) & 255);
  transmit();
  setupTimerUART();
  TXByte = (unsigned char) (toSend & 255);
  transmit();
}

void calibrateClock(void) {
  /* next three lines to use internal calibrated 1MHz clock: */
  BCSCTL1 = CALBC1_1MHZ; // Set range
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 &= ~(DIVS_3); // SMCLK = DCO = 1MHz
}

void configurePins(void) {
  P2REN |= BUTTON; // Enable button resistor
  P2OUT |= BUTTON; // Make button pull up
  P2DIR &= ~BUTTON;
  P2SEL |= BUTTON;
  P2SEL2 &= ~BUTTON;
  P1DIR |= TXD; // Make P1.1 output
  P1OUT |= TXD; // Make P1.1 idle high
  P1DIR |= LED1 + LED2;
}

void setupTimerMorse() {
  TA1CTL = TASSEL_2 + ID_3 + MC_2 + TAIE; // SMCLK as source, divide by 8, in continuous mode
  TA1CCTL0 = CM_3 + CCIS_0 + CAP + CCIE; // Capture rising and falling edge, input CCI0A, capture mode
}

void setupTimerUART() {
  TACCTL0 = OUT;
  TACTL = TASSEL_2 + MC_2 + ID_3; // SMCLK as source, divide by 8, in continuous mode
  P1SEL |= (TXD + RXD); // Set P1.1 to OUT0, set P1.2 to
  P1DIR |= TXD; // Set P1.1 to output
}

void transmit() {
  // TXByte sent in reverse order, little end is start bit
  BitCnt = 0xA; // Number of bits, 8 data + start/stop = 10
  TXByte |= 0x100; // Add stop bit to TXByte
  TXByte = TXByte << 1; // Add space for start bit
  TACCR0 = TAR + bitTime; // Set first interupt to occur in Bitime
  TACCTL0 =  CCIS0 + OUTMOD0 + CCIE; // Set enable interrupts and output mode to Set
  while ( TACCTL0 & CCIE ); // Wait for TXByte to be empty and interupt disabled
}

void __attribute__ ((interrupt(TIMER1_A1_VECTOR))) overflow (void)
{
  TA1CTL &= ~TAIFG;
  overflowCount++;
}

void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) measure (void)
{
  TA1CCTL0 &= ~CCIFG; // reset the interrupt flag
  curTime = (long)(TA1CCR0 + (65535 * overflowCount));
  curTime = curTime - startTime;

  if (curTime > debounce) {
    startTime = TA1CCR0;
    overflowCount = 0;
    time = curTime;
  }
  __bic_SR_register_on_exit(LPM0_bits); // Restart the cpu
}

// Timer A0 interrupt service routine -
#if defined(__TI_COMPILER_VERSION__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
#else
  void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#endif
{

  TACCR0 += bitTime; // Add bitTime offset

  if ( BitCnt == 0){
    P1SEL &= ~(TXD+RXD); // Return P1.1 and P1.2 to normal
    TACCTL0 &= ~ CCIE ; // All bits TXed, disable interrupt
  }
  else{
    TACCTL0 |=  OUTMOD2; // puts output unit in 'reset' mode
    if (TXByte & 0x01)
      TACCTL0 &= ~ OUTMOD2; // puts output unit in set mode
    TXByte = TXByte >> 1; // shift down so the next bit is in place.
    BitCnt --;
  }
}
