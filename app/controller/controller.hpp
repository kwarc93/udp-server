/*
 * controller.hpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#ifndef CONTROLLER_CONTROLLER_HPP_
#define CONTROLLER_CONTROLLER_HPP_

#include <functional>

#include <hal/hal_led.hpp>
#include <hal/hal_button.hpp>

#include <middlewares/active_object.hpp>

class controller : public middlewares::active_object<controller>
{
    friend middlewares::active_object<controller>;
public:
    /* Events */
    struct command_request { char data[64]; size_t data_size; std::function<void(const char *data, size_t data_size)>response; };
    struct button_state_changed { bool state; };

    controller();
    ~controller();

private:
    /* Event handlers */
    void event_handler(const command_request &e);
    void event_handler(const button_state_changed &e);

    hal::leds::debug led;
    hal::buttons::blue_btn button;
    osTimerId_t button_timer;
};


#endif /* CONTROLLER_CONTROLLER_HPP_ */
