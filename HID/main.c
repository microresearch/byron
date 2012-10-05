//xxxxx-hid v. 0.02
//GNU GPL v2 - See the OBDEV license for further details.  (http://www.obdev.at/avrusb/)

#include "global.h"
#include "a2d.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "usbdrv.h"

/* for bootloadjump */

#define BYRON_START_BOOTLOADER 	14
unsigned long __attribute__ ((section (".BOOTSTART"))) bootStart;

#define HEX__(n) 0x##n##UL

#define B8__(x) ((x&0x0000000FLU)?1:0)  \
  +((x&0x000000F0LU)?2:0)  \
  +((x&0x00000F00LU)?4:0)  \
  +((x&0x0000F000LU)?8:0)  \
  +((x&0x000F0000LU)?16:0) \
  +((x&0x00F00000LU)?32:0) \
  +((x&0x0F000000LU)?64:0) \
  +((x&0xF0000000LU)?128:0)

#define B8(d) ((unsigned char)B8__(HEX__(d)))


static uchar reportBuffer[8];				/* buffer for HID reports */
static uchar idleRate;						/* in 4 ms units */
uchar changed= 0;

/*

PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { //USB report descriptor
	0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
	0x09, 0x04,        // USAGE (Joystick)
	0xa1, 0x01,        // COLLECTION (Application)
	
	0x09, 0x30,        // USAGE (shift)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,  // LOGICAL_MAXIMUM (255)
	0x75, 0x08,        // REPORT_SIZE (8)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)
	
	0x09, 0x31,        // USAGE (enc0)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,  // LOGICAL_MAXIMUM (255)
	0x75, 0x08,        // REPORT_SIZE (8)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)
	
	0x09, 0x32,        // USAGE (enc1)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,  // LOGICAL_MAXIMUM (255)
	0x75, 0x08,        // REPORT_SIZE (8)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)
	
	0x09, 0x33,        // USAGE (enc2)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,  // LOGICAL_MAXIMUM (255)
	0x75, 0x08,        // REPORT_SIZE (8)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)
	
	0x09, 0x34,        // USAGE (enc3)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,  // LOGICAL_MAXIMUM (255)
	0x75, 0x08,        // REPORT_SIZE (8)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)
	
	0x09, 0x35,        // USAGE (enc4)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,  // LOGICAL_MAXIMUM (255)
	0x75, 0x08,        // REPORT_SIZE (8)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)
	
	0x09, 0x36,        // USAGE (pot0)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,  // LOGICAL_MAXIMUM (255)
	0x75, 0x08,        // REPORT_SIZE (8)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)
	
	0x09, 0x37,        // USAGE (pot1)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,  // LOGICAL_MAXIMUM (255)
	0x75, 0x08,        // REPORT_SIZE (8)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)
	
	0xc0               // END_COLLECTION
};

*/


PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { //USB report descriptor
	0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
	0x09, 0x04,        // USAGE (Joystick)
	0xa1, 0x01,        // COLLECTION (Application)

	0x09, 0x30,        // USAGE (shift1)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x03, 	           //	  Logical Maximum (1024)
	0x75, 0x0a,        // REPORT_SIZE (10)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)

	0x09, 0x31,        // USAGE (shift1)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x03, 	           //	  Logical Maximum (1024)
	0x75, 0x0a,        // REPORT_SIZE (10)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)

	0x09, 0x32,        // USAGE (shift1)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x03, 	           //	  Logical Maximum (1024)
	0x75, 0x0a,        // REPORT_SIZE (10)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)

	0x09, 0x33,        // USAGE (shift1)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x03, 	           //	  Logical Maximum (1024)
	0x75, 0x0a,        // REPORT_SIZE (10)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)

	0x09, 0x34,        // USAGE (shift1)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x03, 	           //	  Logical Maximum (1024)
	0x75, 0x0a,        // REPORT_SIZE (10)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)

	0x09, 0x35,        // USAGE (shift1)
	0x15, 0x00,        // LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x03, 	           //	  Logical Maximum (1024)
	0x75, 0x0a,        // REPORT_SIZE (10)
	0x95, 0x01,        // REPORT_COUNT (1)
	0x81, 0x02,        // INPUT (Data,Var,Abs)
	
	0x05, 0x09,			   //   Usage_Page (Button)
	0x19, 0x01,			   //   Usage_Minimum (Button 1)
	0x29, 0x04,			   //   Usage_Maximum (Button 8)
	0x15, 0x00,			   //   Logical_Minimum (0)
	0x25, 0x01,			   //   Logical_Maximum (1)
	0x75, 0x01,			   //   Report_Size (1)
	0x95, 0x04,			   //   Report_Count (4)
	0x55, 0x00,			   //   Unit_Exponent (0)
	0x65, 0x00,			   //   Unit (None)
	0x81, 0x02,			   //   Input (Data, Const, Abs)
			
	0xc0               // END_COLLECTION
	};

/*
char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH]
PROGMEM = {
  0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,                    // USAGE (Joystick)
  0xa1, 0x01,                    // COLLECTION (Application)
  0x95, 0x06,                    //   REPORT_COUNT (6)
  0x75, 0x0a,                    //     REPORT_SIZE (10)
  0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
  0x26, 0xfe, 0x03,              //     LOGICAL_MAXIMUM (1023)
  0x09, 0x30,                    //     USAGE (X,yetc)
  0x81, 0x02,                    //     INPUT (Data,Var,Abs)

  0x05, 0x09,			   //   Usage_Page (Button)
  0x19, 0x01,			   //   Usage_Minimum (Button 1)
  0x29, 0x04,			   //   Usage_Maximum (Button 8)
  0x15, 0x00,			   //   Logical_Minimum (0)
  0x25, 0x01,			   //   Logical_Maximum (1)
  0x75, 0x01,			   //   Report_Size (1)
  0x95, 0x04,			   //   Report_Count (4)
  0x55, 0x00,			   //   Unit_Exponent (0)
  0x65, 0x00,			   //   Unit (None)
  0x81, 0x02,			   //   Input (Data, Const, Abs)
  0xc0                           // END_COLLECTION               
  };*/ 


uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data;

    usbMsgPtr = reportBuffer;
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
       
	reportBuffer[0]= 0;
	reportBuffer[1]= 0;
	reportBuffer[2]= 0;
	reportBuffer[3]= 0;
	reportBuffer[4]= 0;
	reportBuffer[5]= 0;
	reportBuffer[6]= 0;
	reportBuffer[7]= 0;
	
            return sizeof(reportBuffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }

	}
	else if (rq->bRequest == BYRON_START_BOOTLOADER){
	  cli();
	  USB_INTR_ENABLE = 0;
	  USB_INTR_CFG = 0;       /* also reset config bits */
	  GICR = (1 << IVCE);  /* enable change of interrupt vectors */
	  GICR = (1 << IVSEL); /* move interrupts to boot flash section */

	  bootStart = 0x55AA1177;      // Bootloader start from application
	  for(;;);   // Reset from watchdog
	}
	return 0;
}

int main(void) {

	uchar	i, j;

	//	wdt_enable(WDTO_2S);
	//	odDebugInit();
	DDRB = 0;		/* all inputs */
	PORTB = 15;
	PORTD = 3;			/* no pullups on USB pins */
	DDRD = ~0;			/* output SE0 for USB reset */

  /* bootloader */
  
  GICR = _BV(IVCE);
  GICR = 0x00;

  bootStart = 0x00000000;
  wdt_enable(WDTO_1S);

	j = 0;
	while(--j){			/* USB Reset by device only required on Watchdog Reset */
		i = 0;
		while(--i);		/* delay >10ms for USB reset */
	}
	
	DDRD = ~(USBMASK + 7) ;		/* all outputs except USB data and PD0/1/2INT*/
	TCCR0 = 5;			/* set prescaler to 1/1024 */

	a2dInit();			/* Initialise analog-to-digital conversion */

	usbInit();			/* Initialise USB */
	sei();				/* Enable interrupts */

	
	unsigned char currentA2DChannel = 0;
	unsigned short a2dData[6] = {0,0,0,0,0,0};

	for(;;){	/* main event loop */
		wdt_reset();
		usbPoll();

		unsigned short a2dValue;
		uchar high;
		uchar low;
		uchar temp;
		uchar xxxxx;
		
		uchar reportBuffer[8];
		
		/* Get value of current a2d channel */
		a2dData[currentA2DChannel] = a2dConvert10bit(currentA2DChannel);
		
		currentA2DChannel++;

		if(currentA2DChannel>5) 
			currentA2DChannel = 0;


		/* The following steps convert the a2d values stored in the a2dData array into
		   the format required for sending in a HID Report packet. This converted data
		   is placed in the reply buffer and usbSetInterrupt is called to notify the
		   system to send the data in this buffer on the next USB interrupt */

		a2dValue = a2dData[0];
		high = a2dValue >> 8;
		low = a2dValue & 255;
		reportBuffer[0] = low;
		temp = high;

		a2dValue = a2dData[1];
		high = a2dValue >> 6;
		low = a2dValue & 63;
		reportBuffer[1] = (low << 2) + temp;
		temp = high;

		a2dValue = a2dData[2];
		high = a2dValue >> 4;
		low = a2dValue & 15;
		reportBuffer[2] = (low << 4) + temp;
		temp = high;

		a2dValue = a2dData[3];
		high = a2dValue >> 2;
		low = a2dValue & 3;
		reportBuffer[3] = (low << 6) + temp;
		temp = high;

		high = 0;
		low = 0;
		reportBuffer[4] = temp;
		temp = high;

		a2dValue = a2dData[4];
		high = a2dValue >> 8;
		low = a2dValue & 255;
		reportBuffer[5] = low + temp;
		temp = high;

		a2dValue = a2dData[5];
		high = a2dValue >> 6;
		low = a2dValue & 63;
		reportBuffer[6] = (low << 2) + temp;
		temp = high;

		// 4 buttons
		
		reportBuffer[7] = (temp & 15) + ((PIND & 2) << 6) +  ((PIND & 1) << 6) +  ((PINB & 2) << 4) +  ((PINB & 4) << 2);

		//	usbSetInterrupt(reportBuffer, 8);
			
		//--send report
		if(usbInterruptIsReady()) {
		  usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
		  //			changed= 0;


		  /*	if(TIFR & (1 << TOV0)){
			TIFR |= 1 << TOV0;
			}*/

		}
	}	
	return 0;
}
