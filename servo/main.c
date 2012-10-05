/* For maximum 8 servos as well as ADC input */

/* servo pins are:

byron pins (bottom row is:)

GND, PD0, PD1, PB1, PB2, PB3, PB0, PD5, PD6, PD7

notes:

- keep ADC as input
- shall we way 4 servos on PB0-PB3 and keep PD pins as digital inputs

refs:

*/



#include "byron.h"
#include <stdio.h>
#include <inttypes.h>
#include <avr/wdt.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>

unsigned long __attribute__ ((section (".BOOTSTART"))) bootStart;

#define F_OSC 12000000
#define UART_BAUD_RATE 9600
#define ADC_PAUSE 		10
#define LED_KEEP_ALIVE	100  
#define USB_REPLY_BBBB 8
#define USB_REPLY_DDDD 9	
#define DDR(x) ((x)-1)

static u08		ad_mux;
static u16		ad_values[8];
static u08		ad_smoothing;
static u08		ad_samplepause;
static u08		usb_reply[12];
static unsigned char lastvalue;

SIGNAL (TIMER1_COMPA_vect) 
{ 

  // deal with the servos

}

uchar usbFunctionSetup(uchar data[8])
{

  // 4 servos
	
  switch (data[1]) {
	  
  case BYRON_START_BOOTLOADER: // ==14
    cli();
    USB_INTR_ENABLE = 0;
    USB_INTR_CFG = 0;       /* also reset config bits */
    GICR = (1 << IVCE);  /* enable change of interrupt vectors */
    GICR = (1 << IVSEL); /* move interrupts to boot flash section */

    bootStart = 0x55AA1177;      // Bootloader start from application
    for(;;);   // Reset from watchdog
    break;

		
  case BYRON_CMD_POLL:    
    usbMsgPtr = usb_reply;
    return sizeof(usb_reply);
    break;

  case BYRON_CMD_SET_SMOOTHING:		
    if (data[2] > 15) ad_smoothing = 15;
    else ad_smoothing = data[2];
    break;

  case BYRON_CMD_LOWER_IN:
    // PD 0 1 5 6 7
    // NOT-pullups are here
    DDRD &= ~(0xE3);
    break;

  case BYRON_CMD_LOWER_OUT:
    // PD 0 1 5 6 7 
    DDRD |= 0xE3;
    //    PORTD = (PORTD&0x1F) | ((data[2]>>1)&7)<<5 ; //CHANGE! 0, 1, 5,6,7
    usb_reply[USB_REPLY_DDDD] = data[2];
    break;
			
  case BYRON_CMD_SET_SERVO0:
    usb_reply[USB_REPLY_BBBB] = data[2];
    break;

  case BYRON_CMD_SET_SERVO1:
    usb_reply[USB_REPLY_BBBB] = data[2];
    break;

  case BYRON_CMD_SET_SERVO2:
    usb_reply[USB_REPLY_BBBB] = data[2];
    break;

  case BYRON_CMD_SET_SERVO3:
    usb_reply[USB_REPLY_BBBB] = data[2];
  } 
  return 0;
}

void checkAnlogPorts (void) {
  unsigned int temp,replymask,replyshift,replybyte;
	
  if (ad_samplepause != 0xff) {													
    if (ad_samplepause < ADC_PAUSE) {
      ad_samplepause++;
    } else {
      ad_StartConversion();
      ad_samplepause = 0xff;
    }
  } else {

    if ( ad_ConversionComplete() ) {								temp = ad_Read10bit();									ad_values[ad_mux] = (ad_values[ad_mux] * ad_smoothing + temp) / (ad_smoothing + 1);
      usb_reply[ad_mux] = ad_values[ad_mux] >> 2;
      replybyte = 10 + (ad_mux / 4);	
      replyshift = ((ad_mux % 4) * 2);
      replymask = (3 << replyshift);	
      usb_reply[replybyte] =	
	(usb_reply[replybyte] & ~replymask) | (replymask & (ad_values[ad_mux] << replyshift));
	
      ad_mux = (ad_mux + 1) % 8;
      ad_SetChannel(ad_mux);
      ad_samplepause = 0;									
    }
  }
}

void checkDigitalPorts(void) {
  int state;
  if ((DDRD&0x01)==0x00) {
    // PD 0 1 5 6 7

    state=((PIND&0x20)>>4) + ((PIND&0x40)>>4) + ((PIND&0x80)>>4); // CHANGE!
    usb_reply[USB_REPLY_DDDD]=state;
  }
}

int main(void)
{
  long count; uchar i,j;

  GICR = _BV(IVCE);
  GICR = 0x00;

  bootStart = 0x00000000;
  DDRC 	= 0x00;
  PORTC = 0x00;

  wdt_enable(WDTO_1S);
  usbInit();
  usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
  i = 0;
  while(--i){             /* fake USB disconnect for > 250 ms */
    wdt_reset();
    _delay_ms(1);
  }
  usbDeviceConnect();
  sei();

  initCoreHardware(); // now just for adc - clean up all

  // servo timer

    TCCR1A = 0;             // normal counting mode 
    TCCR1B = _BV(CS11);     // set prescaler of 8 
    TCNT1 = 0;              // clear the timer count 
    TIFR = _BV(OCF1A);     // clear any pending interrupts; 
    TIMSK =  _BV(OCIE1A) ; // enable the output compare interrupt	  


  while(1) {
    usbPoll();
    checkAnlogPorts();
    checkDigitalPorts();
    wdt_reset();
  }
  return 0;
}

