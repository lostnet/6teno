// include <msp430.h>
#include "fwork.h"
// include <msp430fr5739.h>
//include <signal.h>

// debugging statistics
volatile StenoStats *st = (volatile struct StenoStats *)0xcb00;

// features to implement starting with a cache of the total enabled ones.
volatile StenoFeature *fcache = (struct StenoFeature *) 0xcc00;

// The raw log
volatile StenoLog *s = (volatile struct StenoLog *)0xcd00;

void slp() {
	if (!(fcache->flags & VALID)) { // buildcache

	}
       __disable_interrupt();
	PMMCTL0_H = PMMPW_H;
	PMMCTL0_L |= PMMREGOFF & fcache->pmm_bits;
	PMMCTL0_L &= ~(SVSLE) | fcache->pmm_bits; // -SVSHE
    	// PM5CTL0 &= ~LOCKLPM5;
	PMMCTL0_H = 0x00;
        __bis_SR_register((LPM4_bits + GIE) & fcache->sleep_bits);
        __no_operation();
}

void setp() {

  if ((s->nc >= LOG_MAX) || (s->log[s->nc] != 0)) {
  	s->nc = (s->nc+1) % LOG_MAX;
   	s->log[s->nc] = 0;
  }
  if ((s->lc >= LOG_MAX) || (s->lc == s->nc)) {
	s->lc = s->nc ? s->nc-1:LOG_MAX-1;
	s->i2nch = 0;
  }

   st->mc = 1;
   st->rc += 1;
   st->wk = 0xafdeafde;
}

int main(void) {
  st->mc += 1;
  // stop WD
    WDTCTL = WDTPW + WDTHOLD;
  // unlock pins from sleep?
    PM5CTL0 &= ~LOCKLPM5;

    CSCTL0_H = 0xA5; // Password
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;          // Set max. DCO setting = 8MHz
    CSCTL2 = SELA_1 + SELS_3 + SELM_3;      // set ACLK = VLO,  MCLK = DCO
    CSCTL3 = DIVA_5 + DIVS_3 + DIVM_3;      // ~375Hz, 1MHz, 1MHz
    CSCTL0_H = 0x01; // Close access


  if ((s->flags & s->active_flags) == 0)
	setp();
  // ? __enable_interrupt();
  while (1) {
    	__delay_cycles(20000); // 20 MS?
  	if (!(s->flags & DEBUG)) {
		slp();
   		st->wk = 0xedfecefa;
  	}
  }
  return 1;
}


#pragma vector=PORT1_VECTOR
#pragma vector=PORT2_VECTOR
#pragma vector=PORT3_VECTOR
#pragma vector=PORT4_VECTOR
#pragma vector=USCI_B0_VECTOR
#pragma vector=USCI_A0_VECTOR
#pragma vector=USCI_A1_VECTOR
#pragma vector=DMA_VECTOR
__interrupt void watchInts() {

}
