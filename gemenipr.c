#include "fwork.h"

#define GEMENIFEATURE (fcache[gemenim])
MNUM gemenim;

void GEMENI_onSetup(volatile StenoLog *s, volatile StenoStats *st) {

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
int GEMENI_onInterrupt(volatile StenoLog *s, volatile StenoStats *st)
{}

int GEMENI_onRegister()
{

  gemenim = gemeni;
  GEMENIFEATURE.id = gemenim;
  GEMENIFEATURE.pins = 1;
  GEMENIFEATURE.onSetup = &GEMENI_onSetup;
  GEMENIFEATURE.onFlagsWake = (void*)0;
  GEMENIFEATURE.onInterrupt = *GEMENI_onInterrupt;
  GEMENIFEATURE.ivectors = (void*)0;
  GEMENIFEATURE.flags = 0;
  GEMENIFEATURE.sleep_bits = 0;
  GEMENIFEATURE.pmm_bits = 0;
}
