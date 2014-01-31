#include "fwork.h"

void GEMENI_onSetup(StenoLog *s, StenoStats *st) {

  // Configure UART pins
  P2SEL1 |= BIT0 + BIT1;
  P2SEL0 &= ~(BIT0 + BIT1);

  // Configure UART 0
  UCA0CTL1 |= UCSWRST; 
  UCA0CTL1 = UCSSEL_2;	// Set SMCLK as UCBRCLK
  UCA0BR0 = 0x45;	// 115000 baud
  UCA0BR1 = 0;
  UCA0MCTLW |= 0xAA00;	// 8000/115.2 - INT(8000/115.2) = 0.44..
  UCA0CTL1 &= ~UCSWRST;	// release from reset
  UCA0IE |= UCRXIE;   

}

/*
pragma vector=PORT1_VECTOR
pragma vector=PORT2_VECTOR
pragma vector=PORT3_VECTOR
pragma vector=PORT4_VECTOR


*/
int GEMENI_onInterrupt(StenoLog *s, StenoStats *st)
{}
