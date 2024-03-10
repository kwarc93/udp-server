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

static void udp_server( void *pvParameters )
{
long lBytes;
uint8_t cReceivedString[ 128 ];
struct freertos_sockaddr xClient, xBindAddress;
uint32_t xClientLength = sizeof( xClient );
Socket_t xListeningSocket;

   /* Attempt to open the socket. */
   xListeningSocket = FreeRTOS_socket( FREERTOS_AF_INET,
                                       FREERTOS_SOCK_DGRAM, /*FREERTOS_SOCK_DGRAM for UDP.*/
                                       FREERTOS_IPPROTO_UDP );

   /* Check the socket was created. */
   configASSERT( xListeningSocket != FREERTOS_INVALID_SOCKET );

   /* Bind to port 7. */
   xBindAddress.sin_port = FreeRTOS_htons( 7 );
   FreeRTOS_bind( xListeningSocket, &xBindAddress, sizeof( xBindAddress ) );

   for( ;; )
   {
       /* Receive data from the socket.  ulFlags is zero, so the standard
          interface is used.  By default the block time is portMAX_DELAY, but it
          can be changed using FreeRTOS_setsockopt(). */
       lBytes = FreeRTOS_recvfrom( xListeningSocket,
                                   cReceivedString,
                                   sizeof( cReceivedString ),
                                   0,
                                   &xClient,
                                   &xClientLength );

       if( lBytes > 0 )
       {
           /* Data was received and can be process here. */
           FreeRTOS_sendto( xListeningSocket,
                            cReceivedString,
                            lBytes,
                            0,
                            &xClient,
                            xClientLength );
       }
   }
}

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
static BaseType_t xTasksAlreadyCreated = pdFALSE;

    /* Both eNetworkUp and eNetworkDown events can be processed here. */
    if( eNetworkEvent == eNetworkUp )
    {
        /* Create the tasks that use the TCP/IP stack if they have not already
        been created. */
        if( xTasksAlreadyCreated == pdFALSE )
        {
            /*
             * For convenience, tasks that use FreeRTOS-Plus-TCP can be created here
             * to ensure they are not created before the network is usable.
             */
            printf("Starting UDP server\n");

            osThreadAttr_t attr = { 0 };
            attr.name = "udp_server";
            attr.priority = osPriorityNormal;
            attr.stack_size = 2048;
            osThreadNew(udp_server, NULL, &attr);

            xTasksAlreadyCreated = pdTRUE;
        }
    }
}

void init_thread(void *arg)
{
    /* Initialise the RTOS's IP stack.  The tasks that use the network
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

    printf("System started\n");

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
    return "stm32f746";
}

eDHCPCallbackAnswer_t xApplicationDHCPHook (eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress)
{
    eDHCPCallbackAnswer_t eReturn;
    switch (eDHCPPhase)
    {
        case eDHCPPhasePreDiscover:
            eReturn = eDHCPContinue;
            break;

        case eDHCPPhasePreRequest:
            char addr[16];
            FreeRTOS_inet_ntoa (ulIPAddress, addr);
            printf ("IP address from DHCP: %s\n", addr);
            eReturn = eDHCPContinue;
            break;

        default:
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

