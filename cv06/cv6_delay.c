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

// ---------------- Cviceni 6a - GLCD 128x64 (KS0108), verze se zpozdenim ----------------
// PORTB = ridici signaly, PORTD = 8bitova datova sbernice

#include <xc.h>
#include <stdint.h>
#include "font5x7.h"

#define _XTAL_FREQ 8000000

#define GLCD_DataPort LATD          // datova sbernice (zapis)

#define GLCD_CS1 LATBbits.LATB0     // vyber leve poloviny displeje (sloupce 0-63)
#define GLCD_CS2 LATBbits.LATB1     // vyber prave poloviny displeje (sloupce 64-127)
#define GLCD_RS  LATBbits.LATB2     // 0 = prikaz, 1 = data
#define GLCD_RW  LATBbits.LATB3     // 0 = zapis, 1 = cteni
#define GLCD_EN  LATBbits.LATB4     // hodinovy (enable) signal
#define GLCD_RST LATBbits.LATB5     // reset displeje (0 = reset)

// Pulz na EN - displej si prevezme to, co je na sbernici
void Odesli(void)
{
    GLCD_EN = 1;
    __delay_us(1);
    GLCD_EN = 0;
}

// Poslani prikazu do obou polovin displeje najednou
void Send_Command12(unsigned char cmd)
{
    GLCD_CS1 = 1;           // vybrana leva polovina
    GLCD_CS2 = 1;           // vybrana prava polovina
    GLCD_RW  = 0;           // zapis
    GLCD_RS  = 0;           // prikaz
    __delay_us(3);
    GLCD_DataPort = cmd;    // prikaz na datovou sbernici
    Odesli();
    __delay_ms(2);          // pockame, nez displej prikaz zpracuje
}

// Poslani datoveho bajtu (8 svislych bodu) do obou polovin displeje
void Send_Data12(unsigned char dat)
{
    GLCD_CS1 = 1;
    GLCD_CS2 = 1;
    GLCD_RW  = 0;           // zapis
    GLCD_RS  = 1;           // data
    __delay_us(3);
    GLCD_DataPort = dat;    // data na datovou sbernici
    Odesli();
    __delay_ms(2);
}

// Vykresleni jednoho znaku (font ma 6 bajtu na znak, zacina mezerou ' ')
void Send_char_font5x7(char ch)
{
    uint8_t i;
    for (i = 1; i < 7; i++)
    {
        Send_Data12(font5x7[(ch - ' ') * 6 + i]);   // i-ty sloupec znaku
    }
}

// Vypis retezce znak po znaku
void Send_String_font5x7(const char *s)
{
    while (*s)
    {
        Send_char_font5x7(*s);
        s++;
    }
}

void main(void)
{
    uint8_t m, i;

    ANSELB = 0;             // PORTB digitalni
    ANSELD = 0;             // PORTD digitalni
    TRISB  = 0;             // PORTB vystup
    TRISD  = 0;             // PORTD vystup

    GLCD_RST = 0;           // reset displeje
    __delay_ms(10);
    GLCD_RST = 1;           // konec resetu
    __delay_ms(10);

    Send_Command12(0b00111111);  // Display ON (0x3F)
    Send_Command12(0b11000000);  // Start line = 0 (0xC0)
    Send_Command12(0b10111000);  // Set page = 0 (0xB8 + stranka 0..7)
    Send_Command12(0b01000000);  // Set address (sloupec) = 0 (0x40 + sloupec 0..63)

    for (m = 1; m < 9; m++)      // 8 stranek (radku po 8 bodech)
    {
        for (i = 0; i < 64; i++) // 64 sloupcu v kazde polovine
        {
            Send_Data12(0b01010101);       // vodorovne linky (kazdy druhy bod)
        }
        Send_Command12(0b10111000 + m);    // prechod na dalsi stranku
    }

    // Vyzkouseni textu - stranka 3, sloupec 10
    Send_Command12(0b10111000 + 3);        // stranka 3
    Send_Command12(0b01000000 + 10);       // sloupec 10
    Send_String_font5x7(" AHOJ  ");        // pozn.: obe poloviny jsou vybrane, text bude 2x

    while (1);
}
