/*
 * ÚLOHA 3 – Hodiny + stopky s nastavením času (klávesnice + 7seg displej)
 *
 * Ovládání:
 *   A  – režim HODINY   (zobrazí HH.MM, tečka uprostřed bliká každou sekundu)
 *   B  – režim STOPKY   (do 1 minuty SS.ss = sekundy.setiny, pak MM.SS)
 *   C  – nastavení hodin: napiš 4 číslice HHMM (např. 0 9 4 5 = 09:45),
 *        po 4. číslici se čas uloží; neplatný čas = zadávání znovu
 *   #  – v nastavení: zrušit;  ve stopkách: vynulovat
 *   *  – ve stopkách: start / stop
 *
 * Zapojení:
 *   RB0–RB3 sloupce (výstup), RB4–RB7 řádky (vstup, pull-down propojkou)
 *   PORTD segmenty, RA0–RA3 výběr pozice (SW4: DIS0–DIS3 zapnout)
 *   SW3: LED na PORTA/C/D vypnout
 *
 * Časování: Timer0 dává přesný tik každé 2 ms. Při každém tiku:
 *   - přičte se čas hodin a stopek
 *   - rozsvítí se další pozice displeje (multiplex)
 *   - každých 20 ms se přečte klávesnice (zároveň debounce)
 * (Pouze s __delay_ms by hodiny ujížděly, protože čtení klávesnice
 *  a výpočty zaberou proměnlivý čas.)
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

#define NIC     0xFF    // žádná klávesa
#define KL_A    10
#define KL_B    11
#define KL_C    12
#define KL_D    13
#define KL_HVEZ 14      // *
#define KL_MRIZ 15      // #

#define SEG_POMLCKA 0x40    // jen segment G = "-"
#define SEG_DP      0x80    // desetinná tečka

#define REZIM_HODINY  0
#define REZIM_STOPKY  1
#define REZIM_NASTAV  2

// segmenty číslic 0–9
const uint8_t seg[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

// pozice v matici (řádek*4 + sloupec) -> kód klávesy
const uint8_t mapa[16] = {
    1, 2, 3, KL_A,
    4, 5, 6, KL_B,
    7, 8, 9, KL_C,
    KL_HVEZ, 0, KL_MRIZ, KL_D
};

// ---- stav programu ----
uint8_t hod = 12, min = 0, sek = 0;         // hodiny
uint16_t tik_sek = 0;                       // 500 tiků = 1 s

uint8_t sw_min = 0, sw_sek = 0, sw_set = 0; // stopky
uint8_t sw_bezi = 0;
uint8_t tik_sw = 0;                         // 5 tiků = 10 ms = 1 setina

uint8_t rezim = REZIM_HODINY;
uint8_t vstup[4];                           // zadávané číslice při nastavení
uint8_t pocet = 0;

uint8_t disp[4];        // vzory segmentů: disp[0]=DIS0 (vpravo) ... disp[3]=DIS3 (vlevo)
uint8_t pozice = 0;

// ------------------------------------------------------------------
// Přečte klávesnici, vrátí kód klávesy nebo NIC
uint8_t cti_klavesu(void)
{
    uint8_t radek, maska, sloupec = 0, nalezeno;

    if (!(PORTB & 0xF0)) return NIC;        // nic nestisknuto

    if      (PORTBbits.RB4) radek = 0;
    else if (PORTBbits.RB5) radek = 1;
    else if (PORTBbits.RB6) radek = 2;
    else                    radek = 3;

    maska = 0x10 << radek;

    while ((PORTB & maska) && sloupec < 4)  // vypínej sloupce, dokud řádek nespadne
    {
        LATB <<= 1;
        sloupec++;
        __delay_us(50);
    }
    nalezeno = !(PORTB & maska);
    LATB = 0x0F;                            // obnovit sloupce

    if (!nalezeno || sloupec == 0) return NIC;
    return mapa[radek * 4 + (sloupec - 1)];
}

// ------------------------------------------------------------------
// Reakce na stisk klávesy
void zpracuj(uint8_t k)
{
    if (k == KL_A) { rezim = REZIM_HODINY; return; }
    if (k == KL_B) { rezim = REZIM_STOPKY; return; }
    if (k == KL_C) { rezim = REZIM_NASTAV; pocet = 0; return; }

    if (rezim == REZIM_STOPKY)
    {
        if (k == KL_HVEZ) sw_bezi = !sw_bezi;           // start/stop
        if (k == KL_MRIZ) { sw_bezi = 0; sw_min = 0; sw_sek = 0; sw_set = 0; tik_sw = 0; }
    }
    else if (rezim == REZIM_NASTAV)
    {
        if (k == KL_MRIZ) { rezim = REZIM_HODINY; return; }   // zrušit
        if (k <= 9)                                          // číslice
        {
            vstup[pocet++] = k;
            if (pocet == 4)
            {
                uint8_t h = vstup[0] * 10 + vstup[1];
                uint8_t m = vstup[2] * 10 + vstup[3];
                if (h < 24 && m < 60)
                {
                    hod = h; min = m; sek = 0; tik_sek = 0;
                    rezim = REZIM_HODINY;
                }
                else
                {
                    pocet = 0;                               // neplatné – znovu
                }
            }
        }
    }
}

// ------------------------------------------------------------------
// Naplní disp[] podle aktuálního režimu
void priprav_displej(void)
{
    uint8_t vlevo, vpravo, i;

    if (rezim == REZIM_NASTAV)
    {
        for (i = 0; i < 4; i++)
            disp[3 - i] = (i < pocet) ? seg[vstup[i]] : SEG_POMLCKA;
        return;
    }

    if (rezim == REZIM_HODINY) { vlevo = hod; vpravo = min; }
    else if (sw_min == 0)      { vlevo = sw_sek; vpravo = sw_set; }
    else                       { vlevo = sw_min; vpravo = sw_sek; }

    disp[3] = seg[vlevo / 10];
    disp[2] = seg[vlevo % 10];
    disp[1] = seg[vpravo / 10];
    disp[0] = seg[vpravo % 10];

    // tečka jako dvojtečka: u hodin bliká, u stopek svítí stále
    if (rezim == REZIM_STOPKY || tik_sek < 250) disp[2] |= SEG_DP;
}

// ------------------------------------------------------------------
void main(void)
{
    uint8_t tik_kl = 0;
    uint8_t k, posledni = NIC;

    ANSELA = 0; ANSELB = 0; ANSELC = 0; ANSELD = 0;
    TRISA = 0x00;
    TRISB = 0xF0;           // RB4–RB7 vstupy
    TRISC = 0x00;
    TRISD = 0x00;
    LATA = 0; LATB = 0x0F; LATC = 0; LATD = 0;

    // Timer0: Fosc/4 = 2 MHz, předdělička 1:32 -> 62 500 Hz
    //         perioda 125 (TMR0H = 124) -> 500 Hz = tik každé 2 ms
    T0CON1 = 0b01000101;    // T0CS = Fosc/4, synchronní, předdělička 1:32
    TMR0H  = 124;           // v 8bitovém režimu = porovnávací hodnota
    TMR0L  = 0;
    PIR0bits.TMR0IF = 0;
    T0CON0 = 0b10000000;    // zapnout, 8bitový režim, postdělička 1:1

    while (1)
    {
        while (!PIR0bits.TMR0IF);   // čekej na tik (2 ms)
        PIR0bits.TMR0IF = 0;

        // --- hodiny ---
        if (++tik_sek >= 500)
        {
            tik_sek = 0;
            if (++sek >= 60) { sek = 0;
                if (++min >= 60) { min = 0;
                    if (++hod >= 24) hod = 0; } }
        }

        // --- stopky ---
        if (sw_bezi && ++tik_sw >= 5)
        {
            tik_sw = 0;
            if (++sw_set >= 100) { sw_set = 0;
                if (++sw_sek >= 60) { sw_sek = 0;
                    if (++sw_min >= 100) sw_min = 0; } }
        }

        // --- klávesnice každých 20 ms, reaguje jen na nový stisk ---
        if (++tik_kl >= 10)
        {
            tik_kl = 0;
            k = cti_klavesu();
            if (k != NIC && posledni == NIC) zpracuj(k);
            posledni = k;
        }

        // --- displej: jedna pozice za tik ---
        priprav_displej();
        LATA = 0;                           // zhasnout (proti "duchům")
        LATD = disp[pozice];
        LATA = (uint8_t)(1 << pozice);
        pozice = (pozice + 1) & 0x03;
    }
}
