/*
 * controller.hpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#ifndef CONTROLLER_CONTROLLER_HPP_
#define CONTROLLER_CONTROLLER_HPP_

#include <variant>

#include <hal/hal_led.hpp>
#include <hal/hal_button.hpp>

#include <middlewares/active_object.hpp>

namespace controller_events
{

struct command_request
{
    char data[64];
    size_t data_size;
};

struct button_state_changed
{
    bool state;
};

using incoming = std::variant
<
    command_request,
    button_state_changed
>;

}

class controller : public middlewares::active_object<controller_events::incoming>
{
public:
    controller();
    ~controller();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const controller_events::command_request &e);
    void event_handler(const controller_events::button_state_changed &e);

    hal::leds::debug led;
    hal::buttons::blue_btn button;
    osTimerId_t button_timer;
};


#endif /* CONTROLLER_CONTROLLER_HPP_ */
