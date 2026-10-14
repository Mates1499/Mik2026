#include "config.h"
#include <stdint.h>
#include <pic18f47k40.h>

// Obsluha přerušení VYŠŠÍ priority (vektor 0x0008)
void __interrupt(high_priority) HighIsr(void)
{
    if (PIR0bits.TMR0IF && PIE0bits.TMR0IE)
    {
        LATDbits.LATD0 = ~LATDbits.LATD0; // Blikání LED D0 (1 s)
        PIR0bits.TMR0IF = 0;
    }

    if (PIR0bits.INT0IF && PIE0bits.INT0IE)
    {
        LATDbits.LATD1 = ~LATDbits.LATD1; // Okamžitá reakce na RB0
        PIR0bits.INT0IF = 0;
    }
}

// Obsluha přerušení NIŽŠÍ priority (vektor 0x0018)
void __interrupt(low_priority) LowIsr(void)
{
    if (PIR0bits.INT1IF && PIE0bits.INT1IE)
    {
        LATDbits.LATD2 = 1;               // Rozsvítí LED D2
        __delay_ms(5000);                 // Čeká 5 s
        LATDbits.LATD2 = 0;               // Zhasne LED D2
        PIR0bits.INT1IF = 0;
    }

    if (PIR0bits.INT2IF && PIE0bits.INT2IE)
    {
        LATDbits.LATD3 = 1;               // Rozsvítí LED D3
        __delay_ms(5000);                 // Čeká 5 s
        LATDbits.LATD3 = 0;               // Zhasne LED D3
        PIR0bits.INT2IF = 0;
    }
}

void main(void)
{
    // 1. Nastavení PORTD (LED) jako digitální výstup
    ANSELD = 0x00;
    TRISD = 0x00;
    LATD = 0x00;

    // 2. Nastavení PORTB (RB0, RB1, RB2) jako digitální vstupy + Pull-Up
    ANSELB = 0x00;                        // Digitální vstupy
    TRISB |= 0x07;                        // RB0, RB1, RB2 jako vstupy
    WPUB |= 0x07;                         // Zapnutí interních pull-up rezistorů na RB0, RB1, RB2

    // Explicitní namapování PPS na RB0, RB1, RB2 (pro jistotu)
    INT0PPS = 0x08;                       // RB0 -> INT0
    INT1PPS = 0x09;                       // RB1 -> INT1
    INT2PPS = 0x0A;                       // RB2 -> INT2

    // 3. Nastavení Timer0 na 1 s
    T0CON1 = 0b01000101;                  // Fosc/4, sync, prescaler 1:32
    T0CON0 = 0b10010000;                  // TMR0 ON, 16-bit, postscaler 1:1

    // 4. Nastavení hrany (0 = při stisku do GND, 1 = při stisku do VCC)
    INTCONbits.INT0EDG = 0;
    INTCONbits.INT1EDG = 0;
    INTCONbits.INT2EDG = 0;

    // 5. Nastavení priorit přerušení
    INTCONbits.IPEN = 1;                  // Povolení dvou úrovní priorit
    IPR0bits.TMR0IP = 1;                  // TMR0 = vysoká priorita
    IPR0bits.INT0IP = 1;                  // INT0 = vysoká priorita
    IPR0bits.INT1IP = 0;                  // INT1 = nízká priorita
    IPR0bits.INT2IP = 0;                  // INT2 = nízká priorita

    // 6. Smazání příznaků a povolení přerušení
    PIR0bits.TMR0IF = 0;
    PIR0bits.INT0IF = 0;
    PIR0bits.INT1IF = 0;
    PIR0bits.INT2IF = 0;

    PIE0bits.TMR0IE = 1;
    PIE0bits.INT0IE = 1;
    PIE0bits.INT1IE = 1;
    PIE0bits.INT2IE = 1;

    // 7. Globální povolení přerušení (vysoké i nízké priority)
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    while (1)
    {
    }
}