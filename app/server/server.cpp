/*
 * server.cpp
 *
 *  Created on: 10 mar 2024
 *      Author: kwarc
 */

#include "server.hpp"

#include "app/config.hpp"

#include <cstdio>
#include <cassert>

namespace events = server_events;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

}

void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{
    if (eNetworkEvent == eNetworkUp)
    {
        static const server::event e { events::network_up {}, server::event::flags::immutable };
        server::active_object::instance->send(e);
    }
    else if (eNetworkEvent == eNetworkDown)
    {
        static const server::event e { events::network_down {}, server::event::flags::immutable };
        server::active_object::instance->send(e);
    }
}

const char* pcApplicationHostnameHook(void)
{
    return "udp-server";
}

eDHCPCallbackAnswer_t xApplicationDHCPHook(eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress)
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

BaseType_t xApplicationGetRandomNumber(uint32_t *pulNumber)
{
    static uint32_t rand = 2345;
    *pulNumber = rand++;
    return 1;
}

uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress, uint16_t usSourcePort, uint32_t ulDestinationAddress, uint16_t usDestinationPort)
{
    static uint32_t num = 65982591;
    return num++;
}

//-----------------------------------------------------------------------------
/* private */

void server::dispatch(const event& e)
{
    std::visit([this](auto &&e) { this->event_handler(e); }, e.data);
}

void server::event_handler(const events::network_up &e)
{
    printf("Connected to network\n");
    printf("Starting UDP server...\n");

    /* Open the UDP socket */
    this->listening_socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP);
    assert(this->listening_socket != FREERTOS_INVALID_SOCKET);

    /* Set the socket send timeout */
    const uint32_t socket_send_timeout = 1000;
    FreeRTOS_setsockopt(this->listening_socket, 0, FREERTOS_SO_SNDTIMEO, &socket_send_timeout, sizeof(socket_send_timeout));

    /* Bind to port 7 */
    bind_addr.sin_port = FreeRTOS_htons(7);
    const bool err = FreeRTOS_bind(this->listening_socket, &this->bind_addr, sizeof(this->bind_addr)) != 0;

    /* Create receiving thread */
    osThreadAttr_t receive_thread_attr {0};
    receive_thread_attr.name = "server_rcv";
    receive_thread_attr.priority = osPriorityNormal;
    receive_thread_attr.stack_size = 2048;

    this->receive_thread = osThreadNew(server::receive_thread_loop, this, &receive_thread_attr);
    assert(this->receive_thread != nullptr);

    printf("UDP server %s\n", err ? "start error" : "started");
}

void server::event_handler(const events::network_down &e)
{
    printf("Disconnected from network\n");

    osStatus_t status = osThreadTerminate(this->receive_thread);
    assert(status == osOK);
    this->receive_thread = nullptr;
}

void server::receive_thread_loop(void *arg)
{
    server *this_ptr = static_cast<server*>(arg);

    uint8_t received_data[128];
    uint32_t client_len = sizeof(this_ptr->client_addr);

    while (true)
    {
        int32_t bytes_received = FreeRTOS_recvfrom(this_ptr->listening_socket,
                                                   received_data,
                                                   sizeof(received_data),
                                                   0,
                                                   &this_ptr->client_addr,
                                                   &client_len);

        if (bytes_received > 0)
        {
            /* Echo received data */
            FreeRTOS_sendto(this_ptr->listening_socket,
                            received_data,
                            bytes_received,
                            0,
                            &this_ptr->client_addr,
                            client_len);

            /* TODO: Send event with received data to server object? */
        }
    }
}

//-----------------------------------------------------------------------------
/* public */

server::server() : active_object("server", osPriorityNormal, 2048),
receive_thread {nullptr}, listening_socket {nullptr}, client_addr {0}, bind_addr {0}
{
    FreeRTOS_IPInit(config::ip_addr, config::net_mask, config::gateway_addr, config::dns_addr, config::mac_addr);
}

server::~server()
{

}


