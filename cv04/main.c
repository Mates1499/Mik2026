#include "config.h"

/*
 * Timer0 math (Fosc = 8 MHz):
 *   Timer clock = Fosc/4 = 2 MHz
 *   Prescaler 1:32 -> 2 MHz / 32 = 62 500 ticks per second
 *   16-bit timer overflows at 65 536, so start it at
 *   65 536 - 62 500 = 3036 = 0x0BDC  -> overflow every 1 s
 */
#define TMR0_START_H 0x0B
#define TMR0_START_L 0xDC

// HIGH priority: Timer0 + INT0 (RB0)
void __interrupt(high_priority) isr_high(void)
{
    if (PIR0bits.TMR0IF)            // Timer0 overflow?
    {
        TMR0H = TMR0_START_H;       // reload for next 1 s (H first, then L)
        TMR0L = TMR0_START_L;
        LATDbits.LATD0 = !LATDbits.LATD0;   // blink RD0
        PIR0bits.TMR0IF = 0;
    }

    if (PIR0bits.INT0IF)            // button on RB0?
    {
        LATDbits.LATD7 = !LATDbits.LATD7;   // toggle RD7
        PIR0bits.INT0IF = 0;
    }
}

// LOW priority: INT1 (RB1) + INT2 (RB2), with a long delay
void __interrupt(low_priority) isr_low(void)
{
    if (PIR0bits.INT1IF)            // button on RB1?
    {
        LATDbits.LATD6 = !LATDbits.LATD6;   // toggle RD6
        __delay_ms(5000);           // during this, RB0 + timer still work
        PIR0bits.INT1IF = 0;
    }

    if (PIR0bits.INT2IF)            // button on RB2?
    {
        LATDbits.LATD5 = !LATDbits.LATD5;   // toggle RD5
        __delay_ms(5000);
        PIR0bits.INT2IF = 0;
    }
}

void main(void)
{
    // Port D = LEDs (digital outputs, all off)
    ANSELD = 0x00;
    TRISD  = 0x00;
    LATD   = 0x00;

    // Port B = buttons (digital inputs). INT0=RB0, INT1=RB1, INT2=RB2 by default
    ANSELB = 0x00;
    TRISB  = 0xFF;

    // Timer0
    T0CON1 = 0b01000101;    // Fosc/4, synchronized, prescaler 1:32
    T0CON0 = 0b10010000;    // Timer0 ON, 16-bit, postscaler 1:1
    TMR0H  = TMR0_START_H;
    TMR0L  = TMR0_START_L;

    // Edge for buttons: 1 = rising (press when button gives 1)
    INTCONbits.INT0EDG = 1;
    INTCONbits.INT1EDG = 1;
    INTCONbits.INT2EDG = 1;

    // Priorities: 1 = high, 0 = low
    IPR0bits.TMR0IP = 1;
    IPR0bits.INT0IP = 1;
    IPR0bits.INT1IP = 0;
    IPR0bits.INT2IP = 0;

    // Clear flags
    PIR0bits.TMR0IF = 0;
    PIR0bits.INT0IF = 0;
    PIR0bits.INT1IF = 0;
    PIR0bits.INT2IF = 0;

    // Enable interrupts
    PIE0bits.TMR0IE = 1;
    PIE0bits.INT0IE = 1;
    PIE0bits.INT1IE = 1;
    PIE0bits.INT2IE = 1;

    INTCONbits.IPEN = 1;    // use priorities
    INTCONbits.GIEL = 1;    // enable low priority interrupts
    INTCONbits.GIEH = 1;    // enable high priority interrupts

    while (1)
    {
        // nothing - everything happens in interrupts
    }
}
