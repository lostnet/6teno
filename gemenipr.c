#include "fwork.h"

#define GEMENIFEATURE (fcache[gemenim])
MNUM gemenim;


void GEMENI_onSetup(volatile StenoLog *s, volatile StenoStats *st) {
}

int GEMENI_onFlagsWake(volatile StenoLog *s, volatile StenoStats *st) {

  s->flags &= ~R2NOTIFY;

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
  UCA0IE |= UCTXIE;     // interrupt on TX success

  while (!(UCA0IFG&UCTXIFG));
  s->nch = 0;
  s -> flags |= CSEND;
  UCA0TXBUF = 1 << 7; // first packet, no fn or '#'
}

#pragma vector = USCI_A0_VECTOR
void GEMENI_charsent() {
	unsigned long r = s->log[s->lc];
	if (UCA0IFG & UCTXIFG) {
		s->nch++;
		switch (s->nch) {
			case 1:
				UCA0TXBUF = SL(r) << 5 |TL(r)<<4|KL(r)<<3|PL(r)<<2|WL(r)<<1|HL(r);
			break;
			case 2:
				UCA0TXBUF = RL(r)<<6 | AL(r)<<5 | OL(r)<<4 | STAR(r)<<3;
			break;
			case 3:
				UCA0TXBUF = ER(r)<<3 | UR(r)<<2 | FR(r)<<1 | RR(r);
			break;
			case 4:
				UCA0TXBUF = PR(r)<<6 | BR(r)<<5 | LR(r)<<4 | GR(r)<<3 | TR(r)<<2 | SR(r)<<1 | DRS(r);
			break;
			case 5:
				UCA0TXBUF = POUND(r)<<1 | ZRS(r);
			break;
			default:
				s->lc++;
				if (s->lc != s->nc-1) {

  					s->nch = 0;
  					UCA0TXBUF = 1 << 7; // first packet, no fn or '#'
				} else {
					s->flags &= ~CSEND;
				}

		}
	}
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
  GEMENIFEATURE.pins = 0;
  GEMENIFEATURE.onSetup = &GEMENI_onSetup;
  GEMENIFEATURE.onFlagsWake = &GEMENI_onFlagsWake;
  GEMENIFEATURE.onInterrupt = &GEMENI_onInterrupt;
  GEMENIFEATURE.ivectors = (void*)0;
  GEMENIFEATURE.flags = R2NOTIFY;
  GEMENIFEATURE.sleep_bits = 0;
  GEMENIFEATURE.pmm_bits = 0;
}
