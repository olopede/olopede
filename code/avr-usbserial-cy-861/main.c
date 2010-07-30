/* Name: main.c
 * Project: hid-custom-rq example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-07
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: main.c 692 2008-11-07 15:07:40Z cs $
 */

/*
This example should run on most AVRs with only little changes. No special
hardware resources except INT0 are used. You may have to change usbconfig.h for
different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
at least be connected to INT0 as well.
We assume that an LED is connected to port B bit 0. If you connect it to a
different port or bit, change the macros below:
*/
#define LED_PORT_DDR        DDRB
#define LED_PORT_OUTPUT     PORTB
#define LED_BIT             0

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
void dbg_init(void);
//void dbg(char data);
#define dbg uart_tx
void uart_tx(char data);
#define BUFFER_SIZE 8
uchar buffer[BUFFER_SIZE];
uchar cbuff = 1;

uint32_t baud;
uchar parity;
uchar settings[5];
/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */
PROGMEM char usbDescriptorConfiguration[DESCRIPTOR_SIZE] = {
    9,          /* sizeof(usbDescriptorConfiguration): length of descriptor in bytes */
    USBDESCR_CONFIG,/* descriptor type */
    18 + 7 + 9 + 7, 0,  /* total length of data returned (including inlined descriptors) */
    1,          /* number of interfaces in this configuration */
    1,          /* index of this configuration */
    4,          /* configuration name string index */
    USBATTR_BUSPOWER,   /* attributes */
    40,    /* max USB current in 2mA units */
/* interface descriptor follows inline: */

    9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    0,          /* index of this interface */
    0,          /* alternate setting for this interface */
    2,   /* endpoints excl 0: number of endpoint descriptors to follow */
    3,          /* interface class: HID */
    0,          /* subclass */
    0,          /* protocol */
    0,          /* string index for interface */

    9,          /* sizeof(usbDescrHID): length of descriptor in bytes */
    USBDESCR_HID,   /* descriptor type: HID */
    0x01, 0x01, /* BCD representation of HID version */
    0x00,       /* target country code */
    0x01,       /* number of HID Report (or other HID class) Descriptor infos to follow */
    0x22,       /* descriptor type: report */
    USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH, 0,  /* total length of report descriptor */

    7,          /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x81,       /* IN endpoint number 1 */
    0x03,       /* attrib: Interrupt endpoint */
    8, 0,       /* maximum packet size */
    USB_CFG_INTR_POLL_INTERVAL, /* in ms */
    
    7,          /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x02,       /* OUT endpoint number 1 */
    0x03,       /* attrib: Interrupt endpoint */
    8, 0,       /* maximum packet size */
    USB_CFG_INTR_POLL_INTERVAL, /* in ms */
};
PROGMEM char usbHidReportDescriptor[37] = {    /* USB report descriptor */
    0x06, 0xa0, 0xff,              // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    
    0x09, 0x01,                    //   USAGE (data)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x08,                    //   REPORT_COUNT 8
    0x81, 0x02,                    //   INPUT (Data,Var,Abs,Bit)

    0x09, 0x02,                    //   USAGE (Undefined)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x08,                    //   REPORT_COUNT 8
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs,Bit)

    0x09, 0x03,                    //   USAGE (Undefined)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0xb2, 0x02,                    //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};


/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    
    usbRequest_t    *rq = (void *)data;
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){
        if(rq->bRequest != 0x09 && rq->bRequest != 0x01)
            //dbg(rq->bRequest);
        //dbg(rq->bRequest);
        //dbg(rq->wLength.bytes[0]);
        //dbg(rq->wLength.bytes[1]);
        //dbg(rq->bRequest);
        //dbg(data[0]);
        //dbg(data[5]);
        /*
        if(rq->bRequest == 0x01){
            usbMsgPtr[0] = 4;//rq->wLength.word;
            return 1;
        }else{
            return 0;
        }*/
        return USB_NO_MSG;
    } else {
        //dbg('e');
        //dbg(data[5]);dbg(data[7]);
    }
/*
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_VENDOR){
        // DBG1(0x50, &rq->bRequest, 1);   // debug output: print our request 
        
        if(rq->bRequest == CUSTOM_RQ_SET_STATUS){
            if(rq->wValue.bytes[0] & 1){    // set LED 
                LED_PORT_OUTPUT |= _BV(LED_BIT);
            }else{                          // clear LED 
                LED_PORT_OUTPUT &= ~_BV(LED_BIT);
            }
        }else if(rq->bRequest == CUSTOM_RQ_GET_STATUS){
            static uchar dataBuffer[1];     // buffer must stay valid when usbFunctionSetup returns 
            dataBuffer[0] = ((LED_PORT_OUTPUT & _BV(LED_BIT)) != 0);
            usbMsgPtr = dataBuffer;         // tell the driver which data to return 
            return 1;                       // tell the driver to send 1 byte 
        }
    }else{

    }
    */
    
    return 0;   // default for not implemented requests: return no data back to host 
}
uchar usbFunctionRead(uchar *data, uchar len)
{
    uchar i;
    /*
    uchar i;
    if(len > bytesRemaining)                // len is max chunk size
        len = bytesRemaining;               // senbuffer[0] = (cbuff - 1) | 0x30;
            usbSetInterrupt(buffer, cbuff);d an incomplete chunk
    bytesRemaining -= len;
    for(i = 0; i < len; i++)
        data[i] = getData(currentPosition); // copy the data to the buffer
    return len;                             // return real chunk size
    */
    //dbg(len);
    if(len == 5){ //BAD -- should check packet type instead TODO
        *((uint32_t *)data) = baud;
        data[4] = parity;
        return len; //5
    }
    for(i=0; i < 5; i++){
            data[i] = settings[i];
        }
    //cbuff = 1;
    return 0;
}

uchar usbFunctionWrite(uchar *data, uchar len)
{
    uchar i;
    /*
    uchar i;
    if(len > bytesRemaining)                // if this is the last incomplete chunk
        len = bytesRemaining;               // limit to the amount we can store
    bytesRemaining -= len;
    for(i = 0; i < len; i++)
        buffer[currentPosition++] = data[i];
    return ;             // return 1 if we have all data
    */
    //dbg('w');
    //dbg(len);
    if(len == 5){//BAD -- should check packet type instead TODO
        // baud not quite working TODO
        //baud = *((uint32_t *)data);
        //dbg((12000000 / (16 * baud)) - 1);
        //dbg(data[2]);
        //dbg(data[3]);
        //parity = data[4];
        cbuff = 6;
        for(i=0; i < 5; i++){
            settings[i] = data[i];
            //buffer[i+1] = data[i];
        }
        return 1;
    }
    //dbg(len);
    //dbg(data[0]);
    //dbg(data[1]);
    //dbg(data[2]);
    //dbg(data[3]);
    //dbg(data[4]);
    //dbg('W');
    return 1;
}
void usbFunctionWriteOut(uchar *data, uchar len)
{
    uchar i;
    if(usbRxToken == 2){
        //dbg('o');
        uchar len = data[0] & 0x07;
        uchar status = (data[0] & 0xf8) >> 3;
        //dbg(status);
        //dbg(len);
        if(status == 0x06 && len){
            for(i=1; i<=len; i++){
                uart_tx(data[i]);
                /*
                if(cbuff < 8){
                    buffer[cbuff] = data[i] - 3;
                    cbuff++;
                }*/
            }
            //buffer[0] = (cbuff - 1) | 0x30;
            //usbSetInterrupt(buffer, cbuff);
        }
        //dbg(cbuff);
        //dbg(data[6]);
        //dbg(data[7]);
        //dbg(data[]);
    }else{
        //dbg('W');
        //dbg(usbRxToken);
    }
}

/* ------------------------------------------------------------------------- */

int main(void)
{
uchar   i;
    dbg_init();
    //dbg('-');
    wdt_enable(WDTO_1S);
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    //DBG1(0x00, 0, 0);       /* debug output: main starts */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    //odDebugInit();
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    // //LED_PORT_DDR |= _BV(LED_BIT);   /* make the LED bit an output */
    sei();
    
    //dbg('!');
    //DBG1(0x01, 0, 0);       /* debug output: main loop starts */
    for(;;){                /* main event loop */
#if 0   /* this is a bit too aggressive for a debug output */
        DBG2(0x02, 0, 0);   /* debug output: main loop iterates */
#endif
        wdt_reset();
        usbPoll();
        //while(!usbInterruptIsReady()){
        //    wdt_reset();
        //    usbPoll();
        //}
        if( UCSRA&(1<<RXC) && cbuff < BUFFER_SIZE ) {
            buffer[cbuff] = UDR;
            cbuff++;
        }
        if(cbuff > 1 && usbInterruptIsReady()) {                        // only send if we have data
            buffer[0] = (cbuff - 1) | 0x30;
            usbSetInterrupt(buffer, cbuff);
            //dbg(parity);
            cbuff = 1;
        }
        //cbuff = 1;
        
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
void dbg_init(void){
    UBRRL = 12;
    UCSRB = (1 << RXEN) | (1 << TXEN);
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
}
/*
void dbg32(uint32_t data){
    dbg((data & 0xff000000) >> 24);
    dbg((data & 0x00ff0000) >> 16);
    dbg((data & 0x0000ff00) >> 8);
    dbg((data & 0x000000ff));
}
void dbg(char data){
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}*/


void uart_tx(char data){
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}
