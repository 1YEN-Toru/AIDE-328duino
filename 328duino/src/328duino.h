//
//	328duino Internal RC Oscillator control library
//		(c) 2020	1YEN Toru
//
//
//	OSCCAL table on the EEPROM:
//		1) create file:
//			<Arduino>\hardware\arduino\avr\platform.local.txt
//		2) add platform.local.txt
//			compiler.cpp.extra_flags=-DA328DUINO_EEPCAL=<addr>
//		3) write OSCCAL table to EEPROM.
//			EEPROM[<addr> + 2*i + 0]: frequency[MHz]
//			EEPROM[<addr> + 2*i + 1]: OSCCAL value
//				:	repeat i as much as necessary (i=0~max)
//			EEPROM[<addr> + 2*max]: 0x00 or 0xff (end of table)
//		4) frequency[MHz] must be a range of 1~15[MHz]
//
//	*) I checked only frequencies described underneath.
//		238duino form#1(uno): 20, 24[MHz]
//		238duino form#2(due): RC 8, 10, 12, 14, 15[MHz]
//
//	defined:
//		ARDUINO_AVR_328DUINO_UNO for 328duino form#1(uno), XTAL
//		ARDUINO_AVR_328DUINO_DUE for 328duino form#2(due), RC
//
//
//	2020/04/29	ver.1.10
//		corresponding to 328duino form#1(uno)
//		const int A328dMicrosErrIden; ... Iden>=1
//		const int A328dMicrosErrInum; ... Inum=-1, 0, 1
//			ideal.micros/_micros()=1 + Inum/Iden
//		const float A328dMicrosErr=ideal.micros/_micros();
//		#define	A328DUINO_ERR A328dMicrosErr
//
//	2020/03/14	ver.1.00
//


#ifndef		A328DUINO_h
#define		A328DUINO_h		"1.10"


#include	<inttypes.h>


#ifdef		ARDUINO_AVR_328DUINO
#if			F_CPU>=16000000L
#define		ARDUINO_AVR_328DUINO_UNO
#else
#define		ARDUINO_AVR_328DUINO_DUE
#endif
#endif	//	ARDUINO_AVR_328DUINO

#if			F_CPU<=16000000L
#define		A328DUINO_SFT	4
#elif		F_CPU==25000000L
#define		A328DUINO_SFT	6
#else
#define		A328DUINO_SFT	5
#endif
#define		A328DUINO_CHK	(((64<<A328DUINO_SFT)/(F_CPU/1000000))>>6)
#if			A328DUINO_CHK!=1 && A328DUINO_CHK!=2
#error		"328duino: unsupported F_CPU"
#endif

#if			A328DUINO_SFT==4 || A328DUINO_SFT==5
const	float	A328dMicrosErr=(64./(F_CPU/1000000))/
							(float ((64<<A328DUINO_SFT)/
							(F_CPU/1000000))/(1<<A328DUINO_SFT));
#elif		A328DUINO_SFT==6
const	float	A328dMicrosErr=(64./(F_CPU/1000000))/
							(float (((64<<A328DUINO_SFT) + (F_CPU/2000000))/
							(F_CPU/1000000))/(1<<A328DUINO_SFT));
#endif
// error: ideal.micros/_micros()=Err
#define		A328DUINO_ERR	A328dMicrosErr
// error: ideal.micros/_micros()=1 + Inum/Iden, Inum=-1/0/1
const	int		A328dMicrosErrIden=(abs (A328dMicrosErr - 1.)<1./32767)? 1:
									int (abs (1./(A328dMicrosErr - 1.)) + 0.5);
const	int		A328dMicrosErrInum=(abs (A328dMicrosErr - 1.)<1./32767)? 0:
									(A328dMicrosErr - 1.<0.)? -1: 1;


#ifdef		ARDUINO_AVR_328DUINO_DUE


// memory mapped i/o registers
volatile	uint8_t		*const	A328d_osccal=(uint8_t *)0x0066;


enum	A328dParam
{
	A328dFcpuFactory=8,					// factory setting frequency [MHz]
	A328dSscRstrFactry=0x00,			// SetSysClock(): restore factory calib.
	A328dSscQueryCur=0xff,				// SetSysClock(): query current fcpu
	// error code
	A328dErrNo=0,						// no error
	A328dErrNotFound=-1,				// calibration data not found
	A328dErrGetMem=-2,					// could not allocate memory
	A328dErrBadEepcal=-3,				// bad table in EEPROM
};


class	A328duino
{
	public:

	A328duino (void);
	~A328duino (void);

	int		setSysClock (uint8_t fcpu=A328dSscRstrFactry);

	int		errcod;						// error code
	uint8_t		osccal_0;				// factory OSCCAL value
	uint8_t		fcpu_cur;				// current fcpu [MHz]
	uint16_t	*f_osccal;				// pointer to calibration table
	uint16_t	*f_osccal_e;			// calibration table from EEPROM
};


extern	A328duino	A328d;


#endif	//	ARDUINO_AVR_328DUINO_DUE


#ifdef		ARDUINO_AVR_328DUINO

uint32_t	_micros (void);

#define		micros()	_micros ()

#else	//	ARDUINO_AVR_328DUINO

inline	uint32_t	_micros (void)
{
	return (micros ());
}

#endif	//	ARDUINO_AVR_328DUINO


#endif	//	A328DUINO_h

