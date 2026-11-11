// CONFIG1L
#pragma config FEXTOSC = HS     // External Oscillator mode Selection bits (HS (crystal oscillator) above 8 MHz; PFM set to high power)
#pragma config RSTOSC = EXTOSC_4PLL  // Power-up default value for COSC bits (EXTOSC operating per FEXTOSC bits (device manufacturing default))

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


#pragma config LVP = OFF        // Low Voltage Programming Enable bit (HV on MCLR/VPP must be used for programming)

// ---------------- Cviceni 8a - AD prevodnik, cteni AN0 a AN1, vypis na GLCD ----------------
// Fosc = 32 MHz, reference Vdd/Vss, 10 bitu
// Dotykovy panel: LATC = 0b01 -> mereni osy X na AN0 (RA0)
//                 LATC = 0b10 -> mereni osy Y na AN1 (RA1)
// Tento program slouzi i ke kalibraci (odecteni hodnot v rozich displeje).

#include <xc.h>
#include <stdint.h>
#include "glcd_PIC18F47K40.h"

char txt[7];                // text pro displej

void AD_Init(void)
{
    ADREF  = 0b00000000;    // reference: + = Vdd, - = Vss
    ADCLK  = 31;            // TAD = 2*(31+1)/Fosc = 64/32MHz = 2 us
    ADPCH  = 0;             // kanal AN0 = RA0
    ADCON0 = 0b10000100;    // ADON=1 zapnuto, ADCONT=0 jednorazovy prevod,
                            // ADCS=0 hodiny z Fosc (ADCLK), ADFM=1 zarovnani vpravo
}

uint16_t AD_Convert(void)
{
    ADCON0bits.GO = 1;              // start prevodu
    while (ADCON0bits.GO == 1);     // cekame na konec
    return (((uint16_t)ADRESH) << 8) | ADRESL;
}

void Int_to_String(unsigned int val, char* str)   // cislo -> 5 znaku textu
{
    int8_t i;
    for (i = 4; i >= 0; i--)
    {
        str[i] = val % 10 + '0';
        val /= 10;
    }
    str[5] = '\0';
}

// Zmeri osu X: buzeni LATC = 0b01, cteni z AN0
uint16_t Mer_X(void)
{
    LATC  = 0b00000001;     // buzeni panelu pro osu X
    ADPCH = 0;              // kanal AN0
    __delay_ms(2);          // ustaleni napeti na panelu
    return AD_Convert();
}

// Zmeri osu Y: buzeni LATC = 0b10, cteni z AN1
uint16_t Mer_Y(void)
{
    LATC  = 0b00000010;     // buzeni panelu pro osu Y
    ADPCH = 1;              // kanal AN1
    __delay_ms(2);
    return AD_Convert();
}

// Nakresli znacky (tvar L, ramena 6 bodu) do vsech 4 rohu displeje.
// Panel je oproti displeji trochu posunuty, proto se pri kalibraci
// dotkni presne spicky znacky a odecti hodnoty X a Y.
void Znacky_rohu(void)
{
    uint8_t i;
    for (i = 0; i < 6; i++)
    {
        GLCD_Dot(i, 63);        GLCD_Dot(0, 63 - i);      // levy horni roh   (0, 63)
        GLCD_Dot(i, 0);         GLCD_Dot(0, i);           // levy dolni roh   (0, 0)
        GLCD_Dot(127 - i, 63);  GLCD_Dot(127, 63 - i);    // pravy horni roh  (127, 63)
        GLCD_Dot(127 - i, 0);   GLCD_Dot(127, i);         // pravy dolni roh  (127, 0)
    }
}

void main(void)
{
    ANSELA = 0b00000011;    // RA0, RA1 analogove vstupy
    TRISA  = 0b00000011;    // RA0, RA1 vstupy
    ANSELC = 0b00000000;    // PORTC digitalni (buzeni panelu)
    TRISC  = 0b00000000;    // PORTC vystup
    ANSELB = 0b00000000;    // PORTB digitalni (rizeni GLCD)
    ANSELD = 0b00000000;    // PORTD digitalni (data GLCD)
    TRISB  = 0b00000000;
    TRISD  = 0b00000000;
    GLCD_RST = 1;           // displej neni v resetu

    AD_Init();
    GLCD_Enable();
    GLCD_Clr();
    Znacky_rohu();          // kalibracni znacky v rozich

    while (1)
    {
        // osa X - text uprostred leve poloviny (aby neprekryl znacky)
        GLCD_XY(20, 4);
        GLCD_Send_String("X=");
        Int_to_String(Mer_X(), txt);
        GLCD_Send_String(txt);

        // osa Y - o radek niz
        GLCD_XY(20, 3);
        GLCD_Send_String("Y=");
        Int_to_String(Mer_Y(), txt);
        GLCD_Send_String(txt);

        __delay_ms(50);
    }
}
