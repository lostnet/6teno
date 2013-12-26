// include <msp430.h>
#include <msp430fr5739.h>
//include <signal.h>

#define I2CP (BIT6|BIT7)
#define PADA_M (~I2CP)
// define PADA_M (~0x0)
#define PADB_M ~(BITF|BITE|BITD|BITC|BITB|BITA)

#define NC_ 0xCB00
#define NC ((unsigned int volatile *)NC_)

#define LC_ 0xCB10
#define LC ((unsigned int volatile *)LC_)

#define I2NCH_ 0xCB20
#define I2NCH ((unsigned char volatile *)I2NCH_)

#define RLOG_ 0xCB80
#define RLOG ((unsigned long volatile *)(RLOG_))

#define MC 0
#define RCHRD 2
#define IC 4
#define RC 6
#define WK 8


#define LOG_ 0xCC00
#define LOG ((unsigned long volatile *)(LOG_))
// 4kb/1kc log for testing
#define LOG_MAX 1024

// FLAGS
#define FLGS_ 0xCBF0
#define FLGS ((unsigned int volatile *)FLGS_)
#define CHREADY 1
#define STRT 2
#define R2NOTIFY 4
#define I2CCSEND 8

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


  if ((*NC >= LOG_MAX) || (LOG[*NC] != 0)) {
  	*NC = (*NC+1) % LOG_MAX;
   	LOG[*NC] = 0;
  }
  if ((*LC >= LOG_MAX) || (*LC == *NC)) {
	*LC = *NC ? *NC-1:LOG_MAX-1;
	*I2NCH = 0;
  }

   RLOG[MC] = 1;
   RLOG[RC] += 1;
   RLOG[WK] = 0xafdeafde;
}

void lp () {
  unsigned long volatile *p;
   while(1) {

    if (!(*FLGS & (CHREADY|R2NOTIFY))) {
	slp();
   	RLOG[WK] = 0xedfecefa;
    }

    __delay_cycles(20000); // 20 MS?
    if (*FLGS & R2NOTIFY) {
	*FLGS &= ~R2NOTIFY;
	PJOUT &= ~BIT0;
    }

    if (*FLGS & CHREADY) {
        *FLGS &= ~CHREADY;
   	*NC = (*NC+1) % LOG_MAX;
   	p = &LOG[*NC];
   	*p = 0;
	PJOUT |= BIT0;
        *FLGS |= R2NOTIFY;
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


  if ((*FLGS & CHREADY) == 0)
	setp();
  __enable_interrupt();
  RLOG[MC] += 1;
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

  unsigned long volatile *p;
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

  RLOG[IC]+=1;

  PAIES = (PADA_M & PAIN) | (PAIES & ~PADA_M);
  PBIES = (PADB_M & PBIN) | (PBIES & ~PADB_M);

  PAIFG &= ~PADA_M; // clear flags
  PBIFG &= ~PADB_M;

  upd = (PAIN & PADA_M);
  upd = upd<<16;
  upd |= (PBIN & PADB_M);
  RLOG[RCHRD] |= upd;

  p = &LOG[*NC];
  if (upd == 0 && *p != 0) {
    *FLGS |= CHREADY;
    return 1;
  }

  *p |= upd;
  *FLGS &= ~CHREADY;
 
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
		*I2NCH+=5;
		/* Fall through */
	case USCI_I2C_UCTXIFG0:
             UCB0IFG &= ~UCTXIFG;
	     if (*FLGS & I2CCSEND) {
		*I2NCH+=1;
		if (*I2NCH > 3) {
			*LC = (*LC+1) % LOG_MAX;
			*I2NCH = 0;
			*FLGS ^= I2CCSEND;
			UCB0CTLW0 |= UCTXSTP;           // I2C stop condition
		} else {
			UCB0TXBUF = (unsigned char) (LOG[*LC] >> (*I2NCH * 8));
		}
	     } else {
		UCB0CTLW0 |= UCTXSTP;           // I2C stop condition
	     }
	     break;
	case USCI_I2C_UCRXIFG0:
            	UCB0IFG &= ~UCRXIFG;
		if ((*LC + 1) % LOG_MAX != *NC) {
			*I2NCH = 0;
			*FLGS |= I2CCSEND;
			UCB0TXBUF = (unsigned char) (LOG[*LC] >> (*I2NCH * 8));
		} else {
			UCB0TXBUF = (unsigned char) 0x00;
		}
		
	default: break;
   }
}
