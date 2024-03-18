/*
 * server.cpp
 *
 *  Created on: 10 mar 2024
 *      Author: kwarc
 */

#include "server.hpp"

#include "app/config.hpp"
#include "app/controller/controller.hpp"

#include "hal/hal_random.hpp"

#include <cstdio>
#include <cassert>

//-----------------------------------------------------------------------------
/* helpers */

void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{
    if (eNetworkEvent == eNetworkUp)
    {
        static server::static_event e { server::network_up {} };
        server::instance->send(e);
    }
    else if (eNetworkEvent == eNetworkDown)
    {
        static server::static_event e { server::network_down {} };
        server::instance->send(e);
    }
}

const char* pcApplicationHostnameHook(void)
{
    return "udp-server";
}

eDHCPCallbackAnswer_t xApplicationDHCPHook(eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress)
{
    if (eDHCPPhase == eDHCPPhasePreRequest)
    {
        static server::static_event e { server::ip_addr_assigned { ulIPAddress } };
        server::instance->send(e);
    }

    return eDHCPContinue;
}

BaseType_t xApplicationGetRandomNumber(uint32_t *pulNumber)
{
    *pulNumber = hal::random::get();
    return pdTRUE;
}

uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress, uint16_t usSourcePort, uint32_t ulDestinationAddress, uint16_t usDestinationPort)
{
    return  hal::random::get();
}

static BaseType_t socket_udp_receive_callback(Socket_t socket, void * data, size_t length, const struct freertos_sockaddr * from, const struct freertos_sockaddr * dest)
{
    static server::static_event e { server::udp_data_received {} };
    server::instance->send(e);
    return 0;
}

static void socket_udp_sent_callback(Socket_t socket, size_t length)
{

}

//-----------------------------------------------------------------------------
/* private */

void server::event_handler(const network_up &e)
{
    printf("Connected to network\n");
    printf("Starting UDP server...\n");

    /* Open the UDP socket */
    this->listening_socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP);
    assert(this->listening_socket != FREERTOS_INVALID_SOCKET);

    /* Set UDP callbacks */
    F_TCP_UDP_Handler_t callbacks { nullptr, nullptr, nullptr, socket_udp_receive_callback, socket_udp_sent_callback };
    FreeRTOS_setsockopt(this->listening_socket, 0, FREERTOS_SO_UDP_RECV_HANDLER, &callbacks, sizeof(callbacks));
    FreeRTOS_setsockopt(this->listening_socket, 0, FREERTOS_SO_UDP_SENT_HANDLER, &callbacks, sizeof(callbacks));

    /* Set the socket send timeout */
    const uint32_t socket_send_timeout = 1000;
    FreeRTOS_setsockopt(this->listening_socket, 0, FREERTOS_SO_SNDTIMEO, &socket_send_timeout, sizeof(socket_send_timeout));

    /* Bind to port 7 */
    this->bind_addr.sin_port = FreeRTOS_htons(7);
    const bool err = FreeRTOS_bind(this->listening_socket, &this->bind_addr, sizeof(this->bind_addr)) != 0;

    printf("UDP server %s\n", err ? "start error" : "started");
}

void server::event_handler(const network_down &e)
{
    printf("Disconnected from network\n");
}

void server::event_handler(const ip_addr_assigned &e)
{
    char buf[16] {};
    FreeRTOS_inet_ntoa(e.address, buf);
    printf("IP address: %s\n", buf);
}

void server::event_handler(const udp_data_received &e)
{
    uint32_t client_len = sizeof(this->client_addr);
    controller::command_request cmd_req;

    const int32_t result = FreeRTOS_recvfrom(this->listening_socket,
                                             cmd_req.data,
                                             sizeof(cmd_req.data),
                                             FREERTOS_MSG_DONTWAIT,
                                             &this->client_addr,
                                             &client_len);

    if (result > 0)
    {
        cmd_req.data_size = result;
        cmd_req.data[cmd_req.data_size] = 0;
        controller::instance->send(cmd_req);
    }
    else
    {
        printf("Server error: 'recvfrom' failed\n");
    }
}

void server::event_handler(const command_response &e)
{
    const int32_t result = FreeRTOS_sendto(this->listening_socket,
                                           e.data,
                                           e.data_size,
                                           0,
                                           &this->client_addr,
                                           sizeof(this->client_addr));

    if (result != static_cast<int32_t>(e.data_size) || result < 0)
        printf("Server error: 'sendto' failed\n");
}

//-----------------------------------------------------------------------------
/* public */

server::server() : active_object("server", osPriorityNormal, 2048),
listening_socket {nullptr}, client_addr {0}, bind_addr {0}
{
    hal::random::enable(true);
    FreeRTOS_IPInit(config::ip_addr, config::net_mask, config::gateway_addr, config::dns_addr, config::mac_addr);

#if (ipconfigUSE_DHCP == 0)
    this->send(ip_addr_assigned { FreeRTOS_GetIPAddress() });
#endif
}

server::~server()
{

}


