#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  

void dbg_init(void);
//void dbg(char data[3], char len);
void dbg(char data);

int main(void)
{
    wdt_disable();
    /*
    USB_CFG_IOPORT   &= (uchar)~((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));
    USBDDR    |= ((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));
    _delay_loop_2(40000);   // 10ms
    USBDDR &= ~((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));
    
    
    //UDR = 'b';
    //while (!(UCSRA & (1 << UDRE)));
    //UDR = 0;
    //dbg("on", 2);
    //usbInit();
    */
    //odDebugInit();
    dbg_init();

    
    sei();

    dbg(eeprom_read_word(&eepromWord));
    
    for(;;){                /* main event loop */
        eeprom_write_word(&eepromWord, eeprom_read_word(&eepromWord)+1);
       
    }
    return 0;
}

void dbg_init(void){
    UBRRL = 77;
    UCSRB = (1 << RXEN) | (1 << TXEN);
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
}
/*
void dbg(char data[3], char len){
    char i;
    for(i=0; i<len; i++){
        while (!(UCSRA & (1 << UDRE)));
        UDR = data[i];
    }
}*/
void dbg(char data){
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}

