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
        controller::instance->send(controller::button_state_changed { true });
    }
    else if (button->was_released())
    {
        controller::instance->send(controller::button_state_changed { false });
    }
}

}

//-----------------------------------------------------------------------------
/* private */

void controller::event_handler(const command_request &e)
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

    char cmd_rsp[64] = { 0 };
    size_t rsp_size = 0;

    if (cmd == "led")
    {
        if (arg == "on")
            this->led.set(true);
        else if (arg == "off")
            this->led.set(false);
        else if (arg == "get")
            rsp_size = std::snprintf(cmd_rsp, sizeof(cmd_rsp), ">led is %s\n", (this->led.get() ? "on" : "off"));
    }
    else if (cmd == "button")
    {
        if (arg == "get")
            rsp_size = std::snprintf(cmd_rsp, sizeof(cmd_rsp), ">button is %s\n", (this->button.is_pressed() ? "pressed" : "released"));
    }
    else if (cmd == "print")
    {
        puts(arg.c_str());
    }
    else
    {
        rsp_size = std::snprintf(cmd_rsp, sizeof(cmd_rsp), ">unsupported command\n");
    }

    if (rsp_size > 0)
        e.response(cmd_rsp, rsp_size);
}

void controller::event_handler(const button_state_changed &e)
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
