// CONFIG1L
#pragma config FEXTOSC = HS     // External Oscillator mode Selection bits (HS (crystal oscillator) above 8 MHz; PFM set to high power)
#pragma config RSTOSC = EXTOSC  // Power-up default value for COSC bits (EXTOSC operating per FEXTOSC bits (device manufacturing default))

// CONFIG1H
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = OFF       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)

// CONFIG2L
#pragma config MCLRE = EXTMCLR  // Master Clear Enable bit (If LVP = 0, MCLR pin is MCLR; If LVP = 1, RE3 pin function is MCLR )
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (Power up timer disabled)
#pragma config LPBOREN = OFF    // Low-power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled , SBOREN bit is ignored)

// CONFIG2H
#pragma config BORV = VBOR_2P45 // Brown Out Reset Voltage selection bits (Brown-out Reset Voltage (VBOR) set to 2.45V)
#pragma config ZCD = OFF        // ZCD Disable bit (ZCD disabled. ZCD can be enabled by setting the ZCDSEN bit of ZCDCON)
#pragma config PPS1WAY = ON     // PPSLOCK bit One-Way Set Enable bit (PPSLOCK bit can be cleared and set only once; PPS registers remain locked after one clear/set cycle)
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config DEBUG = OFF      // Debugger Enable bit (Background debugger disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Extended Instruction Set and Indexed Addressing Mode disabled)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled)

// CONFIG3H
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

#pragma config LVP = OFF        // Low Voltage Programming Enable bit

// ---------------- Cviceni 7a - AD prevodnik, voltmetr na RA5 (AN5) ----------------
// Reference = Vdd (5 V) a Vss (0 V), 10 bitu
// Citlivost = 5 V / 1024 = 4,883 mV na 1 krok
// U [mV] = ADC * 5000 / 1024

#include <xc.h>
#include <stdint.h>
#include "lcd_PIC18F47K40.h"

uint16_t adresult;          // vysledek ADC (0..1023)
uint16_t napeti;            // napeti v mV
char txt[10];               // text pro LCD

void AD_Init(void)
{
    ADREF  = 0b00000000;    // reference: + = Vdd, - = Vss
    ADCLK  = 7;             // TAD = 2*(7+1)/Fosc = 16/8MHz = 2 us
    ADPCH  = 0b000101;      // kanal AN5 = RA5 (port A = 0..7)
    ADCON0 = 0b10000100;    // ADON=1 zapnuto, ADCONT=0 jednorazovy prevod,
                            // ADCS=0 hodiny z Fosc (ADCLK), ADFM=1 zarovnani vpravo
}

uint16_t AD_Convert(void)
{
    ADCON0bits.GO = 1;              // start prevodu
    while (ADCON0bits.GO == 1);     // cekame na konec
    return (((uint16_t)ADRESH) << 8) | ADRESL;   // 10bit vysledek
}

void main(void)
{
    ANSELA = 0b00100000;    // RA5 analogovy vstup
    TRISA  = 0b11111111;    // PORTA vstup
    ANSELB = 0b00000000;    // PORTB (LCD) digitalni
    TRISB  = 0b00000000;    // PORTB vystup
    LATB   = 0b00000000;

    LCD_Init();
    AD_Init();

    while (1)
    {
        __delay_ms(100);
        adresult = AD_Convert();
        napeti = (uint32_t)adresult * 5000 / 1024;   // prepocet na mV

        // text "U=x.xxx V"
        txt[0] = 'U';
        txt[1] = '=';
        txt[2] = napeti / 1000 + '0';       // volty
        txt[3] = '.';
        txt[4] = napeti / 100 % 10 + '0';   // desetiny
        txt[5] = napeti / 10 % 10 + '0';    // setiny
        txt[6] = napeti % 10 + '0';         // tisiciny
        txt[7] = ' ';
        txt[8] = 'V';
        txt[9] = 0;                         // konec retezce

        Send_Command(0b10000000);   // kurzor na zacatek 1. radku (bez mazani = neblika)
        Send_String(txt);
    }
}
