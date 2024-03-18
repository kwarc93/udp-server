/*
 * controller.cpp
 *
 *  Created on: 10 mar 2024
 *      Author: kwarc
 */

#include "controller.hpp"
#include "app/server/server.hpp"

#include <string>
#include <cstdio>

namespace events = controller_events;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

void button_timer_cb(void *arg)
{
    if (arg == nullptr)
        return;

    auto *button = static_cast<hal::button*>(arg);

    button->debounce();

    if (button->was_pressed())
    {
        controller::instance->send(events::button_state_changed { true });
    }
    else if (button->was_released())
    {
        controller::instance->send(events::button_state_changed { false });
    }
}

}

//-----------------------------------------------------------------------------
/* private */

void controller::event_handler(const events::command_request &e)
{
    const std::string cmd_req = e.data;

    size_t delim_pos = cmd_req.find(" ");
    if (delim_pos == cmd_req.npos)
        return;

    const std::string cmd = cmd_req.substr(0, delim_pos);
    size_t end_pos = cmd_req.find("\n");
    if (end_pos == cmd_req.npos)
        return;

    const std::string arg = cmd_req.substr(delim_pos + 1, end_pos - delim_pos - 1);

    server_events::command_response cmd_rsp {};

    if (cmd == "led")
    {
        if (arg == "on")
            this->led.set(true);
        else if (arg == "off")
            this->led.set(false);
        else if (arg == "get")
            cmd_rsp.data_size = std::snprintf(cmd_rsp.data, sizeof(cmd_rsp.data), ">led is %s\n",(this->led.get() ? "on" : "off"));
    }
    else if (cmd == "button")
    {
        if (arg == "get")
            cmd_rsp.data_size = std::snprintf(cmd_rsp.data, sizeof(cmd_rsp.data), ">button is %s\n",(this->button.is_pressed() ? "pressed" : "released"));
    }
    else if (cmd == "print")
    {
        puts(arg.c_str());
    }
    else
    {
        cmd_rsp.data_size = std::snprintf(cmd_rsp.data, sizeof(cmd_rsp.data), ">unsupported command\n");
    }

    if (cmd_rsp.data_size > 0)
        server::instance->send(cmd_rsp);
}

void controller::event_handler(const events::button_state_changed &e)
{
    printf("Button %s\n", e.state ? "pressed" : "released");
}

//-----------------------------------------------------------------------------
/* public */

controller::controller() : active_object("controller", osPriorityNormal, 2048)
{
    /* Create timer for button debouncing */
    this->button_timer = osTimerNew(button_timer_cb, osTimerPeriodic, &this->button, NULL);
    assert(this->button_timer != nullptr);
    osTimerStart(this->button_timer, 20);
}

controller::~controller()
{

}
