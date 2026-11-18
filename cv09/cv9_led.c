// ---------------- Cviceni 9a - STM32F103RB, blikani LED ----------------
// LED (PICSimLab): PB0 - PB15, LED sviti pri log. 1
// Hodiny: po resetu interni HSI = 8 MHz

#include <stdint.h>
#ifndef STM32F103xB
#define STM32F103xB             // typ procesoru pro stm32f1xx.h
#endif
#include <stm32f1xx.h>

// Jednoduche zpozdeni smyckou (neni presne)
void Delay(volatile uint32_t n)
{
    while (n > 0) n--;
}

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;     // povoleni hodin pro GPIOB
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;     // povoleni hodin pro AFIO
    AFIO->MAPR   |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE; // vypnuti JTAG -> PB3, PB4 budou normalni piny

    // Kazdy pin ma 4 bity: CNF[1:0] MODE[1:0]
    // 0x2 = 0010 -> CNF=00 (push-pull), MODE=10 (vystup 2 MHz)
    GPIOB->CRL &= 0x00000000;   // vynulovani nastaveni PB0 - PB7
    GPIOB->CRL |= 0x22222222;   // PB0 - PB7 vystupy
    GPIOB->CRH &= 0x00000000;   // vynulovani nastaveni PB8 - PB15
    GPIOB->CRH |= 0x22222222;   // PB8 - PB15 vystupy

    GPIOB->ODR = 0x5555;        // pocatecni stav: sviti kazda druha LED

    for (;;)
    {
        GPIOB->ODR ^= 0xFFFF;   // zmena stavu vsech LED (negace)
        Delay(200000);
    }
}
