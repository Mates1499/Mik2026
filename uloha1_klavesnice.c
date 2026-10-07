/*
 * ÚLOHA 1 – Obsluha klávesnice 4x4 + indikace na LED
 *
 * Zapojení:
 *   RB0–RB3 = sloupce (výstupy, log. 1)
 *   RB4–RB7 = řádky   (vstupy, pull-down – nastavit propojkou na desce!)
 *   PORTC, PORTD = LED (na SW3 zapnout PORTC a PORTD)
 *
 * Klávesy 1. a 2. řádku svítí na RC0–RC7, 3. a 4. řádku na RD0–RD7:
 *   1 2 3 A  -> RC0 RC1 RC2 RC3
 *   4 5 6 B  -> RC4 RC5 RC6 RC7
 *   7 8 9 C  -> RD0 RD1 RD2 RD3
 *   * 0 # D  -> RD4 RD5 RD6 RD7
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

uint8_t radek, sloupec, maska, klavesa;

void main(void)
{
    ANSELA = 0x00;          // vše digitální
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;

    TRISA  = 0x00;          // výstup
    TRISB  = 0b11110000;    // RB7–RB4 vstupy (řádky), RB3–RB0 výstupy (sloupce)
    TRISC  = 0x00;          // LED
    TRISD  = 0x00;          // LED

    LATA   = 0x00;
    LATB   = 0b00001111;    // všechny sloupce na log. 1
    LATC   = 0x00;
    LATD   = 0x00;

    while (1)
    {
        if (PORTB & 0xF0)                   // je některý řádek v log. 1 = stisk
        {
            // 1) který řádek
            if      (PORTBbits.RB4) radek = 0;
            else if (PORTBbits.RB5) radek = 1;
            else if (PORTBbits.RB6) radek = 2;
            else                    radek = 3;

            maska = 0x10 << radek;          // bit řádku: 0x10, 0x20, 0x40, 0x80

            // 2) který sloupec – postupně vypínáme sloupce RB0, RB1, ...
            sloupec = 0;
            while ((PORTB & maska) && sloupec < 4)
            {
                LATB <<= 1;
                sloupec++;
                __delay_ms(1);              // ustálení napětí
            }
            LATB = 0b00001111;              // obnovit všechny sloupce

            // 3) číslo klávesy 0..15
            klavesa = radek * 4 + (sloupec - 1);

            // 4) rozsviť jednu LED
            LATC = 0;
            LATD = 0;
            if (klavesa < 8) LATC = (uint8_t)(1 << klavesa);
            else             LATD = (uint8_t)(1 << (klavesa - 8));
        }
        else
        {
            LATC = 0;                       // nic nestisknuto
            LATD = 0;
        }
        __delay_ms(10);                     // zároveň jednoduchý debounce
    }

}
/* #include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 8000000

uint8_t radek, sloupec, klavesa;

void main(void)
{
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;

    TRISA  = 0x00;
    TRISB  = 0b11110000;    // RB7–RB4 vstupy (řádky), RB3–RB0 výstupy (sloupce)
    TRISC  = 0x00;          // LED
    TRISD  = 0x00;          // LED

    LATA   = 0x00;
    LATB   = 0b00001111;    // všechny sloupce v log. 1
    LATC   = 0x00;
    LATD   = 0x00;

    while (1)
    {
        // 1) Kontrola, zda je stisknuta jakákoliv klávesa
        if (PORTB & 0xF0)
        {
            // Detekce stisknutého řádku (RB4 = 0, RB5 = 1, RB6 = 2, RB7 = 3)
            if      (PORTBbits.RB4) radek = 0;
            else if (PORTBbits.RB5) radek = 1;
            else if (PORTBbits.RB6) radek = 2;
            else                    radek = 3;

            // 2) Zjištění sloupce: aktivujeme vždy pouze JEDEN sloupec
            sloupec = 0;
            for (uint8_t c = 0; c < 4; c++)
            {
                LATB = (uint8_t)(1 << c);   // aktivní pouze sloupec RB0, RB1, RB2 nebo RB3
                __delay_ms(1);              // ustálení napětí

                // Pokud na daném řádku stále čteme log. 1, máme správný sloupec
                if (PORTB & (1 << (radek + 4)))
                {
                    sloupec = c;
                    break;
                }
            }

            LATB = 0b00001111;              // vrátit log. 1 na všechny sloupce

            // 3) Výpočet indexu klávesy (0 až 15)
            klavesa = (radek * 4) + sloupec;

            // 4) Rozsvícení odpovídající LED
            LATC = 0x00;
            LATD = 0x00;

            if (klavesa < 8)
            {
                LATC = (uint8_t)(1 << klavesa);         // Řádek 0 a 1 -> RC0 až RC7
            }
            else
            {
                LATD = (uint8_t)(1 << (klavesa - 8));   // Řádek 2 a 3 -> RD0 až RD7
            }
        }
        else
        {
            LATC = 0x00;
            LATD = 0x00;
        }

        __delay_ms(10); // debounce
    }
}*/