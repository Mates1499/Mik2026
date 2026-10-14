/*
 * NEJJEDNODUŠŠÍ STOPKY – vše v main(), bez vlastních funkcí
 *
 * Displej ukazuje  SS.cc  (sekundy . setiny), max. 99.99 s
 *   *  = start / stop
 *   #  = vynulovat
 *
 * Deska: SW4 -> DIS0–DIS3 ON, SW3 -> LED na PORTA/C/D OFF,
 *        PORTB pull-down.
 *
 * Jeden průchod smyčkou = 4 pozice × 2,5 ms = 10 ms = 1 setina.
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

// segmenty číslic 0–9 (bit0 = A ... bit6 = G, bit7 = tečka)
const uint8_t seg[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

uint8_t sek = 0;        // sekundy
uint8_t set = 0;        // setiny
uint8_t bezi = 0;       // 1 = stopky běží
uint8_t hvezda = 0;     // je teď stisknuta * ?
uint8_t predtim = 0;    // byla * stisknuta minule?

void main(void)
{
    ANSELA = 0; ANSELB = 0; ANSELC = 0; ANSELD = 0;
    TRISA = 0x00;       // RA0–RA3 výběr pozice displeje
    TRISB = 0xF0;       // RB4–RB7 vstupy (řádky), RB0–RB3 výstupy (sloupce)
    TRISC = 0x00;
    TRISD = 0x00;       // segmenty
    LATA = 0; LATB = 0; LATC = 0; LATD = 0;

    while (1)
    {
        // ---------- KLÁVESNICE ----------
        // * a # jsou obě ve 4. řádku (RB7).
        // Zapnu jen jeden sloupec a podívám se, jestli RB7 = 1.

        LATB = 0b00000001;          // jen sloupec RB0 (klávesa *)
        __delay_us(10);
        hvezda = PORTBbits.RB7;

        LATB = 0b00000100;          // jen sloupec RB2 (klávesa #)
        __delay_us(10);
        if (PORTBbits.RB7 == 1)     // # = vynulovat
        {
            bezi = 0;
            sek = 0;
            set = 0;
        }

        // * přepne start/stop jen v okamžiku stisku (ne při držení)
        if (hvezda == 1 && predtim == 0)
        {
            bezi = !bezi;
        }
        predtim = hvezda;

        // ---------- POČÍTÁNÍ ČASU ----------
        if (bezi == 1)
        {
            set++;
            if (set >= 100)
            {
                set = 0;
                sek++;
                if (sek >= 100) sek = 0;
            }
        }

        // ---------- DISPLEJ (multiplex, celkem 10 ms) ----------
        LATA = 0; LATD = seg[set % 10];        LATA = 0b0001; __delay_us(2500); // DIS0 setiny jednotky
        LATA = 0; LATD = seg[set / 10];        LATA = 0b0010; __delay_us(2500); // DIS1 setiny desítky
        LATA = 0; LATD = seg[sek % 10] | 0x80; LATA = 0b0100; __delay_us(2500); // DIS2 sekundy + tečka
        LATA = 0; LATD = seg[sek / 10];        LATA = 0b1000; __delay_us(2500); // DIS3 sekundy desítky
    }
}
