#include "m_pd.h"
#include "../byron/byron_cmds.h"
#include </usr/include/usb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USBDEV_SHARED_VENDOR    	0x16C0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   	0x27D8  /* Obdev's free shared PID */
#define OUTLETS 					10
#define DEFAULT_CLOCK_INTERVAL		40

/* TODO:  */

/* test all */

typedef struct _byron			
{
  t_object p_ob;	
  usb_dev_handle *dev_handle;	
  void *m_clock;
  double m_interval;
  double m_interval_bak;
  int is_running;
  int do_10_bit;
  int debug_flag;
  void *outlets[OUTLETS];
  int values[10];
} t_byron;

void *byron_class;

void *byron_new(t_symbol *s);
void byron_assist(t_byron *x, void *b, long m, long a, char *s);
void byron_bang(t_byron *x);				
void byron_close(t_byron *x);
void byron_debug(t_byron *x,  long n);
void byron_int(t_byron *x,long n);
void byron_output(t_byron *x, t_floatarg nnn);
void byron_pwm(t_byron *x, t_symbol *s, t_floatarg nnn);
void byron_input(t_byron *x);
void byron_open(t_byron *x);
void byron_poll(t_byron *x, t_floatarg nnn);
void byron_smooth(t_byron *x, t_floatarg nnn);
void byron_start(t_byron *x);
void byron_stop(t_byron *x);

static int usbGetStringAscii(usb_dev_handle *dev, int ind, int langid, char *buf, int buflen);
void find_device(t_byron *x);

void byron_output(t_byron *x, t_floatarg nnn)
{
  int cmd, n;
  int nBytes;
  unsigned char buffer[8];
  n=(int)nnn;
  cmd = 0;
  cmd = BYRON_CMD_LOWER_OUT;
	
  if (n < 0) n = 0;
  if (n > 255) n = 255;
	
  if (!(x->dev_handle)) find_device(x);
  else {
    nBytes = usb_control_msg(x->dev_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, cmd, n, 0, (char *)buffer, sizeof(buffer), 10);
  }
}

void byron_pwm(t_byron *x, t_symbol *s, t_floatarg nnn)
{

  // TODO: fix so uses integer

  int cmd,n;
  int nBytes;
  unsigned char buffer[8];
  cmd=0;
  if (s == gensym("a"))	cmd = BYRON_CMD_SET_PWM0;
  else if (s == gensym("b"))	cmd = BYRON_CMD_SET_PWM1;
  else if (s == gensym("c"))	cmd = BYRON_CMD_SET_PWM2;
  else if (s == gensym("d"))	cmd = BYRON_CMD_SET_PWM3;

  else {
    post ("byron: unknown port\n");
    return;
  }
  n=(int)nnn;
  //  post("%d",n);

  if (n < 0) n = 0;
  if (n > 255) n = 255;
	
  if (!(x->dev_handle)) find_device(x);
  else {
    nBytes = usb_control_msg(x->dev_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, cmd, n, 0, (char *)buffer, sizeof(buffer), 10);
  }
}

void byron_input(t_byron *x)
{
  int cmd;
  int nBytes;
  unsigned char buffer[8];
	
  cmd = 0;
  cmd = BYRON_CMD_LOWER_IN;
	
  if (!(x->dev_handle)) find_device(x);
  else {
    nBytes = usb_control_msg(x->dev_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, cmd, 0, 0, (char *)buffer, sizeof(buffer), 10);
  }
}

void byron_debug(t_byron *x, long n)
{
  if (n)	x->debug_flag = 1;
  else 	x->debug_flag = 0;
}

void byron_bang(t_byron *x)
{
  int                 nBytes,i,n;
  int 				replymask,replyshift,replybyte;
  int					temp;
  unsigned char       buffer[12];
	
  if (!(x->dev_handle)) find_device(x);
  else {
    nBytes = usb_control_msg(x->dev_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, BYRON_CMD_POLL, 0, 0, (char *)buffer, sizeof(buffer), 10);
	
    if(nBytes < sizeof(buffer)){
      if (x->debug_flag) {
	if(nBytes < 0)
	  post( "USB error: %s\n", usb_strerror());
	post( "only %d bytes status received\n", nBytes);
      }
    } else {
      for (i = 0; i < OUTLETS; i++) {
	n = i;
	temp = buffer[n];
	// add 2 stuffed bits from end of buffer if we're doing 10bit precision
	if (n < 8) {
	  if (x->do_10_bit) {
	    if (n < 4)  replybyte = buffer[10];
	    else replybyte = buffer[11];
							
	    replyshift = ((n % 4) * 2);								replymask = (3 << replyshift);
							
	    temp = temp * 4 + ((replybyte & replymask) >> replyshift);	// add 2 LSB
							
	  }
	}
					
	if (x->values[i] != temp) {					// output if value has changed
	  //Max						outlet_int(x->outlets[i], temp);
	  outlet_float(x->outlets[i], temp);
	  x->values[i] = temp;
	}
      }
    }
  }
}

void byron_open(t_byron *x)
{
  if (x->dev_handle) {
    post("byron: There is already a connection to byron",0);
  } else find_device(x);
}

void byron_close(t_byron *x)
{
  if (x->dev_handle) {
    usb_close(x->dev_handle);
    x->dev_handle = NULL;
    post("byron: Closed connection to byron",0);
  } else
    post("byron: There was no open connection to byron",0);
}

void byron_poll(t_byron *x, t_floatarg nnn){
  int n=(int)nnn;
  if (n > 0) { 
    x->m_interval = n;
    x->m_interval_bak = n;
    byron_start(x);
  } else {
    byron_stop(x);
  }
}

void byron_smooth(t_byron *x, t_floatarg nnn) {
  int nBytes;
  unsigned char buffer[8];
  int n=(int)nnn;
  if (n < 0) n = 0;
  if (n > 15) n = 15;

  if (!(x->dev_handle)) find_device(x);
  else {
    nBytes = usb_control_msg(x->dev_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, BYRON_CMD_SET_SMOOTHING, n, 0, (char *)buffer, sizeof(buffer), 10);
  }

}

void byron_int(t_byron *x,long n) {
  if (n) {
    if (!x->is_running) byron_start(x);
  } else {
    if (x->is_running) byron_stop(x);
  }
}

void byron_start (t_byron *x) { 
  if (!x->is_running) {
    clock_delay(x->m_clock,0.);
    x->is_running  = 1;
  }
} 

void byron_stop (t_byron *x) { 
  if (x->is_running) {
    x->is_running  = 0;
    clock_unset(x->m_clock); 
    byron_close(x);
  }
} 

void byron_tick(t_byron *x) { 
  clock_delay(x->m_clock, x->m_interval);
  byron_bang(x);
} 

int byron_setup(void)
{
  byron_class = class_new ( gensym("byron"),(t_newmethod)byron_new, 0, sizeof(t_byron),CLASS_DEFAULT,0);
  class_addbang(byron_class, (t_method)byron_bang);
  class_addfloat(byron_class, (t_method)byron_int);
  class_addmethod(byron_class, (t_method)byron_debug,gensym("debug"), A_DEFSYM, A_DEFFLOAT, 0);
  class_addmethod(byron_class, (t_method)byron_pwm,gensym("pwm"), A_DEFSYM, A_DEFFLOAT, 0);
  class_addmethod(byron_class, (t_method)byron_open, gensym("open"), 0);		
  class_addmethod(byron_class, (t_method)byron_close, gensym("close"), 0);	
  class_addmethod(byron_class, (t_method)byron_poll, gensym("poll"), A_DEFFLOAT,0);	
  class_addmethod(byron_class, (t_method)byron_output, gensym("output"), A_DEFFLOAT,0);	
  class_addmethod(byron_class, (t_method)byron_input, gensym("input"),0);	
  class_addmethod(byron_class, (t_method)byron_smooth, gensym("smooth"), A_DEFFLOAT,0);	
  class_addmethod(byron_class, (t_method)byron_start, gensym("start"), 0);	
  class_addmethod(byron_class, (t_method)byron_stop, gensym("stop"), 0);	

  post("byron version 0.1 - (c) 2010 [ x x x x x ]",0);
  usb_init();
  return 1;
}

//--------------------------------------------------------------------------

void *byron_new(t_symbol *s)
{
  t_byron *x;									
  x = (t_byron *)pd_new(byron_class);
  x->m_clock = clock_new(x,(t_method)byron_tick);
 
  x->do_10_bit = 1;
	
  x->m_interval = DEFAULT_CLOCK_INTERVAL;
  x->m_interval_bak = DEFAULT_CLOCK_INTERVAL;
  x->debug_flag = 0;
  x->dev_handle = NULL;
  int i;

  for (i=0; i < OUTLETS; i++) {
    x->outlets[i] = outlet_new(&x->p_ob, &s_float);
  }	
  return x;
}

void byron_free(t_byron *x)
{
  if (x->dev_handle) usb_close(x->dev_handle);
  freebytes((t_object *)x->m_clock, sizeof(x->m_clock));
}


static int  usbGetStringAscii(usb_dev_handle *dev, int ind, int langid, char *buf, int buflen)
{
  char    buffer[256];
  int     rval, i;

  if((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + ind, langid, buffer, sizeof(buffer), 1000)) < 0)
    return rval;
  if(buffer[1] != USB_DT_STRING)
    return 0;
  if((unsigned char)buffer[0] < rval)
    rval = (unsigned char)buffer[0];
  rval /= 2;
  /* lossy conversion to ISO Latin1 */
  for(i=1;i<rval;i++){
    if(i > buflen)  /* destination buffer overflow */
      break;
    buf[i-1] = buffer[2 * i];
    if(buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
      buf[i-1] = '?';
  }
  buf[i-1] = 0;
  return i-1;
}

void find_device(t_byron *x)
{
  usb_dev_handle      *handle = NULL;
  struct usb_bus      *bus;
  struct usb_device   *dev;
	
  //  post("byron: here");
  usb_find_busses();
  usb_find_devices();
  for(bus=usb_busses; bus ; bus=bus->next){
    //    post("byron: herexxx");
    for(dev=bus->devices; dev; dev=dev->next){
      if(dev->descriptor.idVendor == USBDEV_SHARED_VENDOR && dev->descriptor.idProduct == USBDEV_SHARED_PRODUCT){
	char    string[256];
	int     len;
	//	post("byron: found one");
	handle = usb_open(dev);
	if(!handle){
	  post("Warning: cannot open USB device: %s", usb_strerror());
	  continue;
	}
	len = usbGetStringAscii(handle, dev->descriptor.iManufacturer, 0x0409, string, sizeof(string));
	if(len < 0){
	  post("byron: warning: cannot query manufacturer for device: %s", usb_strerror());
	  goto skipDevice;
	}

	if(strcmp(string, "www.1010.co.uk") != 0)
	  {
	    post("errxxx\n");
	    goto skipDevice;
	  }

	len = usbGetStringAscii(handle, dev->descriptor.iProduct, 0x0409, string, sizeof(string));
	if(len < 0){
	  post("byron: warning: cannot query product for device: %s", usb_strerror());
	  goto skipDevice;
	}
	if(strcmp(string, "byron") == 0)
	  break;
      skipDevice:
	post("skipped\n");
	usb_close(handle);
	handle = NULL;
      }
    }
    if(handle)
      break;
  }
	
  if(!handle){
    post("byron: Could not find USB device byron");
    x->dev_handle = NULL;
    if (x->m_interval < 10000) x->m_interval *=2;
  } else {
    x->dev_handle = handle;
    post("byron: Found USB device byron");
    x->m_interval = x->m_interval_bak;		
    if (x->is_running) byron_tick(x);
    else byron_bang(x);
  }
}
