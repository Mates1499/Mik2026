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

// ---------------- Cviceni 8b - dotykovy panel, aplikace "malovani" ----------------
// Postup kalibrace:
//  1) Spust program cv8_ad.c, dotkni se leveho dolniho rohu (bod 0,0)
//     a zapis hodnoty X a Y do X_MIN a Y_MIN (offset).
//  2) Dotkni se praveho horniho rohu (bod 127,63) a zapis hodnoty do X_MAX a Y_MAX.
// Prepocet:  x = (X_ad - X_MIN) * 127 / (X_MAX - X_MIN)
//            y = (Y_ad - Y_MIN) *  63 / (Y_MAX - Y_MIN)

#include <xc.h>
#include <stdint.h>
#include "glcd_PIC18F47K40.h"

// ---- KALIBRACNI KONSTANTY - prepis podle svych namerenych hodnot! ----
#define X_MIN 100           // AD hodnota X v bode 0,0
#define X_MAX 900           // AD hodnota X v bode 127,63
#define Y_MIN 150           // AD hodnota Y v bode 0,0
#define Y_MAX 850           // AD hodnota Y v bode 127,63
#define PRAH  50            // pod touto hodnotou se panel nedotyka

void AD_Init(void)
{
    ADREF  = 0b00000000;    // reference Vdd / Vss
    ADCLK  = 31;            // TAD = 2 us pri Fosc = 32 MHz
    ADPCH  = 0;             // kanal AN0
    ADCON0 = 0b10000100;    // zapnuto, jednorazovy prevod, Fosc, zarovnani vpravo
}

uint16_t AD_Convert(void)
{
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO == 1);
    return (((uint16_t)ADRESH) << 8) | ADRESL;
}

// Osa X: buzeni LATC = 0b01, cteni AN0
uint16_t Mer_X(void)
{
    LATC  = 0b00000001;
    ADPCH = 0;
    __delay_ms(2);          // ustaleni
    return AD_Convert();
}

// Osa Y: buzeni LATC = 0b10, cteni AN1
uint16_t Mer_Y(void)
{
    LATC  = 0b00000010;
    ADPCH = 1;
    __delay_ms(2);
    return AD_Convert();
}

// Prepocet AD hodnoty na souradnici displeje (0..max)
uint8_t Prepocet(uint16_t ad, uint16_t min, uint16_t mx, uint8_t max)
{
    int32_t v;
    v = ((int32_t)ad - min) * max / ((int32_t)mx - min);
    if (v < 0)   v = 0;     // omezeni na rozsah displeje
    if (v > max) v = max;
    return (uint8_t)v;
}

void main(void)
{
    uint16_t adx, ady;

    ANSELA = 0b00000011;    // RA0, RA1 analogove
    TRISA  = 0b00000011;
    ANSELC = 0b00000000;    // PORTC digitalni vystup (buzeni panelu)
    TRISC  = 0b00000000;
    ANSELB = 0b00000000;    // GLCD
    ANSELD = 0b00000000;
    TRISB  = 0b00000000;
    TRISD  = 0b00000000;
    GLCD_RST = 1;

    AD_Init();
    GLCD_Enable();
    GLCD_Clr();

    while (1)
    {
        adx = Mer_X();
        ady = Mer_Y();

        if (adx > PRAH && ady > PRAH)   // panel je stisknuty
        {
            GLCD_Dot(Prepocet(adx, X_MIN, X_MAX, 127),
                     Prepocet(ady, Y_MIN, Y_MAX, 63));   // nakresli bod
        }
    }
}
