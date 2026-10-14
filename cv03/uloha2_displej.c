/*
 * ÚLOHA 2 – Obsluha 7segmentového displeje (časový multiplex)
 *
 * Zapojení:
 *   PORTD = segmenty: RD0=A, RD1=B, RD2=C, RD3=D, RD4=E, RD5=F, RD6=G, RD7=DP
 *   RA0–RA3 = výběr pozice: RA0=DIS0 (vpravo) ... RA3=DIS3 (vlevo)
 *   SW4: zapnout DIS0–DIS3 (přepínače 1–4)
 *   SW3: vypnout LED na PORTA, PORTC, PORTD (jinak blikají se segmenty)
 *
 * Program zobrazuje čítač 0000–9999, který se zvyšuje asi 2x za sekundu.
 *
 * Princip multiplexu: vždy svítí jen JEDNA pozice. Pozice se střídají
 * tak rychle (každé 2 ms, celý displej 8 ms = 125 Hz), že oko vidí
 * všechny čtyři najednou.
 */

// CONFIG1L
#pragma config FEXTOSC = HS
#pragma config RSTOSC = EXTOSC
// CONFIG1H
#pragma config CLKOUTEN = OFF
#pragma config CSWEN = ON
#pragma config FCMEN = ON
// CONFIG2L
#pragma config MCLRE = EXTMCLR
#pragma config PWRTE = OFF
#pragma config LPBOREN = OFF
#pragma config BOREN = SBORDIS
// CONFIG2H
#pragma config BORV = VBOR_2P45
#pragma config ZCD = OFF
#pragma config PPS1WAY = ON
#pragma config STVREN = ON
#pragma config DEBUG = OFF
#pragma config XINST = OFF
// CONFIG3L
#pragma config WDTCPS = WDTCPS_31
#pragma config WDTE = OFF
// CONFIG3H
#pragma config WDTCWS = WDTCWS_7
#pragma config WDTCCS = SC
#pragma config LVP = OFF

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 8000000

// Kódy segmentů pro číslice 0–9 a A–F (bit0 = A ... bit6 = G, 1 = svítí)
const uint8_t seg[16] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x77, // A
    0x7C, // b
    0x39, // C
    0x5E, // d
    0x79, // E
    0x71  // F
};

uint8_t cislice[4];     // cislice[0] = DIS0 (vpravo) ... cislice[3] = DIS3 (vlevo)

// Jedno "projetí" všech 4 pozic = 8 ms
void zobraz(void)
{
    uint8_t pozice;
    for (pozice = 0; pozice < 4; pozice++)
    {
        LATA = 0;                       // nejdřív vše zhasnout (proti "duchům")
        LATD = seg[cislice[pozice]];    // nastavit segmenty
        LATA = (uint8_t)(1 << pozice);  // zapnout jednu pozici
        __delay_ms(2);
    }
    LATA = 0;
}

void main(void)
{
    uint16_t citac = 0;
    uint8_t  opakovani;

    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;

    TRISA  = 0x00;      // RA0–RA3 výběr pozice
    TRISB  = 0xF0;
    TRISC  = 0x00;
    TRISD  = 0x00;      // segmenty

    LATA = 0; LATB = 0x0F; LATC = 0; LATD = 0;

    while (1)
    {
        // rozložit číslo na jednotlivé číslice
        cislice[0] =  citac        % 10;   // jednotky
        cislice[1] = (citac / 10)  % 10;   // desítky
        cislice[2] = (citac / 100) % 10;   // stovky
        cislice[3] = (citac / 1000) % 10;  // tisíce

        // 60 x 8 ms = cca 0,5 s zobrazování
        for (opakovani = 0; opakovani < 60; opakovani++)
        {
            zobraz();
        }

        citac++;
        if (citac > 9999) citac = 0;
    }
}
