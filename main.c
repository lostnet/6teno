// include <msp430.h>
#include <msp430fr5739.h>
//include <signal.h>


struct StenoStats {
	unsigned long volatile mc; // main entrances
	unsigned long volatile rchrd; // raw chord(s) overlayed
	unsigned long volatile ic; // interrupt entrances
	unsigned long volatile rc; // reconfigure entrances
	unsigned long volatile wk; // bad wakeup
};

// 4kb/1kc log for testing
#define LOG_MAX 1024

struct StenoLog {
	unsigned int nc; // next chord location
	unsigned int lc; // last sent chord
	unsigned char i2nch; // last sent sub byte
	unsigned char flags; // log status
	unsigned char active_flags; // signs of life
	unsigned char resrv;
	unsigned int volatile log[LOG_MAX]; // bulk log
};

struct AsyncTask {
	unsigned long pins;
	void (*setup)();
	void (*onFlagsWake)();
	void (*interrupt)();
	void *ivectors;
	unsigned char flags;
};

#define I2CP (BIT6|BIT7)
#define PADA_M (~I2CP)
// define PADA_M (~0x0)
#define PADB_M ~(BITF|BITE|BITD|BITC|BITB|BITA)


// FLAGS
#define CHREADY 1
#define STRT 2
#define R2NOTIFY 4
#define I2CCSEND 8

volatile struct StenoStats *st = (volatile struct StenoStats *)0xcb00;
volatile struct StenoLog *s = (volatile struct StenoLog *)0xcc00;

void slp() {
	if (PAIN & BIT7) {
		PAIES |= BIT6; // high -> low;
		PAIE |= BIT6;
	}
	if (PAIN & BIT0) { // block sleep for a reset
		return;
	}

       __disable_interrupt();
	PMMCTL0_H = PMMPW_H;
	PMMCTL0_L |= PMMREGOFF;
	PMMCTL0_L &= ~(SVSHE + SVSLE);
    	PM5CTL0 &= ~LOCKLPM5;
	PMMCTL0_H = 0x00;
        __bis_SR_register(LPM4_bits + GIE);
        __no_operation();

}

void setp() {
  
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

  if (PAIN & BIT7) {
	  PJDIR |= BIT0; // signal pin
	  PJOUT &= ~BIT0;
  }


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

void lp () {
   while(1) {

    if (!(s->flags & (CHREADY|R2NOTIFY))) {
	slp();
   	st->wk = 0xedfecefa;
    }

    __delay_cycles(20000); // 20 MS?
    if (s->flags & R2NOTIFY) {
	s->flags &= ~R2NOTIFY;
	PJOUT &= ~BIT0;
    }

    if (s->flags & CHREADY) {
        s->flags &= ~CHREADY;
   	s->nc = (s->nc+1) % LOG_MAX;
   	s->log[s->nc] = 0;
	PJOUT |= BIT0;
        s->flags |= R2NOTIFY;
    }
   }

}

int main(void) {
  // stop WD
    WDTCTL = WDTPW + WDTHOLD;
  // unlock pins from sleep?
    PM5CTL0 &= ~LOCKLPM5;

  // Init SMCLK = MCLk = ACLK = 1MHz
    CSCTL0_H = 0xA5;
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;          // Set max. DCO setting = 8MHz
    CSCTL2 = SELA_3 + SELS_3 + SELM_3;      // set ACLK = MCLK = DCO
    CSCTL3 = DIVA_3 + DIVS_3 + DIVM_3;      // set all dividers to 1MHz
    CSCTL0_H = 0x01;


  if ((s->flags & CHREADY) == 0)
	setp();
  __enable_interrupt();
  st->mc += 1;
  lp();
  return 1;
}

/*
pragma vector=PORT1_VECTOR
pragma vector=PORT2_VECTOR
pragma vector=PORT3_VECTOR
pragma vector=PORT4_VECTOR
*/
static inline int keyEvent()
{

  unsigned long volatile upd = 0;

  if (PAIFG & BIT7) { // i2c is active
    PAIE ^= BIT6;
  // boiler plate
    UCB0CTLW0 |= UCSWRST;                  //Software reset enabled
    UCB0CTLW0 |= UCMODE_3  + UCSYNC;        //I2C mode, sync mode
    UCB0I2COA0 = 0x6A + UCOAEN;
    UCB0CTLW0 &=~UCSWRST;                   //clear reset register
    UCB0IE |=  UCTXIE0 + UCRXIE0 + UCSTPIE + UCNACKIE;
  }

  if (!(PAIFG & PADA_M || PBIFG & PADB_M))
	return 0;

  st->ic +=1;

  PAIES = (PADA_M & PAIN) | (PAIES & ~PADA_M);
  PBIES = (PADB_M & PBIN) | (PBIES & ~PADB_M);

  PAIFG &= ~PADA_M; // clear flags
  PBIFG &= ~PADB_M;

  upd = (PAIN & PADA_M);
  upd = upd<<16;
  upd |= (PBIN & PADB_M);
  st->rchrd |= upd;

  if (upd == 0 && s->log[s->nc] != 0) {
    s->nc += 1;
    s->flags |= CHREADY;
    return 1;
  }

  s->log[s->nc] |= upd;
  s->flags &= ~CHREADY;
 
  return 0;
}

#pragma vector=PORT1_VECTOR
#pragma vector=PORT2_VECTOR
#pragma vector=PORT3_VECTOR
#pragma vector=PORT4_VECTOR
__interrupt void pins() {
  if (keyEvent())
    __bic_SR_register_on_exit(LPM4_bits);
}
/*
interrupt(PORT2_VECTOR) p2() {
  if (keyEvent())
    __bic_SR_register_on_exit(LPM4_bits);
}
interrupt(PORT3_VECTOR) p3() {
  if (keyEvent())
    __bic_SR_register_on_exit(LPM4_bits);
}
interrupt(PORT4_VECTOR) p4() {
  if (keyEvent())
    __bic_SR_register_on_exit(LPM4_bits);
}
*/

#pragma vector=USCI_B0_VECTOR
__interrupt void i2c() {
   //switch(__even_in_range(UCB0IV,0x1E)) {
   switch (UCB0IV&0x1E) {
	case USCI_I2C_UCNACKIFG:
		s->i2nch+=5;
		/* Fall through */
	case USCI_I2C_UCTXIFG0:
             UCB0IFG &= ~UCTXIFG;
	     if (s->flags & I2CCSEND) {
		s->i2nch+=1;
		if (s->i2nch > 3) {
			s->lc = (s->lc+1) % LOG_MAX;
			s->i2nch = 0;
			s->flags ^= I2CCSEND;
			UCB0CTLW0 |= UCTXSTP;           // I2C stop condition
		} else {
			UCB0TXBUF = (unsigned char) (s->log[s->lc] >> (s->i2nch * 8));
		}
	     } else {
		UCB0CTLW0 |= UCTXSTP;           // I2C stop condition
	     }
	     break;
	case USCI_I2C_UCRXIFG0:
            	UCB0IFG &= ~UCRXIFG;
		if ((s->lc + 1) % LOG_MAX != s->nc) {
			s->i2nch = 0;
			s->flags |= I2CCSEND;
			UCB0TXBUF = (unsigned char) (s->log[s->lc] >> (s->i2nch * 8));
		} else {
			UCB0TXBUF = (unsigned char) 0x00;
		}
		
	default: break;
   }
}
