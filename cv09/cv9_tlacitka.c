// ---------------- Cviceni 9b - STM32F103RB, tlacitka -> LED ----------------
// Tlacitka (PICSimLab): PA0 - PA7, stisk = log. 1
// LED: PB0 - PB7, sviti pri log. 1
// Stisknuti tlacitka na PAx rozsviti LED na PBx.

#include <stdint.h>
#ifndef STM32F103xB
#define STM32F103xB
#endif
#include <stm32f1xx.h>

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;     // hodiny pro GPIOA (tlacitka)
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;     // hodiny pro GPIOB (LED)
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;     // hodiny pro AFIO
    AFIO->MAPR   |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE; // PB3, PB4 jako normalni piny

    // PB0 - PB7 vystupy: 0x2 = CNF=00 push-pull, MODE=10 vystup 2 MHz
    GPIOB->CRL &= 0x00000000;
    GPIOB->CRL |= 0x22222222;

    // PA0 - PA7 vstupy: 0x8 = CNF=10 (vstup s pull-up/down), MODE=00 (vstup)
    GPIOA->CRL &= 0x00000000;
    GPIOA->CRL |= 0x88888888;
    GPIOA->ODR &= ~0x00FF;      // ODR = 0 -> pull-down (nestisknute = 0)

    for (;;)
    {
        uint32_t tl = GPIOA->IDR & 0x00FF;      // precteni tlacitek PA0 - PA7

        GPIOB->ODR = (GPIOB->ODR & ~0x00FF) | tl;   // zapis na LED PB0 - PB7
    }
}
