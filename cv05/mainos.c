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


#pragma config LVP = OFF        // Low Voltage Programming Enable bit (HV on MCLR/VPP must be used for programming)



#include <xc.h>
#include <stdint.h>
#include <pic18f47k40.h>
#include "lcd_PIC18F47K40.h"

#define _XTAL_FREQ 8000000




void main(void) 
{
    ANSELB = 0b00000000;    // PORTB digitalni
    TRISB  = 0b00000000;    // PORTB vystup
    LATB   = 0b00000000;    // vse na 0
    __delay_ms(50);         // pockame, nez displej po zapnuti nabehne

 // Inicializace radice LCD - str. 46 v lcd_hitachi44780.pdf
 // (Send_Command posila 2 pulky: 0000 a pak 0011 -> ta prvni je "nic", druha je ta dulezita)
    Send_Command (0b00000011);   // 0011 - "jsem v 8bit rezimu" (1. pokus)
    Send_Command (0b00000011);   // 0011 - (2. pokus)
    Send_Command (0b00000011);   // 0011 - (3. pokus)
    Send_Command (0b00000010);   // 0010 - prepni do 4bitoveho rezimu
    Send_Command (0b00101000);   // Function set: 4 bity (DL=0), 2 radky (N=1), font 5x8 (F=0)
    Send_Command (0b00001000);   // Display OFF
    Send_Command (0b00000001);   // Clear display (smaze displej, kurzor domu)
    Send_Command (0b00000110);   // Entry mode: kurzor jde doprava (I/D=1), bez posunu (S=0)

 // Nastaveni parametru a zobrazeni znaku - str. 43 v lcd_hitachi44780.pdf
    Send_Command (0b00101000);   // krok 2: Function set  0 0 1 DL N F - -   (DL=0 -> 4 bity)
    Send_Command (0b00001110);   // krok 3: Display ON    0 0 0 0 1 D C B   (D=1 displej, C=1 kurzor, B=0 neblika)
    Send_Command (0b00000110);   // krok 4: Entry mode    0 0 0 0 0 1 I/D S (I/D=1 doprava, S=0)

    Send_Data ('H');
    Send_Data ('e');
    Send_Data ('l');
    Send_Data ('l');
    Send_Data ('o');

 // ---------- Ostatni: vyzkouseni dalsich funkci (tabulka 6, str. 24-25) ----------

 // Skok na 2. radek: prikaz "Set DDRAM address" = 1 + adresa
 // 1. radek zacina adresou 0x00 -> prikaz 0x80
 // 2. radek zacina adresou 0x40 -> prikaz 0xC0
    Send_Command (0b11000000);   // kurzor na zacatek 2. radku
    Send_Data ('P');
    Send_Data ('I');
    Send_Data ('C');
    Send_Data (' ');

 // Vlastni znak (srdicko) - ulozi se do CGRAM na pozici 0
 // prikaz "Set CGRAM address" = 01 + adresa -> 0x40 = znak c. 0
    Send_Command (0b01000000);
    Send_Data (0b00000);         // kazdy radek znaku = 5 bodu (5 spodnich bitu)
    Send_Data (0b01010);         //  . X . X .
    Send_Data (0b11111);         //  X X X X X
    Send_Data (0b11111);         //  X X X X X
    Send_Data (0b01110);         //  . X X X .
    Send_Data (0b00100);         //  . . X . .
    Send_Data (0b00000);
    Send_Data (0b00000);

    Send_Command (0b11000100);   // zpet do DDRAM: 2. radek, 5. pozice (0x40 + 4)
    Send_Data (0);               // vypise vlastni znak c. 0 (srdicko)

 // Vypis zprava doleva: Entry mode s I/D = 0
    Send_Command (0b00000100);   // kurzor se ted posouva doleva
    Send_Command (0b11001111);   // kurzor na posledni pozici 2. radku (0x4F)
    Send_Data ('A');             // na displeji bude "...BA" konci radku - jde to pozpatku
    Send_Data ('B');
    Send_Command (0b00000110);   // zpet: kurzor doprava

    __delay_ms(3000);            // 3 s se podivame na vysledek

    while(1)
    {
        // Posun celeho displeje doleva: prikaz Cursor/Display shift  0001 S/C R/L - -
        // S/C=1 (posouva se displej), R/L=0 (doleva)
        Send_Command (0b00011000);
        __delay_ms(500);
    }
    
    return;
}
