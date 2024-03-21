/*
 * main.cpp
 *
 *  Created on: 10 mar 2024
 *      Author: kwarc
 */

#include <memory>
#include <cassert>
#include <cstdio>

#include <hal/hal_system.hpp>

#include "cmsis_os2.h"

#include "drivers/stm32f7/rcc.hpp"
#include "drivers/stm32f7/gpio.hpp"

#include "app/controller/controller.hpp"
#include "app/server/server.hpp"

void init_thread(void *arg)
{
    /* Create active objects */
    auto ctrl = controller();
    auto srv = server(ctrl);

    osThreadSuspend(osThreadGetId());
}

int main(void)
{
    hal::system::init();

    printf("System started\n");

    osKernelInitialize();
    osThreadNew(init_thread, NULL, NULL);
    if (osKernelGetState() == osKernelReady)
        osKernelStart();

    assert(!"OS kernel start error");

    while (1);

    return 0;
}

extern "C" void HAL_ETH_MspInit(void)
{
    using namespace drivers;

    rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, ETHMAC), true);
    rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, ETHMACTX), true);
    rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, ETHMACRX), true);

    gpio::configure({gpio::port::portg, gpio::pin::pin14}, gpio::mode::af, gpio::af::af11);
    gpio::configure({gpio::port::portg, gpio::pin::pin13}, gpio::mode::af, gpio::af::af11);
    gpio::configure({gpio::port::portg, gpio::pin::pin11}, gpio::mode::af, gpio::af::af11);
    gpio::configure({gpio::port::portc, gpio::pin::pin1}, gpio::mode::af, gpio::af::af11);
    gpio::configure({gpio::port::porta, gpio::pin::pin1}, gpio::mode::af, gpio::af::af11);
    gpio::configure({gpio::port::portc, gpio::pin::pin4}, gpio::mode::af, gpio::af::af11);
    gpio::configure({gpio::port::porta, gpio::pin::pin2}, gpio::mode::af, gpio::af::af11);
    gpio::configure({gpio::port::portc, gpio::pin::pin5}, gpio::mode::af, gpio::af::af11);
    gpio::configure({gpio::port::porta, gpio::pin::pin7}, gpio::mode::af, gpio::af::af11);

    NVIC_ClearPendingIRQ(ETH_IRQn);
    NVIC_SetPriority(ETH_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(ETH_IRQn);
}

