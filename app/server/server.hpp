/*
 * server.hpp
 *
 *  Created on: 10 mar 2024
 *      Author: kwarc
 */

#ifndef SERVER_SERVER_HPP_
#define SERVER_SERVER_HPP_

#include <variant>

#include <middlewares/active_object.hpp>

#include "FreeRTOS_IP.h"

namespace server_events
{

struct network_up
{

};

struct network_down
{

};

struct command_response
{
    char data[64];
    size_t data_size;
};

using incoming = std::variant
<
    network_up,
    network_down,
    command_response
>;

}

class server : public middlewares::active_object<server_events::incoming>
{
public:
    server();
    ~server();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const server_events::network_up &e);
    void event_handler(const server_events::network_down &e);
    void event_handler(const server_events::command_response &e);

    static void receive_thread_loop(void *arg);

    osThreadId_t receive_thread;
    Socket_t listening_socket;
    struct freertos_sockaddr client_addr, bind_addr;
};

#endif /* SERVER_SERVER_HPP_ */
