/* 
 * Originally from blink1-mini-tool
 *                     
 * Will work on small unix-based systems that have just libusb-0.1.4
 * No need for pthread & iconv, which is needed for hidapi-based tools
 * 
 * Known to work on:
 * - Ubuntu Linux
 * - Mac OS X 
 * - TomatoUSB WRT / OpenWrt / DD-WRT
 *
 * 2012, Tod E. Kurt, http://todbot.com/blog/ , http://thingm.com/
 *
 */

// taken from blink1/hardware/firmware/usbconfig.h
#define IDENT_VENDOR_NUM        0x27B8
#define IDENT_PRODUCT_NUM       0x01ED
#define IDENT_VENDOR_STRING     "ThingM"
#define IDENT_PRODUCT_STRING    "blink(1)"

#include "hiddata.h"

int blink1_open(usbDevice_t **dev);
char *blink1_error_msg(int errCode);
void blink1_close(usbDevice_t *dev);
int blink1_fadeToRGB(usbDevice_t *dev, int fadeMillis,
                     uint8_t r, uint8_t g, uint8_t b);
int blink1_setRGB(usbDevice_t *dev, uint8_t r, uint8_t g, uint8_t b );
//static int hexread(char *buffer, char *string, int buflen);
