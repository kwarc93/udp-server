/*
 * FreeRTOS+TCP V3.1.0
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

#ifndef FREERTOS_IP_CONFIG_H
#define FREERTOS_IP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Define the byte order of the target MCU (the MCU FreeRTOS+TCP is executing
on).  Valid options are pdFREERTOS_BIG_ENDIAN and pdFREERTOS_LITTLE_ENDIAN. */
#define ipconfigBYTE_ORDER pdFREERTOS_LITTLE_ENDIAN

#define ipconfigMAC_INTERRUPT_PRIORITY ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY )
#define ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM  ( 1 )
#define ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM  ( 1 )

#define ipconfigZERO_COPY_RX_DRIVER   ( 1 )
#define ipconfigZERO_COPY_TX_DRIVER   ( 1 )

#define ipconfigUSE_LINKED_RX_MESSAGES                ( 1 )
#define ipconfigSUPPORT_NETWORK_DOWN_EVENT            ( 1 )

#define ipconfigETHERNET_MINIMUM_PACKET_BYTES  ( 60 )
#define ipconfigUSE_DHCP_HOOK    ( 1 )

/* Include support for DNS caching.  For TCP, having a small DNS cache is very
useful.  When a cache is present, ipconfigDNS_REQUEST_ATTEMPTS can be kept low
and also DNS may use small timeouts.  If a DNS reply comes in after the DNS
socket has been destroyed, the result will be stored into the cache.  The next
call to FreeRTOS_gethostbyname() will return immediately, without even creating
a socket. */
#define ipconfigUSE_DNS_CACHE    ( 1 )

/* The IP stack executes it its own task (although any application task can make
use of its services through the published sockets API). ipconfigIP_TASK_PRIORITY
sets the priority of the task that executes the IP stack.  The priority is a
standard FreeRTOS task priority so can take any value from 0 (the lowest
priority) to (configMAX_PRIORITIES - 1) (the highest priority).
configMAX_PRIORITIES is a standard FreeRTOS configuration parameter defined in
FreeRTOSConfig.h, not FreeRTOSIPConfig.h. Consideration needs to be given as to
the priority assigned to the task executing the IP stack relative to the
priority assigned to tasks that use the IP stack. */

#define  niEMAC_HANDLER_TASK_PRIORITY  (configMAX_PRIORITIES - 1)
#define  ipconfigIP_TASK_PRIORITY   (configMAX_PRIORITIES - 2)
#define  ipconfigIPERF_PRIORITY_IPERF_TASK (configMAX_PRIORITIES - 3)

/* The size, in words (not bytes), of the stack allocated to the FreeRTOS+TCP
task.  This setting is less important when the FreeRTOS Win32 simulator is used
as the Win32 simulator only stores a fixed amount of information on the task
stack.  FreeRTOS includes optional stack overflow detection, see:
http://www.freertos.org/Stacks-and-stack-overflow-checking.html */
#define ipconfigIP_TASK_STACK_SIZE_WORDS ( configMINIMAL_STACK_SIZE * 2U )


/* If ipconfigUSE_NETWORK_EVENT_HOOK is set to 1 then FreeRTOS+TCP will call the
network event hook at the appropriate times.  If ipconfigUSE_NETWORK_EVENT_HOOK
is not set to 1 then the network event hook will never be called.  See
http://www.FreeRTOS.org/FreeRTOS-Plus/FreeRTOS_Plus_UDP/API/vApplicationIPNetworkEventHook.shtml
*/
#define ipconfigUSE_NETWORK_EVENT_HOOK 1

#define ipconfigUSE_CALLBACKS 1

/* If ipconfigUSE_DHCP is 1 then FreeRTOS+TCP will attempt to retrieve an IP
address, netmask, DNS server address and gateway address from a DHCP server.  If
ipconfigUSE_DHCP is 0 then FreeRTOS+TCP will use a static IP address.  The
stack will revert to using the static IP address even when ipconfigUSE_DHCP is
set to 1 if a valid configuration cannot be obtained from a DHCP server for any
reason.  The static configuration used is that passed into the stack by the
FreeRTOS_IPInit() function call. */
#define ipconfigUSE_DHCP    1
#define ipconfigDHCP_REGISTER_HOSTNAME 1

/* ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS defines the total number of network buffer that
are available to the IP stack.  The total number of network buffers is limited
to ensure the total amount of RAM that can be consumed by the IP stack is capped
to a pre-determinable value. */

#define ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS  ( 20 )

/* USE_TCP: Use TCP and all its features */
#define ipconfigUSE_TCP    ( 1 )

/* USE_WIN: Let TCP use windowing mechanism. */
#define ipconfigUSE_TCP_WIN   ( 1 )

/* The MTU is the maximum number of bytes the payload of a network frame can
contain.  For normal Ethernet V2 frames the maximum MTU is 1500.  Setting a
lower value can save RAM, depending on the buffer management scheme used.  If
ipconfigCAN_FRAGMENT_OUTGOING_PACKETS is 1 then (ipconfigNETWORK_MTU - 28) must
be divisible by 8. */

#define ipconfigNETWORK_MTU     ( 1500 )

/* Set ipconfigUSE_DNS to 1 to include a basic DNS client/resolver.  DNS is used
through the FreeRTOS_gethostbyname() API function. */
#define ipconfigUSE_DNS   1

/* If ipconfigSUPPORT_SELECT_FUNCTION is set to 1 then the FreeRTOS_select()
(and associated) API function is available. */
#define ipconfigSUPPORT_SELECT_FUNCTION    1

/* If ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is set to 1 then it is the
responsibility of the Ethernet interface to filter out packets that are of no
interest.  If the Ethernet interface does not implement this functionality, then
set ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES to 0 to have the IP stack
perform the filtering instead (it is much less efficient for the stack to do it
because the packet will already have been passed into the stack).  If the
Ethernet driver does all the necessary filtering in hardware then software
filtering can be removed by using a value other than 1 or 0. */
#define ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES 1
#define ipconfigETHERNET_DRIVER_FILTERS_PACKETS 1

/* Define the size of the pool of TCP window descriptors.  On the average, each
TCP socket will use up to 2 x 6 descriptors, meaning that it can have 2 x 6
outstanding packets (for Rx and Tx).  When using up to 10 TP sockets
simultaneously, one could define TCP_WIN_SEG_COUNT as 120. */
#define ipconfigTCP_WIN_SEG_COUNT 64

/* When using call-back handlers, the driver may check if the handler points to
real program memory (RAM or flash) or just has a random non-zero value. */
#define ipconfigIS_VALID_PROG_ADDRESS(x) ( (x) != NULL )

/* UDP Logging related constants follow.  The standard UDP logging facility
writes formatted strings to a buffer, and creates a task that removes messages
from the buffer and sends them to the UDP address and port defined by the
constants that follow. */

/* Prototype for the function used to print out.  In this case the standard
UDP logging facility is used. */
extern int lUDPLoggingPrintf( const char *pcFormatString, ... );

/* Set to 1 to print out debug messages.  If ipconfigHAS_DEBUG_PRINTF is set to
1 then FreeRTOS_debug_printf should be defined to the function used to print
out the debugging messages. */
#ifndef ipconfigHAS_DEBUG_PRINTF
 #define ipconfigHAS_DEBUG_PRINTF 0
#endif

#if( ipconfigHAS_DEBUG_PRINTF == 1 )
 #define FreeRTOS_debug_printf( X ) ( void ) lUDPLoggingPrintf X
#endif

/* Set to 1 to print out non debugging messages, for example the output of the
FreeRTOS_netstat() command, and ping replies.  If ipconfigHAS_PRINTF is set to 1
then FreeRTOS_printf should be set to the function used to print out the
messages. */
#ifndef ipconfigHAS_PRINTF
 #define ipconfigHAS_PRINTF   0
#endif

#if( ipconfigHAS_PRINTF == 1 )
 #define FreeRTOS_printf(X)   ( void ) lUDPLoggingPrintf X
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FREERTOS_IP_CONFIG_H */
