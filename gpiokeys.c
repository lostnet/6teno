#include "fwork.h"

#define GPIOFEATURE (fcache[gpiom])
MNUM gpiom;

void GPIO_onSetup(volatile StenoLog *s, volatile StenoStats *st) {
  
  PASEL1 &= (~PADA_M);
  PBSEL1 &= (~PADB_M); // leave the rest

  PADIR &= (~PADA_M); // inputs
  PBDIR &= (~PADB_M);
  
  // pull high
//  PAOUT |= PADA_M;
//  PBOUT |= PADB_M;

  PAOUT &= (~PADA_M); // pull low
  PBOUT &= (~PADB_M);

  PAREN |= PADA_M; // enable R
  PBREN |= PADB_M;

//  PAIES |= PADA_M; // on high->low edge
//  PBIES |= PADB_M;

  PAIES &= ~PADA_M; 
  PBIES &= ~PADB_M; // on low->high edge

  PAIE |= PADA_M; // enable
  PBIE |= PADB_M;

  PAIFG &= ~PADA_M; // clear
  PBIFG &= ~PADB_M;

}

/*
pragma vector=PORT1_VECTOR
pragma vector=PORT2_VECTOR
pragma vector=PORT3_VECTOR
pragma vector=PORT4_VECTOR

There are 3 potential groups of pins in a chord:
	currently pressed (pull 1 low & watch, free float the rest)
	past pressed (pull low again, watching is optional)
	never pressed (pull low, watch all for going high)

*/
int GPIO_onInterrupt(volatile StenoLog *s, volatile StenoStats *st)
{

  unsigned long volatile upd = 0;


  upd = (PAIN & PADA_M);
  upd = upd<<16;
  upd |= (PBIN & PADB_M);
  st->rchrd |= upd;

  PAREN |= PADA_M; // enable R on allkeys 
  PBREN |= PADB_M;
  PAREN &= ~(PAIN & PADA_M); // disable R on downkeys
  PBREN &= ~(PBIN & PADB_M);

  PAIE |= PADA_M; // enable I on allkeys
  PBIE |= PADB_M;
  PAIE &= ~(PAIN & PADA_M); // disable I on downkeys
  PBIE &= ~(PBIN & PADB_M);


  PAIES = (PADA_M & PAIN) | (PAIES & ~PADA_M); // set edges
  PBIES = (PADB_M & PBIN) | (PBIES & ~PADB_M);

  PAIFG &= ~PADA_M; // clear flags
  PBIFG &= ~PADB_M;

  TA0CTL |= TACLR;	// reset debounce time	

  if (upd == 0) {
	if(s->log[s->nc] != 0) {
    		s->nc += 1;
    		s->flags |= CHREADY;

    		return 1;
	}
  } else {
	int x;
	x = __builtin_ctzl(upd); // get 1 pressedkey
	if (x < 16) { // B Pad
		x = 1<<x;
		PBIE  |= x;
		PBREN |= x;
		// IES already setup
	} else {  // A PAD
		x = 1 << (x-16);
		PAIE |= x;
		PAREN |= x;
	}
  	s->log[s->nc] |= upd;
	s->flags &= ~CHREADY;
  }
 
  return 0;
}

int GPIO_onFlagsWake(volatile StenoLog *s, volatile StenoStats *st) {
  	TA0CCR0 = DEBOUNCE; // reset bounce wait on any key event
	TA0CTL = TASSEL_1 + MC_1 + TAIE;	

}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void GPIO_Timer(void) {
	if (s->flags & CHREADY) {
		s->flags &= ~CHREADY;
		s->nc = (s->nc+1) % LOG_MAX;
		s->log[s->nc] = 0;
		s->flags |= R2NOTIFY;
	}
	TA0CTL = 0;
}

int GPIO_onRegister() {
  gpiom = gpio;
  GPIOFEATURE.id = gpio;
  GPIOFEATURE.pins = PADA_M;
  GPIOFEATURE.pins = (GPIOFEATURE.pins << 16) | PADB_M;
  GPIOFEATURE.onSetup = &GPIO_onSetup;
  GPIOFEATURE.onFlagsWake = &GPIO_onFlagsWake;
  GPIOFEATURE.onInterrupt = &GPIO_onInterrupt;
  GPIOFEATURE.ivectors = (void*)0;
  GPIOFEATURE.flags = CHREADY;
  GPIOFEATURE.sleep_bits = 0;
  GPIOFEATURE.pmm_bits = 0;
}
