/* 
 * AVR "usber" programmer & UART converter
 *
 * Zach Banks <zach@olopede.com>
 * Creation Date: Aug 11, 2010
 * Tabsize: 4
 * Copyright: (c) 2010 by olopede llc
 * License: GNU GPL v2
 * Target: ATmega168 @ 16Mhz
 *
 */
 
/*
 * This code is a combination of 2 existing projects
 * AVR-CDC : http://www.recursion.jp/avrcdc/
 * USBasp  : http://www.fischl.de/usbasp/
 * And based on the v-usb library for AVRs
 * http://www.obdev.at/products/vusb
 *
 */

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "usbdrv.h"
#include "uart.h"
#include "usbasp.h"
#include "isp.h"
#include "clock.h"


#define PRG_UART 0
#define PRG_ASP  1
char prgMode = PRG_ASP;


enum {
    SEND_ENCAPSULATED_COMMAND = 0,
    GET_ENCAPSULATED_RESPONSE,
    SET_COMM_FEATURE,
    GET_COMM_FEATURE,
    CLEAR_COMM_FEATURE,
    SET_LINE_CODING = 0x20,
    GET_LINE_CODING,
    SET_CONTROL_LINE_STATE,
    SEND_BREAK
};

const PROGMEM char configDescrAsp[] = {    /* USB configuration descriptor */
    9,          /* sizeof(usbDescriptorConfiguration): length of descriptor in bytes */
    USBDESCR_CONFIG,    /* descriptor type */
    18, 0,      /* total length of data returned (including inlined descriptors) */
    1,          /* number of interfaces in this configuration */
    1,          /* index of this configuration */
    0,          /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    USBATTR_SELFPOWER,      /* attributes */
#else
    (char)USBATTR_BUSPOWER, /* attributes */
#endif
    USB_CFG_MAX_BUS_POWER/2,            /* max USB current in 2mA units */
/* interface descriptor follows inline: */
    9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    0,          /* index of this interface */
    0,          /* alternate setting for this interface */
    0, /* endpoints excl 0: number of endpoint descriptors to follow */
    0,
    0,
    0,
    0,          /* string index for interface */

};

const PROGMEM char deviceDescrAsp[] = {    /* USB device descriptor */
    18,         /* sizeof(usbDescriptorDevice): length of descriptor in bytes */
    USBDESCR_DEVICE,        /* descriptor type */
    0x10, 0x01,             /* USB version supported */
    0xff,
    0,
    0,                      /* protocol */
    8,                      /* max packet size */
    /* the following two casts affect the first byte of the constant only, but
     * that's sufficient to avoid a warning with the default values.
     */
    0xc0, 0x16,/* 2 bytes */
    0xdc, 0x05,/* 2 bytes */
    0x03, 0x01, /* 2 bytes */
    1,         /* manufacturer string index */
    2,        /* product string index */
    USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER != 0 ? 3 : 0,  /* serial number string index */
    1,          /* number of configurations */
};

const PROGMEM char deviceDescrCDC[] = {    /* USB device descriptor */
    18,         /* sizeof(usbDescriptorDevice): length of descriptor in bytes */
    USBDESCR_DEVICE,        /* descriptor type */
    0x10, 0x01,             /* USB version supported */
    2,
    0,
    0,                      /* protocol */
    8,                      /* max packet size */
    /* the following two casts affect the first byte of the constant only, but
     * that's sufficient to avoid a warning with the default values.
     */
    0xc0, 0x16,/* 2 bytes */
    0xdc, 0xff,/* 2 bytes */
    0x01, 0x00, /* 2 bytes */
    USB_CFG_DESCR_PROPS_STRING_VENDOR != 0 ? 1 : 0,         /* manufacturer string index */
    USB_CFG_DESCR_PROPS_STRING_PRODUCT != 0 ? 2 : 0,        /* product string index */
    USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER != 0 ? 3 : 0,  /* serial number string index */
    1,          /* number of configurations */
};

const PROGMEM char configDescrCDC[] = {   /* USB configuration descriptor */
    9,          /* sizeof(usbDescrConfig): length of descriptor in bytes */
    USBDESCR_CONFIG,    /* descriptor type */
    67,
    0,          /* total length of data returned (including inlined descriptors) */
    2,          /* number of interfaces in this configuration */
    1,          /* index of this configuration */
    0,          /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    (1 << 7) | USBATTR_SELFPOWER,       /* attributes */
#else
    (1 << 7),                           /* attributes */
#endif
    USB_CFG_MAX_BUS_POWER/2,            /* max USB current in 2mA units */

    /* interface descriptor follows inline: */
    9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    0,          /* index of this interface */
    0,          /* alternate setting for this interface */
    1,   /* endpoints excl 0: number of endpoint descriptors to follow */
    2,
    2,
    1,
    0,          /* string index for interface */

    /* CDC Class-Specific descriptor */
    5,           /* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    0,           /* header functional descriptor */
    0x10, 0x01,

    4,           /* sizeof(usbDescrCDC_AcmFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    2,           /* abstract control management functional descriptor */
    0x02,        /* SET_LINE_CODING,    GET_LINE_CODING, SET_CONTROL_LINE_STATE    */

    5,           /* sizeof(usbDescrCDC_UnionFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    6,           /* union functional descriptor */
    0,           /* CDC_COMM_INTF_ID */
    1,           /* CDC_DATA_INTF_ID */

    5,           /* sizeof(usbDescrCDC_CallMgtFn): length of descriptor in bytes */
    0x24,        /* descriptor type */
    1,           /* call management functional descriptor */
    3,           /* allow management on data interface, handles call management by itself */
    1,           /* CDC_DATA_INTF_ID */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x80|3,        /* IN endpoint number */
    0x03,        /* attrib: Interrupt endpoint */
    8, 0,        /* maximum packet size */
    200,        /* in ms */

    /* Interface Descriptor  */
    9,           /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE,           /* descriptor type */
    1,           /* index of this interface */
    0,           /* alternate setting for this interface */
    2,           /* endpoints excl 0: number of endpoint descriptors to follow */
    0x0A,        /* Data Interface Class Codes */
    0,
    0,           /* Data Interface Class Protocol Codes */
    0,           /* string index for interface */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x01,        /* OUT endpoint number 1 */
    0x02,        /* attrib: Bulk endpoint */
    8, 0,        /* maximum packet size */
    0,           /* in ms */

    /* Endpoint Descriptor */
    7,           /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x81,        /* IN endpoint number 1 */
    0x02,        /* attrib: Bulk endpoint */
    8, 0,        /* maximum packet size */
    0,           /* in ms */
};


uchar usbFunctionDescriptor(usbRequest_t *rq)
{
    
    if(rq->wValue.bytes[1] == USBDESCR_DEVICE){
        if(prgMode == PRG_ASP){
            usbMsgPtr = (uchar *)deviceDescrAsp;
            return deviceDescrAsp[0];
        }else{
            usbMsgPtr = (uchar *)deviceDescrCDC;
            return deviceDescrCDC[0];
        }
    }else{  /* must be config descriptor */
        if(prgMode == PRG_ASP){
            usbMsgPtr = (uchar *)configDescrAsp;
            return sizeof(configDescrAsp);
        }else{
            usbMsgPtr = (uchar *)configDescrCDC;
            return sizeof(configDescrCDC);
        }
    }
}


uchar               sendEmptyFrame;
static uchar        intr3Status;    /* used to control interrupt endpoint transmissions */

static uchar        stopbit, parity, databit;
static usbDWord_t   baud;

static void resetUart(void)
{

    uartInit(baud.dword, parity, stopbit, databit);
    irptr    = 0;
    iwptr    = 0;
    urptr    = 0;
    uwptr    = 0;
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface  CDC ------------------------ */
/* ------------------------------------------------------------------------- */

uchar usbFunctionSetup_cdc(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */

        if( rq->bRequest==GET_LINE_CODING || rq->bRequest==SET_LINE_CODING ){
            return 0xff;
        /*    GET_LINE_CODING -> usbFunctionRead()    */
        /*    SET_LINE_CODING -> usbFunctionWrite()    */
        }
        if(rq->bRequest == SET_CONTROL_LINE_STATE){
            UART_CTRL_PORT	= (UART_CTRL_PORT&~(1<<UART_CTRL_DTR))|((rq->wValue.word&1)<<UART_CTRL_DTR);

#if USB_CFG_HAVE_INTRIN_ENDPOINT3
            /* Report serial state (carrier detect). On several Unix platforms,
             * tty devices can only be opened when carrier detect is set.
             */
            if( intr3Status==0 )
                intr3Status = 2;
#endif
        }
#if 1
        /*  Prepare bulk-in endpoint to respond to early termination   */
        if((rq->bmRequestType & USBRQ_DIR_MASK) == USBRQ_DIR_HOST_TO_DEVICE)
            sendEmptyFrame  = 1;
#endif
    }

    return 0;
}


/*---------------------------------------------------------------------------*/
/* usbFunctionRead                                                          */
/*---------------------------------------------------------------------------*/

uchar usbFunctionRead_cdc( uchar *data, uchar len )
{

    data[0] = baud.bytes[0];
    data[1] = baud.bytes[1];
    data[2] = baud.bytes[2];
    data[3] = baud.bytes[3];
    data[4] = stopbit;
    data[5] = parity;
    data[6] = databit;

    return 7;
}


/*---------------------------------------------------------------------------*/
/* usbFunctionWrite                                                          */
/*---------------------------------------------------------------------------*/

uchar usbFunctionWrite_cdc( uchar *data, uchar len )
{

    /*    SET_LINE_CODING    */
    baud.bytes[0] = data[0];
    baud.bytes[1] = data[1];
    baud.bytes[2] = data[2];
    baud.bytes[3] = data[3];

    stopbit    = data[4];
    parity     = data[5];
    databit    = data[6];

    if( parity>2 )
        parity    = 0;
    if( stopbit==1 )
        stopbit    = 0;

    resetUart();

    return 1;
}


void usbFunctionWriteOut_cdc( uchar *data, uchar len )
{

    /*  usb -> rs232c:  transmit char    */
    for( ; len; len-- ) {
        uchar   uwnxt;

        uwnxt = (uwptr+1) & TX_MASK;
        if( uwnxt!=irptr ) {
            tx_buf[uwptr] = *data++;
            uwptr = uwnxt;
        }
    }

    /*  postpone receiving next data    */
    if( uartTxBytesFree()<=HW_CDC_BULK_OUT_SIZE )
        usbDisableAllRequests();
}


static void hardwareInit_cdc(void)
{
    wdt_enable(WDTO_1S);
    /* activate pull-ups except on USB lines */
    USB_CFG_IOPORT   = (uchar)~((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));
    /* all pins input except USB (-> USB reset) */
#ifdef USB_CFG_PULLUP_IOPORT    /* use usbDeviceConnect()/usbDeviceDisconnect() if available */
    USBDDR    = 0;    /* we do RESET by deactivating pullup */
    usbDeviceDisconnect();
#else
    USBDDR    = (1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT);
#endif

    /* 250 ms disconnect */
    wdt_reset();
    _delay_ms(250);

#ifdef USB_CFG_PULLUP_IOPORT
    usbDeviceConnect();
#else
    USBDDR    = 0;      /*  remove USB reset condition */
#endif

    /*    USART configuration    */
    baud.dword  = UART_DEFAULT_BPS;
    stopbit = 0;
    parity  = 0;
    databit = 8;
    resetUart();
}

static void mainLoop_cdc(){
    wdt_reset();
    usbPoll();
    uartPoll();

#if USB_CFG_HAVE_INTRIN_ENDPOINT3
    /* We need to report rx and tx carrier after open attempt */
    if(intr3Status != 0 && usbInterruptIsReady3()){
        static uchar serialStateNotification[10] = {0xa1, 0x20, 0, 0, 0, 0, 2, 0, 3, 0};

        if(intr3Status == 2){
            usbSetInterrupt3(serialStateNotification, 8);
        }else{
            usbSetInterrupt3(serialStateNotification+8, 2);
        }
        intr3Status--;
    }
#endif
}



/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ASP ------------------------- */
/* ------------------------------------------------------------------------- */

static uchar replyBuffer[8];

static uchar prog_state = PROG_STATE_IDLE;
static uchar prog_sck = USBASP_ISP_SCK_AUTO;

static uchar prog_address_newmode = 0;
static unsigned long prog_address;
static unsigned int prog_nbytes = 0;
static unsigned int prog_pagesize;
static uchar prog_blockflags;
static uchar prog_pagecounter;


uchar usbFunctionSetup_asp(uchar data[8]) {

	uchar len = 0;

	if (data[1] == USBASP_FUNC_CONNECT) {

		/* set SCK speed */
		if ((PINC & (1 << PC2)) == 0) {
			ispSetSCKOption(USBASP_ISP_SCK_8);
		} else {
			ispSetSCKOption(prog_sck);
		}

		/* set compatibility mode of address delivering */
		prog_address_newmode = 0;

		ledRedOn();
		ispConnect();

	} else if (data[1] == USBASP_FUNC_DISCONNECT) {
		ispDisconnect();
		ledRedOff();

	} else if (data[1] == USBASP_FUNC_TRANSMIT) {
		replyBuffer[0] = ispTransmit(data[2]);
		replyBuffer[1] = ispTransmit(data[3]);
		replyBuffer[2] = ispTransmit(data[4]);
		replyBuffer[3] = ispTransmit(data[5]);
		len = 4;

	} else if (data[1] == USBASP_FUNC_READFLASH) {

		if (!prog_address_newmode)
			prog_address = (data[3] << 8) | data[2];

		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_READFLASH;
		len = 0xff; /* multiple in */

	} else if (data[1] == USBASP_FUNC_READEEPROM) {

		if (!prog_address_newmode)
			prog_address = (data[3] << 8) | data[2];

		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_READEEPROM;
		len = 0xff; /* multiple in */

	} else if (data[1] == USBASP_FUNC_ENABLEPROG) {
		replyBuffer[0] = ispEnterProgrammingMode();
		len = 1;

	} else if (data[1] == USBASP_FUNC_WRITEFLASH) {

		if (!prog_address_newmode)
			prog_address = (data[3] << 8) | data[2];

		prog_pagesize = data[4];
		prog_blockflags = data[5] & 0x0F;
		prog_pagesize += (((unsigned int) data[5] & 0xF0) << 4);
		if (prog_blockflags & PROG_BLOCKFLAG_FIRST) {
			prog_pagecounter = prog_pagesize;
		}
		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_WRITEFLASH;
		len = 0xff; /* multiple out */

	} else if (data[1] == USBASP_FUNC_WRITEEEPROM) {

		if (!prog_address_newmode)
			prog_address = (data[3] << 8) | data[2];

		prog_pagesize = 0;
		prog_blockflags = 0;
		prog_nbytes = (data[7] << 8) | data[6];
		prog_state = PROG_STATE_WRITEEEPROM;
		len = 0xff; /* multiple out */

	} else if (data[1] == USBASP_FUNC_SETLONGADDRESS) {

		/* set new mode of address delivering (ignore address delivered in commands) */
		prog_address_newmode = 1;
		/* set new address */
		prog_address = *((unsigned long*) &data[2]);

	} else if (data[1] == USBASP_FUNC_SETISPSCK) {

		/* set sck option */
		prog_sck = data[2];
		replyBuffer[0] = 0;
		len = 1;
	}

	usbMsgPtr = replyBuffer;

	return len;
}

uchar usbFunctionRead_asp(uchar *data, uchar len) {

	uchar i;

	/* check if programmer is in correct read state */
	if ((prog_state != PROG_STATE_READFLASH) && (prog_state
			!= PROG_STATE_READEEPROM)) {
		return 0xff;
	}

	/* fill packet */
	for (i = 0; i < len; i++) {
		if (prog_state == PROG_STATE_READFLASH) {
			data[i] = ispReadFlash(prog_address);
		} else {
			data[i] = ispReadEEPROM(prog_address);
		}
		prog_address++;
	}

	/* last packet? */
	if (len < 8) {
		prog_state = PROG_STATE_IDLE;
	}

	return len;
}

uchar usbFunctionWrite_asp(uchar *data, uchar len) {

	uchar retVal = 0;
	uchar i;

	/* check if programmer is in correct write state */
	if ((prog_state != PROG_STATE_WRITEFLASH) && (prog_state
			!= PROG_STATE_WRITEEEPROM)) {
		return 0xff;
	}

	for (i = 0; i < len; i++) {

		if (prog_state == PROG_STATE_WRITEFLASH) {
			/* Flash */

			if (prog_pagesize == 0) {
				/* not paged */
				ispWriteFlash(prog_address, data[i], 1);
			} else {
				/* paged */
				ispWriteFlash(prog_address, data[i], 0);
				prog_pagecounter--;
				if (prog_pagecounter == 0) {
					ispFlushPage(prog_address, data[i]);
					prog_pagecounter = prog_pagesize;
				}
			}

		} else {
			/* EEPROM */
			ispWriteEEPROM(prog_address, data[i]);
		}

		prog_nbytes--;

		if (prog_nbytes == 0) {
			prog_state = PROG_STATE_IDLE;
			if ((prog_blockflags & PROG_BLOCKFLAG_LAST) && (prog_pagecounter
					!= prog_pagesize)) {

				/* last block and page flush pending, so flush it now */
				ispFlushPage(prog_address, data[i]);
			}

			retVal = 1; // Need to return 1 when no more data is to be received
		}

		prog_address++;
	}

	return retVal;
}




static void hardwareInit_asp(){
    uchar i, j;

	/* no pullups on USB and ISP pins */
	//PORTD = 0;
	PORTB = 0;
	/* all outputs except PD2 = INT0 */
	//DDRD = ~(1 << 2);

	/* output SE0 for USB reset */
	DDRB = ~0;
	j = 0;
	/* USB Reset by device only required on Watchdog Reset */
	while (--j) {
		i = 0;
		/* delay >10ms for USB reset */
		while (--i)
			;
	}
	/* all USB and ISP pins inputs */
	DDRB = 0;

	/* all inputs except PC0, PC1 */
	DDRC = 0x03;
	PORTC = 0xfe;

	/* init timer */
	clockInit();
}


uchar usbFunctionSetup(uchar data[8]){
    if(prgMode == PRG_ASP){
        return usbFunctionSetup_asp(data);
    }else{ //CDC
        return usbFunctionSetup_cdc(data);
    }
}
uchar usbFunctionRead(uchar *data, uchar len){
    if(prgMode == PRG_ASP){
        return usbFunctionRead_asp(data, len);
    }else{ //CDC
        return usbFunctionRead_cdc(data, len);
    }
}
uchar usbFunctionWrite(uchar *data, uchar len){
    if(prgMode == PRG_ASP){
        return usbFunctionWrite_asp(data, len);
    }else{ //CDC
        return usbFunctionWrite_cdc(data, len);
    }
}
void usbFunctionWriteOut( uchar *data, uchar len ){
    if(prgMode == PRG_ASP){
        //return usbFunctionWriteOut_asp(data, len);
    }else{ //CDC
        return usbFunctionWriteOut_cdc(data, len);
    }
}


int main(void)
{
    PORTB = 0x01;
    DDRB = 0x01;
    prgMode = PINB & 0x01;
    /*
    if(prgMode == PRG_ASP){
        //usbFunctionSetup = usbFunctionSetup_asp;
        //usbFunctionRead = usbFunctionRead_asp;
        //usbFunctionWrite = usbFunctionWrite_asp;
        //usbFunctionWriteOut = usbFunctionWriteOut_asp;
        *hardwareInit = &hardwareInit_asp;
        *mainLoop = &usbPoll;//mainLoop_asp;
    }else{
        //usbFunctionSetup = usbFunctionSetup_cdc;
        //usbFunctionRead = usbFunctionRead_cdc;
        //usbFunctionWrite = usbFunctionWrite_cdc;
        //usbFunctionWriteOut = usbFunctionWriteOut_cdc;
        hardwareInit = &hardwareInit_cdc;
        mainLoop = &mainLoop_cdc;
    }*/
    
    //odDebugInit();
    if(prgMode == PRG_ASP){
        hardwareInit_asp();
        usbInit();
        sei();
        for(;;){
            usbPoll();
        }
    }else{
        hardwareInit_cdc();
        usbInit();

        intr3Status = 0;
        sendEmptyFrame  = 0;

        sei();
        for(;;){    /* main event loop */
            mainLoop_cdc();
        }
    }
    

    return 0;
}

