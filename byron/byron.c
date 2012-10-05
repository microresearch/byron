#include <avr/wdt.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "usbdrv.h"
#include "byron.h"

int ad_ConversionComplete (void) {
	return (!(ADCSRA & (1 << ADSC)));
}

 int ad_Read10bit (void) {
 	return (ADCL | ADCH << 8);
 }
 
 int ad_Read8bit (void) {
 	return ad_Read10bit() >> 2;
 }
 
 void ad_SetChannel (uchar mux) {
 	ADMUX = (ADMUX & ~ADC_MUX_MASK) | (mux & ADC_MUX_MASK);
 }
 
 void ad_StartConversion () {
 			ADCSRA |= (1 << ADIF); 
			ADCSRA |= (1 << ADSC);
}

void usbReset(void) {
 	u08  i, j;
 	
	USBOUT &= ~USBMASK;	// make sure USB pins are pulled low 	
 	USBDDR |= USBMASK;	// set USB pins to output -> SE0
 	
    j = 0;
    while(--j) {       
        i = 0;
        while(--i);     // delay >10ms for USB reset
    }
 
    USBDDR &= ~USBMASK; // set USB pins to input
}

void initCoreHardware(void)
{
	// --------------------- Init AD Converter
  
	sbi(ADCSRA, ADEN);
	//		cbi(ADCSRA, ADATE);
	ADCSRA = ((ADCSRA & ~ADC_PRESCALE_MASK) | ADC_PRESCALE_DIV64);
	sbi(ADMUX,REFS0);cbi(ADMUX,REFS1);
	cbi(ADCSRA, ADLAR);
	//	sbi(ADCSRA, ADIE);
	cbi(ADCSRA, ADIE);
}
