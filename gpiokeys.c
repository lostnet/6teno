#include "fwork.h"

void onSetup(StenoLog *s, StenoStats *st) {
  
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
int onInterrupt(StenoLog *s, StenoStats *st)
{

  unsigned long volatile upd = 0;

  if (!(PAIFG & PADA_M || PBIFG & PADB_M))
	return 0;

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
