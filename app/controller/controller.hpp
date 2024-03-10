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

struct led_toggle
{

};

struct button_state_changed
{
    bool state;
};

using incoming = std::variant
<
    button_state_changed,
    led_toggle
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
    void event_handler(const controller_events::led_toggle &e);
    void event_handler(const controller_events::button_state_changed &e);

    int error_code;

    hal::buttons::blue_btn button;
    osTimerId_t button_timer;

    hal::leds::debug led;
    osTimerId_t led_timer;
};


#endif /* CONTROLLER_CONTROLLER_HPP_ */
