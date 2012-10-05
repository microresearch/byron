/* Name: byronjump.c
 * Project: byronjump: jump into bootloader from differing firmwares
 * Author: Martin Howse after ...
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>        /* this is libusb */
#include "opendevice.h" /* common code moved to separate module */

#include "../../byron/byron_cmds.h"   /* custom request numbers */
#include "../../byron/usbconfig.h"  /* for  names only*/

/* what about other commands?*/

static void usage(char *name)
{
  fprintf(stderr, "usage:\n");
}

int main(int argc, char **argv)
{

  /* need to handle VID/HID pairs as follows 
     all VID 0x16c0 

     PID: 

     HID joy/keyb: 0x27db
     BYRON/usbasp: 0x27d8

     and what about VENDOR_NAME = 'w', 'w', 'w', '.', '1', '0', '1', '0', '.', 'c', 'o', '.', 'u', 'k'


     and || DEVICE_NAME?

     HID: 'x', 'x', 'x', 'x', 'x', '-', 'a', 'v', 'r'
     BYRON: 'b', 'y', 'r', 'o', 'n'
     USBASP: 'U', 'S', 'B', 'a', 's', 'p'
     KEYBOARD: #define USB_CFG_DEVICE_NAME     'H', 'I', 'D', 'K', 'e', 'y', 's'

     and I guess we should also have another for custom code

  */

  usb_dev_handle      *handle = NULL;
  unsigned char rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};

  /* TODO: allocate for these */

  char vendor[24]; char product[24];

  char                buffer[4];
  int                 cnt, vid, pid;

  usb_init();
  /* compute VID/PID from usbconfig.h so that there is a central source of information */
  vid = rawVid[1] * 256 + rawVid[0];
  pid = rawPid[1] * 256 + rawPid[0];
  /* The following function is in opendevice.c: */

  /* try Byron first */

  //  vendor[] = {USB_CFG_VENDOR_NAME, 0}; product[] = {USB_CFG_DEVICE_NAME, 0};
  strcpy(vendor, "www.1010.co.uk");
  strcpy(product, "byron");

  if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
    fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
    //        exit(1);
	
    /* try HID */
    pid= 0x27*256 + 0xdb;
    strcpy(product, "xxxxx-avr");

    if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
      fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
    
      /* try USBASP */

      strcpy(vendor, "www.fischl.de");
      pid= 0x05*256 + 0xdc;
      strcpy(product, "USBasp");

      if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
        fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);

	/* try KEYBOARD */
	strcpy(vendor, "www.1010.co.uk");

	pid= 0x27*256 + 0xdb; 
	strcpy(product, "HIDKeys");

	if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
	  fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);

	/* try MIDI */

	strcpy(vendor, "www.1010.co.uk");
	pid= 0x05*256 + 0xe4;
	strcpy(product, "AVR-MIDI");

	if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
	  fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
	  	  exit(1);
	}


	}
      }
    }
  }

  /* Since we use only control endpoint 0, we don't need to choose a
   * configuration and interface. Reading device descriptor and setting a
   * configuration and interface is done through endpoint 0 after all.
   * However, newer versions of Linux require that we claim an interface
   * even for endpoint 0. Enable the following code if your operating system
   * needs it: */
#if 0
  int retries = 1, usbConfiguration = 1, usbInterface = 0;
  if(usb_set_configuration(handle, usbConfiguration) && showWarnings){
    fprintf(stderr, "Warning: could not set configuration: %s\n", usb_strerror());
  }
  /* now try to claim the interface and detach the kernel HID driver on
   * Linux and other operating systems which support the call. */
  while((len = usb_claim_interface(handle, usbInterface)) != 0 && retries-- > 0){
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
    if(usb_detach_kernel_driver_np(handle, 0) < 0 && showWarnings){
      fprintf(stderr, "Warning: could not detach kernel driver: %s\n", usb_strerror());
    }
#endif
  }
#endif
  cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,BYRON_START_BOOTLOADER, 0, 0, buffer, sizeof(buffer), 10);

  usb_close(handle);
  return 0;
}
