#include <xc.h>
#include "lcd_PIC18F47K40.h"

// Zapojeni na PORTB:
//   RB0..RB3 = datove vodice D4..D7 displeje
//   RB4      = RS  (0 = prikaz, 1 = znak/data)
//   RB5      = EN  (impuls 1->0 = displej si precte data)

// Kratky impuls na EN -> LCD si "vezme" to, co je prave na datovych vodicich
void Send()  
{
  LCD_EN = 1;
  __delay_us(1);
  LCD_EN = 0;
}

// Poslani PRIKAZU (RS = 0)
// Komunikace je 4bitova -> bajt se posila nadvakrat: nejdriv horni 4 bity, pak dolni 4 bity
void Send_Command (char cmd)
{
  LCD_DATA = cmd >> 4;      // horni pulka bajtu na RB0..RB3
  LCD_RS = 0;               // 0 = prikaz
  Send();
  LCD_DATA = cmd & 0x0F;    // dolni pulka bajtu na RB0..RB3
  LCD_RS = 0;
  Send();
  __delay_ms(2);            // prikazy (hlavne Clear) trvaji az 1.52 ms
}

// Poslani ZNAKU (RS = 1) - ASCII kod znaku se zobrazi na displeji
void Send_Data (char dat)
{
  LCD_DATA = dat >> 4;      // horni pulka bajtu
  LCD_RS = 1;               // 1 = data (znak)
  Send();
  LCD_DATA = dat & 0x0F;    // dolni pulka bajtu
  LCD_RS = 1;
  Send();
  __delay_us(50);           // zapis znaku trva cca 40 us
}
