//
//	328duino Internal RC Oscillator control library
//		(c) 2020	1YEN Toru
//


#ifdef		ARDUINO_AVR_328DUINO
#include	<Arduino.h>
#include	"328duino.h"
#ifdef		ARDUINO_AVR_328DUINO_DUE
#ifdef		A328DUINO_EEPCAL
#include	<EEPROM.h>
#endif	//	A328DUINO_EEPCAL
#endif	//	ARDUINO_AVR_328DUINO_DUE
#endif	//	ARDUINO_AVR_328DUINO


#ifdef		ARDUINO_AVR_328DUINO_DUE

A328duino	A328d;


static	uint16_t	f_osccal_0[]=
{
	// fcpu vs. OSCCAL value table
	// contents: (fcpu[MHz]<<8) + OSCCAL
// Attention! ********************************
// This is a empty table.
// You must calibrate your own ATmega328 Internal RC Oscillator by yourself.
// And over write your calibration table to here.
// Another case, you can specify A328DUINO_EEPCAL=<addr> macro,
//	and write calibration table to EEPROM[<addr>].
// see) 328duino.h header comment.
// ********************************
//	0x089d,								// 8MHz
//	0x0ac6,								// 10MHz
//	0x0ce3,								// 12MHz
//	0x0ef8,								// 14MHz
//	0x0fff,								// 15MHz
	0x0000								// end of table
};


A328duino::A328duino (void)
{
	// A328duino constructor

	// initialize
	errcod=A328dErrNo;
	fcpu_cur=A328dFcpuFactory;
	f_osccal=f_osccal_0;
	f_osccal_e=NULL;
	osccal_0=(*A328d_osccal);

#ifdef		A328DUINO_EEPCAL
	// read calibration table from EEPROM
	// size of table
	int		idx;
	int		siz;
	for (idx=A328DUINO_EEPCAL, siz=0; idx<E2END; idx += 2, siz++)
		if (EEPROM[idx]==0x00 || EEPROM[idx]==0xff)
			break;
	if (siz==0)
	{
		// ERROR:
		errcod=A328dErrBadEepcal;
	}
	else
	{
		// get memory
		f_osccal_e=new uint16_t[siz + 1];
		if (f_osccal==NULL)
		{
			// ERROR:
			errcod=A328dErrGetMem;
		}
		else
		{
			// read table
			int		adr;
			for (idx=0, adr=A328DUINO_EEPCAL; idx<siz; idx++, adr += 2)
				f_osccal_e[idx]=(EEPROM[adr]<<8) + EEPROM[adr + 1];		// BE
			f_osccal_e[idx]=0x0000;
			f_osccal=f_osccal_e;
		}
	}
#endif	//	A328DUINO_EEPCAL

	// search calibration data and set system clock
	if (errcod==A328dErrNo)
		errcod=setSysClock (F_CPU/1000000);
}


int		A328duino::setSysClock (
uint8_t		fcpu)
{
	// set system (cpu) clock
	// fcpu==0x00: restore OSCCAL to factory setting value
	// fcpu==0xff: returns current fcpu [MHz], fcpu not changed
	// return: A328dErrNo / A328dErrNotFound / fcpu_cur(>0)
	int		rtncod;
	int		idx;

	// parameter
	if (fcpu==A328dSscQueryCur)
	{
		// returns current system clock [MHz]
		return (fcpu_cur);
	}
	if (fcpu==A328dSscRstrFactry)
	{
		// restore OSCCAL to factory setting value
		(*A328d_osccal)=osccal_0;
		fcpu_cur=A328dFcpuFactory;
		return (A328dErrNo);
	}

	// search calibration data and set system clock
	rtncod=A328dErrNotFound;
	for (idx=0; f_osccal[idx]!=0x0000; idx++)
	{
		if ((f_osccal[idx]>>8)==fcpu)
		{
			// found
			rtncod=A328dErrNo;
			fcpu_cur=fcpu;
			(*A328d_osccal)=f_osccal[idx]&0xff;
			break;
		}
	}

	return (rtncod);
}


A328duino::~A328duino (void)
{
	// A328duino destructor

	// free memory
	if (f_osccal_e!=NULL)
		delete [] f_osccal_e;
}

#endif	//	ARDUINO_AVR_328DUINO_DUE


#ifdef		ARDUINO_AVR_328DUINO

uint32_t	_micros (void)
{
	unsigned long m;
	uint8_t oldSREG = SREG, t;
	extern	volatile	unsigned	long	timer0_overflow_count;

	cli();
	m = timer0_overflow_count;
#if defined(TCNT0)
	t = TCNT0;
#elif defined(TCNT0L)
	t = TCNT0L;
#else
	#error TIMER 0 not defined
#endif

#ifdef TIFR0
	if ((TIFR0 & _BV(TOV0)) && (t < 255))
		m++;
#else
	if ((TIFR & _BV(TOV0)) && (t < 255))
		m++;
#endif

	SREG = oldSREG;

	// for the frequency which is not an exponentiation of 2
	uint32_t	us;
	m=(m<<8) | t;

	// us=m<unsigned int 32>*<const fixed point 8>
	//		<const fixed point 8>=<unsigned int 4>.<frac 4>
	//		<unsigned int 4> must be 0b01xx or 0b10xx
#if			A328DUINO_SFT==6
	t=((64<<A328DUINO_SFT) + (F_CPU/1000000/2))/(F_CPU/1000000);
#else
	t=(64<<A328DUINO_SFT)/(F_CPU/1000000);
#endif
	us=m;								// b6
	if (t&0x80)
		us += m;						// b7
	for (t<<=2; t!=0; t<<=1)			// b5~b0
	{
		m>>=1;
		if (t&0x80)
			us += m;
	}

	return (us<<(6 - A328DUINO_SFT));
}

#endif	//	ARDUINO_AVR_328DUINO

