/*
 * hal_interface.hpp
 *
 *  Created on: 21 paź 2020
 *      Author: kwarc
 */

#ifndef HAL_INTERFACE_HPP_
#define HAL_INTERFACE_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>

namespace hal::interface
{
    template<typename T>
    class io_bus
    {
    public:
        typedef std::function<void(const T *data, std::size_t bytes_read)> read_cb_t;
        typedef std::function<void(std::size_t bytes_written)> write_cb_t;

        virtual ~io_bus() {};
        virtual T read(void) = 0;
        virtual void write(T byte) = 0;
        virtual std::size_t read(T *data, std::size_t size) = 0;
        virtual std::size_t write(const T *data, std::size_t size) = 0;
        virtual void read(T *data, std::size_t size, const read_cb_t &callback) = 0;
        virtual void write(const T *data, std::size_t size, const write_cb_t &callback) = 0;
    };

    class serial : public io_bus<std::byte>
    {
    public:
        virtual ~serial() {};
        virtual void listen(bool value) { this->listening = value; };
    protected:
        bool listening;
    };

    class led
    {
    public:
        virtual ~led() {};
        virtual void set(uint8_t brightness) = 0;
        virtual uint8_t get(void) = 0;
    };

    class button
    {
    public:
        virtual ~button() {};
        virtual bool is_pressed(void) = 0;
    };

    class temperature_sensor
    {
    public:
        virtual ~temperature_sensor() {};
        virtual float read_temperature(void) = 0;
    };

}

#endif /* HAL_INTERFACE_HPP_ */
