int GPIO_onRegister();

#define POS(m,n)	!!(n&((unsigned long)1<<m))

#define	POUNDP		16
#define	POUND(n)	POS(POUNDP,n)

#define	STARP		8
#define	STAR(n)		POS(STARP,n)

#define	SLP		17
#define	SL(n)		POS(SLP,n)

#define	TLP		18
#define	TL(n)		POS(TLP,n)

#define	KLP		0	
#define	KL(n)		POS(KLP,n)

#define	PLP		1
#define	PL(n)		POS(PLP,n)

#define	WLP		2
#define	WL(n)		POS(WLP,n)

#define	HLP		3
#define	HL(n)		POS(HLP,n)

#define	RLP		19
#define	RL(n)		POS(RLP,n)

#define	ALP		20
#define	AL(n)		POS(ALP,n)

#define	OLP		21
#define	OL(n)		POS(OLP,n)

#define	ERP		9
#define	ER(n)		POS(ERP,n)

#define	URP		29
#define	UR(n)		POS(URP,n)

#define	FRP		26
#define	FR(n)		POS(FRP,n)

#define	RRP		4
#define	RR(n)		POS(RRP,n)

#define	PRP		5
#define	PR(n)		POS(PRP,n)

#define	BRP		6
#define	BR(n)		POS(BRP,n)

#define	LRP		7
#define	LR(n)		POS(LRP,n)

#define	GRP		31
#define	GR(n)		POS(GRP,n)

#define	TRP		27
#define	TR(n)		POS(TRP,n)

#define	SRP		28
#define	SR(n)		POS(SRP,n)


#define	DRSP		22
#define	DRS(n)		POS(DRSP,n)

#define	ZRSP		23
#define	ZRS(n)		POS(ZRSP,n)


#define DRI2CP		24
#define	DRI2C(n)	POS(DRI2CP,n)

#define ZRI2CP		25
#define	ZRI2C(n)	POS(ZRI2CP,n)


#define FNP		30
#define	FN(n)		POS(FNP,n)

