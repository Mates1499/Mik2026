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

// ---------------- Cviceni 7b - teplotni senzor LM35 na RE1 ----------------
// LM35: 0 mV + 10 mV/stupen C  ->  T [C] = U [mV] / 10
// Takze U v mV je primo teplota v desetinach stupne (250 mV = 25,0 C)

#include <xc.h>
#include <stdint.h>
#include "lcd_PIC18F47K40.h"

uint16_t adresult;          // vysledek ADC
uint16_t teplota;           // teplota v desetinach C (= napeti v mV)
char txt[10];

void AD_Init(void)
{
    ADREF  = 0b00000000;    // reference Vdd / Vss
    ADCLK  = 7;             // TAD = 2 us
    ADPCH  = 0b100001;      // kanal ANE1 = RE1 (port E = 32..34, RE1 = 33)
    ADCON0 = 0b10000100;    // zapnuto, jednorazovy prevod, Fosc, zarovnani vpravo
}

uint16_t AD_Convert(void)
{
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO == 1);
    return (((uint16_t)ADRESH) << 8) | ADRESL;
}

void main(void)
{
    ANSELE = 0b00000010;    // RE1 analogovy vstup
    TRISE  = 0b00000010;    // RE1 vstup
    ANSELB = 0b00000000;    // PORTB (LCD) digitalni
    TRISB  = 0b00000000;
    LATB   = 0b00000000;

    LCD_Init();
    AD_Init();

    while (1)
    {
        __delay_ms(200);
        adresult = AD_Convert();
        teplota = (uint32_t)adresult * 5000 / 1024;  // mV = desetiny C

        // text "T=xx.x C"
        txt[0] = 'T';
        txt[1] = '=';
        txt[2] = teplota / 100 % 10 + '0';  // desitky
        txt[3] = teplota / 10 % 10 + '0';   // jednotky
        txt[4] = '.';
        txt[5] = teplota % 10 + '0';        // desetiny
        txt[6] = 0xDF;                      // znak stupne na alfanumerickem LCD
        txt[7] = 'C';
        txt[8] = 0;

        Send_Command(0b10000000);   // kurzor na zacatek 1. radku
        Send_String(txt);
    }
}
