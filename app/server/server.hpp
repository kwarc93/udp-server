/*
 * server.hpp
 *
 *  Created on: 10 mar 2024
 *      Author: kwarc
 */

#ifndef SERVER_SERVER_HPP_
#define SERVER_SERVER_HPP_

#include <middlewares/active_object.hpp>

#include "FreeRTOS_IP.h"

class controller;

class server : public middlewares::active_object<server>
{
    friend middlewares::active_object<server>;
public:
    /* Events */
    struct network_up {};
    struct network_down {};
    struct ip_addr_assigned { uint32_t address; };
    struct udp_data_received {};
    struct command_response { char data[64]; size_t data_size; };

    server(controller &c);
    ~server();

private:
    /* Event handlers */
    void event_handler(const network_up &e);
    void event_handler(const network_down &e);
    void event_handler(const ip_addr_assigned &e);
    void event_handler(const udp_data_received &e);
    void event_handler(const command_response &e);

    controller *ctrl;
    Socket_t listening_socket;
    struct freertos_sockaddr client_addr, bind_addr;
};

#endif /* SERVER_SERVER_HPP_ */
