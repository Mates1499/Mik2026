#include "config.h"
#include <stdint.h>
#include <pic18f47k40.h>

// Obsluha přerušení VYŠŠÍ priority
void __interrupt(high_priority) HighIsr(void)
{
    // Přerušení od časovače TMR0
    if (PIR0bits.TMR0IF && PIE0bits.TMR0IE)
    {
        LATDbits.LATD0 = ~LATDbits.LATD0; // Blikání LED D0 každou sekundu
        PIR0bits.TMR0IF = 0;              // Smazání příznaku TMR0
    }

    // Přerušení od tlačítka INT0 (vyšší priorita)
    if (PIR0bits.INT0IF && PIE0bits.INT0IE)
    {
        LATDbits.LATD1 = ~LATDbits.LATD1; // Okamžitá změna stavu LED D1
        PIR0bits.INT0IF = 0;              // Smazání příznaku INT0
    }
}

// Obsluha přerušení NIŽŠÍ priority
void __interrupt(low_priority) LowIsr(void)
{
    // Přerušení od tlačítka INT1 (nižší priorita)
    if (PIR0bits.INT1IF && PIE0bits.INT1IE)
    {
        LATDbits.LATD2 = 1;               // Rozsvícení LED D2 po dobu čekání
        __delay_ms(5000);                 // Zpoždění 5 s (během něj lze vyvolat INT0 a TMR0!)
        LATDbits.LATD2 = 0;               // Zhasnutí LED D2
        PIR0bits.INT1IF = 0;              // Smazání příznaku INT1
    }

    // Přerušení od tlačítka INT2 (nižší priorita)
    if (PIR0bits.INT2IF && PIE0bits.INT2IE)
    {
        LATDbits.LATD3 = 1;               // Rozsvícení LED D3 po dobu čekání
        __delay_ms(5000);                 // Zpoždění 5 s
        LATDbits.LATD3 = 0;               // Zhasnutí LED D3
        PIR0bits.INT2IF = 0;              // Smazání příznaku INT2
    }
}

void main(void)
{
    // 1. Nastavení PORTD (LED) jako digitální výstup
    ANSELD = 0x00;
    TRISD = 0x00;
    LATD = 0x00;                          // Na začátku všechny LED zhasnuté

    // 2. Nastavení PORTB (Tlačítka RB0, RB1, RB2) jako digitální vstupy
    ANSELB = 0x00;                        // Digitální vstupy na PORTB
    TRISBbits.TRISB0 = 1;                 // INT0
    TRISBbits.TRISB1 = 1;                 // INT1
    TRISBbits.TRISB2 = 1;                 // INT2

    // 3. Nastavení Timer0 na cca 1 s (Fosc/4, synchronní, 16-bit, prescaler 1:32)
    T0CON1 = 0b01000101;                  // CS = 010 (Fosc/4), ASYNC = 0, CKPS = 0101 (1:32)
    T0CON0 = 0b10010000;                  // EN = 1, 16BIT = 1, OUTPS = 0000 (1:1)

    // 4. Nastavení hran pro tlačítka INT0, INT1, INT2 (0 = sestupná hrana, 1 = náběžná)
    INTCONbits.INT0EDG = 0;
    INTCONbits.INT1EDG = 0;
    INTCONbits.INT2EDG = 0;

    // 5. Nastavení priorit přerušení
    INTCONbits.IPEN = 1;                  // Povolení priorit přerušení (Interrupt Priority Enable)
    
    IPR0bits.TMR0IP = 1;                  // Timer0 -> Vyšší priorita (1)
    IPR0bits.INT0IP = 1;                  // INT0   -> Vyšší priorita (1)
    IPR0bits.INT1IP = 0;                  // INT1   -> Nižší priorita (0)
    IPR0bits.INT2IP = 0;                  // INT2   -> Nižší priorita (0)

    // 6. Vynulování příznaků přerušení (PIR0)
    PIR0bits.TMR0IF = 0;
    PIR0bits.INT0IF = 0;
    PIR0bits.INT1IF = 0;
    PIR0bits.INT2IF = 0;

    // 7. Povolení jednotlivých zdrojů přerušení (PIE0)
    PIE0bits.TMR0IE = 1;
    PIE0bits.INT0IE = 1;
    PIE0bits.INT1IE = 1;
    PIE0bits.INT2IE = 1;

    // 8. Globální povolení přerušení (pro vyšší i nižší prioritu)
    INTCONbits.GIEH = 1;                  // Povolení přerušení s vyšší prioritou
    INTCONbits.GIEL = 1;                  // Povolení přerušení s nižší prioritou

    while (1)
    {
        // Nekonečná smyčka
    }

    return;
}