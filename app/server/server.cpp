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

namespace events = server_events;

//-----------------------------------------------------------------------------
/* helpers */

void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{
    if (eNetworkEvent == eNetworkUp)
    {
        static const server::event e { events::network_up {}, server::event::flags::immutable };
        server::instance->send(e);
    }
    else if (eNetworkEvent == eNetworkDown)
    {
        static const server::event e { events::network_down {}, server::event::flags::immutable };
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
        static server::event e { events::ip_addr_assigned { ulIPAddress }, server::event::flags::immutable };
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

    this->receive_thread = osThreadNew(this->receive_thread_loop, this, &receive_thread_attr);
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

void server::event_handler(const events::ip_addr_assigned &e)
{
    char buf[16] {};
    FreeRTOS_inet_ntoa(e.address, buf);
    printf("IP address: %s\n", buf);
}

void server::event_handler(const events::command_response &e)
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

void server::receive_thread_loop(void *arg)
{
    auto *this_ptr = static_cast<server*>(arg);

    uint32_t client_len = sizeof(this_ptr->client_addr);

    controller_events::command_request cmd_req;

    while (true)
    {
        const int32_t result = FreeRTOS_recvfrom(this_ptr->listening_socket,
                                                 cmd_req.data,
                                                 sizeof(cmd_req.data),
                                                 0,
                                                 &this_ptr->client_addr,
                                                 &client_len);

        if (result > 0)
        {
            cmd_req.data_size = result;
            cmd_req.data[cmd_req.data_size] = 0;
            controller::instance->send({ cmd_req });
        }
        else
        {
            printf("Server error: 'recvfrom' failed\n");
        }
    }
}

//-----------------------------------------------------------------------------
/* public */

server::server() : active_object("server", osPriorityNormal, 2048),
receive_thread {nullptr}, listening_socket {nullptr}, client_addr {0}, bind_addr {0}
{
    hal::random::enable(true);
    FreeRTOS_IPInit(config::ip_addr, config::net_mask, config::gateway_addr, config::dns_addr, config::mac_addr);

#if (ipconfigUSE_DHCP == 0)
    this->send({ events::ip_addr_assigned { FreeRTOS_GetIPAddress() } });
#endif
}

server::~server()
{

}


