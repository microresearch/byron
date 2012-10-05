#include <avr/io.h>
#include <avr/interrupt.h>
#include "usbdrv/usbdrv.h"
#include "byron_cmds.h"
#define F_CPU       		 12000000

extern int ad_ConversionComplete (void);
extern int ad_Read10bit (void);
extern int ad_Read8bit (void);
extern void ad_SetChannel (uchar mux);
extern void ad_StartConversion ();

extern  void initCoreHardware(void);
extern void usbReset(void);

typedef unsigned char  u08;
typedef   signed char  s08;
typedef unsigned short u16;
typedef   signed short s16;

#ifndef BV
	#define BV(bit)			(1<<(bit))
#endif
#ifndef cbi
	#define cbi(reg,bit)	reg &= ~(BV(bit))
#endif
#ifndef sbi
	#define sbi(reg,bit)	reg |= (BV(bit))
#endif

#define ADC_PRESCALE_DIV2		0x00	///< 0x01,0x00 -> CPU clk/2
#define ADC_PRESCALE_DIV4		0x02	///< 0x02 -> CPU clk/4
#define ADC_PRESCALE_DIV8		0x03	///< 0x03 -> CPU clk/8
#define ADC_PRESCALE_DIV16		0x04	///< 0x04 -> CPU clk/16
#define ADC_PRESCALE_DIV32		0x05	///< 0x05 -> CPU clk/32
#define ADC_PRESCALE_DIV64		0x06	///< 0x06 -> CPU clk/64
#define ADC_PRESCALE_DIV128		0x07	///< 0x07 -> CPU clk/128
#define ADC_PRESCALE			ADC_PRESCALE_DIV64
#define ADC_PRESCALE_MASK		0x07
#define ADC_MUX_MASK			0x1F
