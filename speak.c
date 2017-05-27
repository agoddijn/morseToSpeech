#include "msp430.h"

#define LED1 BIT0
#define LED2 BIT6
#define OUTPUT BIT4

volatile int count = 0;
volatile char toTransmit[] = "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG";
volatile int unitTime = 5000; // Default 19660, fastest 500

unsigned int getSeq(char letter);
void outputLetter(char letter);
void outputHigh(int times);
void outputLow(int times);
void setupTimer(void);

void main(void){
  WDTCTL = WDTPW + WDTHOLD;  // Stop WDT

  __enable_interrupt();
  setupTimer();

  P1DIR |= LED2 + LED1;           // P1.1 to output

  P1OUT &= ~LED1;
  P1OUT &= ~LED2;

  for (int i = 0; i < 8; i ++) {
    outputHigh(1);
    outputLow(1);
  }

  for (int i = 0; toTransmit[i] != 0; i++){
    outputLetter(toTransmit[i]);
  }
  outputLow(10);
  outputHigh(1);
  P1DIR &= ~OUTPUT;
}

void setupTimer() {
  // SMCLK calibration
  BCSCTL1 = CALBC1_1MHZ;            // Set range
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 &= ~(DIVS_3);             // SMCLK = DCO = 1MHz

  // Timer setup
  CCR0 = unitTime;                  // Period
  CCTL1 = CCIE;
  TACTL = TASSEL_2 + ID_3 + MC_1;   // SMCLK, up mode, divide by 8
};

void outputLetter(char letter){
  int seq = getSeq(letter);
  if(seq == 1) {
    outputLow(4);
    return;
  } else {
    while(1) {
      if (!(seq & 0b1100000000000000)) {
        outputLow(3);
        return;
      } else {
        int cur = (seq & 0b1000000000000000) >> 15;
        if (cur == 1) {
          outputHigh(1);
        } else {
          outputLow(1);
        }
        seq = seq << 1;
      }
    }
  }

};

void outputHigh(int times) {
  P1OUT &= ~LED1;
  P1OUT |= LED2;
  while (count < times) {}
  count = 0;
  return;
};

void outputLow(int times) {
  P1OUT |= LED1;
  P1OUT &= ~LED2;
  while (count < times) {}
  count = 0;
  return;
};

void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) overflow (void)
{
  CCTL1 &= ~CCIFG;
  count++;
};

unsigned int getSeq(char letter) {
  switch (letter) {
    case 32:
      return 1;
    case 65:
      return 47104;
    case 66:
      return 60032;
    case 67:
      return 60320;
    case 68:
      return 59904;
    case 69:
      return 32768;
    case 70:
      return 44672;
    case 71:
      return 61056;
    case 72:
      return 43520;
    case 73:
      return 40960;
    case 74:
      return 48056;
    case 75:
      return 60288;
    case 76:
      return 47744;
    case 77:
      return 60928;
    case 78:
      return 59392;
    case 79:
      return 61152;
    case 80:
      return 48032;
    case 81:
      return 61112;
    case 82:
      return 47616;
    case 83:
      return 43008;
    case 84:
      return 57344;
    case 85:
      return 44544;
    case 86:
      return 43904;
    case 87:
      return 48000;
    case 88:
      return 60128;
    case 89:
      return 60344;
    case 90:
      return 61088;
    default:
      return 33;
  }
};