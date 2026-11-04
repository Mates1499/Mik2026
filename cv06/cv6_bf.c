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

// ---------------- Cviceni 6b - GLCD 128x64 (KS0108), verze s ctenim Busy Flagu ----------------
// Misto pevnych zpozdeni (__delay_ms) se ceka, dokud displej neni pripraven (BF = 0).

#include <xc.h>
#include <stdint.h>
#include "font5x7.h"

#define _XTAL_FREQ 8000000

#define GLCD_DataPort LATD          // datova sbernice - zapis
#define GLCD_DataIn   PORTD         // datova sbernice - cteni
#define GLCD_DataDir  TRISD         // smer sbernice (0 = vystup, 1 = vstup)

#define GLCD_CS1 LATBbits.LATB0     // vyber leve poloviny
#define GLCD_CS2 LATBbits.LATB1     // vyber prave poloviny
#define GLCD_RS  LATBbits.LATB2     // 0 = prikaz/status, 1 = data
#define GLCD_RW  LATBbits.LATB3     // 0 = zapis, 1 = cteni
#define GLCD_EN  LATBbits.LATB4     // enable
#define GLCD_RST LATBbits.LATB5     // reset

// Pulz na EN
void Odesli(void)
{
    GLCD_EN = 1;
    __delay_us(1);
    GLCD_EN = 0;
}

// Ceka, az jeden radic (cs1 = 1 leva, cs1 = 0 prava polovina) prestane byt busy
void Wait_BF_one(uint8_t cs1)
{
    uint8_t status;

    GLCD_DataDir = 0xFF;    // sbernice jako vstup (cteme z displeje)
    GLCD_CS1 = cs1;         // vybereme jen jeden radic
    GLCD_CS2 = !cs1;
    GLCD_RS  = 0;           // cteni statusu
    GLCD_RW  = 1;           // cteni

    do
    {
        GLCD_EN = 1;
        __delay_us(1);
        status = GLCD_DataIn;   // precteme status (bit 7 = BUSY)
        GLCD_EN = 0;
        __delay_us(1);
    } while (status & 0x80);    // dokud je BUSY = 1, cekame

    GLCD_RW = 0;
    GLCD_DataDir = 0x00;    // sbernice zpet jako vystup
}

// Ceka na oba radice (obe poloviny displeje)
void Wait_BF(void)
{
    Wait_BF_one(1);         // leva polovina
    Wait_BF_one(0);         // prava polovina
}

// Prikaz do obou polovin
void Send_Command12(unsigned char cmd)
{
    Wait_BF();              // misto __delay_ms(2)
    GLCD_CS1 = 1;
    GLCD_CS2 = 1;
    GLCD_RW  = 0;           // zapis
    GLCD_RS  = 0;           // prikaz
    GLCD_DataPort = cmd;
    Odesli();
}

// Data do obou polovin
void Send_Data12(unsigned char dat)
{
    Wait_BF();              // misto __delay_ms(2)
    GLCD_CS1 = 1;
    GLCD_CS2 = 1;
    GLCD_RW  = 0;           // zapis
    GLCD_RS  = 1;           // data
    GLCD_DataPort = dat;
    Odesli();
}

// Jeden znak fontu 5x7 (6 bajtu na znak, prvni znak v tabulce je mezera)
void Send_char_font5x7(char ch)
{
    uint8_t i;
    for (i = 1; i < 7; i++)
    {
        Send_Data12(font5x7[(ch - ' ') * 6 + i]);
    }
}

// Retezec
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
    ANSELD = 0;             // PORTD digitalni (nutne i pro cteni BF!)
    TRISB  = 0;
    TRISD  = 0;

    GLCD_RST = 0;           // reset displeje
    __delay_ms(10);
    GLCD_RST = 1;
    __delay_ms(10);         // po resetu jeste kratce pockame

    Send_Command12(0b00111111);  // Display ON
    Send_Command12(0b11000000);  // Start line = 0
    Send_Command12(0b10111000);  // Page = 0
    Send_Command12(0b01000000);  // Address = 0

    for (m = 1; m < 9; m++)      // vodorovne linky pres cely displej
    {
        for (i = 0; i < 64; i++)
        {
            Send_Data12(0b01010101);
        }
        Send_Command12(0b10111000 + m);
    }

    Send_Command12(0b10111000 + 3);   // stranka 3
    Send_Command12(0b01000000 + 10);  // sloupec 10
    Send_String_font5x7(" AHOJ  ");

    while (1);
}
