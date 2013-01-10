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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>    // for memset() et al
#include <stdint.h>    // for uint8_t
#include <unistd.h>    // for usleep()

#include "hiddata.h"
#include "blink1-lib.h"

// taken from blink1/hardware/firmware/usbconfig.h
#define IDENT_VENDOR_NUM        0x27B8
#define IDENT_PRODUCT_NUM       0x01ED
#define IDENT_VENDOR_STRING     "ThingM"
#define IDENT_PRODUCT_STRING    "blink(1)"

/**
 * Open up a blink(1) for transactions.
 * returns 0 on success, and opened device in "dev"
 * or returns non-zero error that can be decoded with blink1_error_msg()
 * FIXME: what happens when multiple are plugged in?
 */
int blink1_open(usbDevice_t **dev)
{
    return usbhidOpenDevice(dev, 
                            IDENT_VENDOR_NUM,  IDENT_VENDOR_STRING,
                            IDENT_PRODUCT_NUM, IDENT_PRODUCT_STRING,
                            1);  // NOTE: '0' means "not using report IDs"
}

/**
 * Close a Blink1 
 */
void blink1_close(usbDevice_t *dev)
{
    usbhidCloseDevice(dev);
}

/**
 *
 */
int blink1_fadeToRGB(usbDevice_t *dev, int fadeMillis,
                        uint8_t r, uint8_t g, uint8_t b)
{
    char buffer[9];
    int err;

    if (dev == NULL) {
        return -1; // BLINK1_ERR_NOTOPEN;
    }

    int dms = fadeMillis/10;  // millis_divided_by_10

	/*
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;

	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	*/

    buffer[0] = 0;
    buffer[1] = 'c';
    buffer[2] = r;
    buffer[3] = g;
    buffer[4] = b;
    buffer[5] = (dms >> 8);
    buffer[6] = dms % 0xff;

    if( (err = usbhidSetReport(dev, buffer, sizeof(buffer))) != 0) {
        fprintf(stderr,"error writing data: %s\n",blink1_error_msg(err));
    }
    return err;  // FIXME: remove fprintf
}

/**
 *
 */
int blink1_setRGB(usbDevice_t *dev, uint8_t r, uint8_t g, uint8_t b )
{
    char buffer[9];
    int err;

    if (dev == NULL) {
        return -1; // BLINK1_ERR_NOTOPEN;
    }

	/*
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;

	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	*/

    buffer[0] = 0;
    buffer[1] = 'n';
    buffer[2] = r;
    buffer[3] = g;
    buffer[4] = b;
    
    if( (err = usbhidSetReport(dev, buffer, sizeof(buffer))) != 0) {
        fprintf(stderr,"error writing data: %s\n",blink1_error_msg(err));
    }
    return err;  // FIXME: remove fprintf
}


//
char *blink1_error_msg(int errCode)
{
    static char buffer[80];

    switch(errCode){
        case USBOPEN_ERR_ACCESS:    return "Access to device denied";
        case USBOPEN_ERR_NOTFOUND:  return "The specified device was not found";
        case USBOPEN_ERR_IO:        return "Communication error with device";
        default:
            sprintf(buffer, "Unknown USB error %d", errCode);
            return buffer;
    }
    return NULL;    /* not reached */
}

/*
static int hexread(char *buffer, char *string, int buflen)
{
char    *s;
int     pos = 0;

    while((s = strtok(string, ", ")) != NULL && pos < buflen){
        string = NULL;
        buffer[pos++] = (char)strtol(s, NULL, 0);
    }
    return pos;
}
*/
