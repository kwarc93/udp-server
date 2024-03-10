/*
 * main.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <memory>
#include <cassert>
#include <cstdio>

#include <hal/hal_system.hpp>

#include "cmsis_os2.h"
#include "FreeRTOS_IP.h"

#include "drivers/stm32f7/rcc.hpp"
#include "drivers/stm32f7/gpio.hpp"
#include "app/controller/controller.hpp"

/* The MAC address array is not declared const as the MAC address will
normally be read from an EEPROM and not hard coded (in real deployed
applications).*/
static uint8_t ucMACAddress[ 6 ] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };

/* Define the network addressing.  These parameters will be used if either
ipconfigUDE_DHCP is 0 or if ipconfigUSE_DHCP is 1 but DHCP auto configuration
failed. */
static const uint8_t ucIPAddress[ 4 ] = { 192, 168, 1, 30 };
static const uint8_t ucNetMask[ 4 ] = { 255, 255, 255, 0 };
static const uint8_t ucGatewayAddress[ 4 ] = { 192, 168, 1, 1 };

/* The following is the address of an OpenDNS server. */
static const uint8_t ucDNSServerAddress[ 4 ] = { 208, 67, 222, 222 };


void init_thread(void *arg)
{
    /* Initialise the RTOS's TCP/IP stack.  The tasks that use the network
    are created in the vApplicationIPNetworkEventHook() hook function
    below.  The hook function is called when the network connects. */
    FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);

    /* Create active objects */

    auto ctrl = std::make_unique<controller>();

    osThreadSuspend(osThreadGetId());
}

int main(void)
{
    hal::system::init();

    printf("System started\r\n");

    osKernelInitialize();
    osThreadNew(init_thread, NULL, NULL);
    if (osKernelGetState() == osKernelReady)
        osKernelStart();

    assert(!"OS kernel start error");

    while (1);

    return 0;
}

const char * pcApplicationHostnameHook( void )
{
    return "udp-server";
}

eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase,
                                            uint32_t ulIPAddress )
{
eDHCPCallbackAnswer_t eReturn;
  /* This hook is called in a couple of places during the DHCP process, as
  identified by the eDHCPPhase parameter. */
  switch( eDHCPPhase )
  {
    case eDHCPPhasePreDiscover  :
      /* A DHCP discovery is about to be sent out.  eDHCPContinue is
      returned to allow the discovery to go out.

      If eDHCPUseDefaults had been returned instead then the DHCP process
      would be stopped and the statically configured IP address would be
      used.

      If eDHCPStopNoChanges had been returned instead then the DHCP
      process would be stopped and whatever the current network
      configuration was would continue to be used. */
      eReturn = eDHCPContinue;
      break;

    case eDHCPPhasePreRequest  :
      /* An offer has been received from the DHCP server, and the offered
      IP address is passed in the ulIPAddress parameter. */
      char addr[16];
      FreeRTOS_inet_ntoa(ulIPAddress, addr);
      printf("[DHCP] IP: %s\r\n", addr);
      eReturn = eDHCPContinue;
      break;

    default :
      /* Cannot be reached, but set eReturn to prevent compiler warnings
      where compilers are disposed to generating one. */
      eReturn = eDHCPContinue;
      break;
  }

  return eReturn;
}
BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    static uint32_t rand = 2345;
    *pulNumber = rand++;
    return 1;
}

uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress, uint16_t usSourcePort, uint32_t ulDestinationAddress, uint16_t usDestinationPort )
{
    static uint32_t num = 65982591;
    return num++;
}

extern "C" void HAL_ETH_MspInit(void)
{
    using namespace drivers;

    rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, ETHMAC), true);
    rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, ETHMACTX), true);
    rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, ETHMACRX), true);

    /*
    ETH GPIO Configuration
    PG14     ------> ETH_TXD1
    PG13     ------> ETH_TXD0
    PG11     ------> ETH_TX_EN
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PC4     ------> ETH_RXD0
    PA2     ------> ETH_MDIO
    PC5     ------> ETH_RXD1
    PA7     ------> ETH_CRS_DV
    */

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

