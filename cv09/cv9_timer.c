// ---------------- Cviceni 9c - STM32F103RB, casovac TIM3 ----------------
// TIM3 generuje UEV (preteceni) kazdou 1 s, pri kazdem UEV se zmeni stav LED.
// Hodiny casovace = 8 MHz (HSI, bez delicek APB)
// PSC = 7999 -> 8 MHz / 8000 = 1 kHz (1 tik = 1 ms)
// ARR = 999  -> 1000 tiku = 1 s
// V PICSimLabu nastav IO (MHz) = 10 a Qemu CPU MIPS = 7.81

#include <stdint.h>
#ifndef STM32F103xB
#define STM32F103xB
#endif
#include <stm32f1xx.h>

int main(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;     // hodiny pro GPIOB (LED)
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;     // hodiny pro AFIO
    AFIO->MAPR   |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE; // PB3, PB4 jako normalni piny
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;     // hodiny pro TIM3

    // PB0 - PB15 vystupy push-pull 2 MHz
    GPIOB->CRL &= 0x00000000;
    GPIOB->CRL |= 0x22222222;
    GPIOB->CRH &= 0x00000000;
    GPIOB->CRH |= 0x22222222;

    // Nastaveni casovace
    TIM3->PSC = 7999;               // delicka: 8 MHz / (7999+1) = 1 kHz
    TIM3->ARR = 999;                // restart po 1000 tikach = 1 s
    TIM3->EGR = TIM_EGR_UG;         // okamzite nacteni PSC a ARR
    TIM3->SR &= ~TIM_SR_UIF;        // smazani priznaku po UG
    TIM3->CR1 |= TIM_CR1_CEN;       // spusteni casovace

    for (;;)
    {
        if (TIM3->SR & TIM_SR_UIF)      // doslo k UEV (uplynula 1 s)?
        {
            TIM3->SR &= ~TIM_SR_UIF;    // smazani priznaku UIF
            GPIOB->ODR ^= 0xFFFF;       // zmena stavu LED
        }
    }
}
