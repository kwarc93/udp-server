/*
 * active_object.hpp
 *
 *  Created on: 2 sty 2023
 *      Author: kwarc
 */

#ifndef ACTIVE_OBJECT_HPP_
#define ACTIVE_OBJECT_HPP_

#include "cmsis_os2.h"

#include <string>
#include <cassert>

namespace middlewares
{

template <typename active>
class active_object
{
    struct base_event
    {
        base_event(bool dynamic) : dynamic {dynamic} {}
        virtual ~base_event() {}
        virtual void dispatch(active *obj) = 0;
        const bool dynamic;
    };

public:
    template <typename event>
    struct dynamic_event : public base_event
    {
        dynamic_event(event &&e) : base_event(true), evt(std::move(e)) { }
        void dispatch(active *obj) { obj->event_handler(evt); }
        event evt;
    };

    template <typename event>
    struct static_event : public base_event
    {
        static_event(event &&e) : base_event(false), evt(std::move(e)) { }
        void dispatch(active *obj) { obj->event_handler(evt); }
        event evt;
    };

    active_object(const std::string_view &name, osPriority_t priority, size_t stack_size, uint32_t queue_size = 32)
    {
        /* It is assumed that each active object is unique */
        assert(this->instance == nullptr);
        this->instance = this;

        /* Create queue of events */
        this->queue_attr.name = name.data();

        this->queue = osMessageQueueNew(queue_size, sizeof(base_event*), &this->queue_attr);
        assert(this->queue != nullptr);

        /* Create worker thread */
        this->thread_attr.name = name.data();
        this->thread_attr.priority = priority;
        this->thread_attr.stack_size = stack_size;

        this->thread = osThreadNew(active_object::thread_loop, this, &this->thread_attr);
        assert(this->thread != nullptr);
    }

    virtual ~active_object()
    {
        osStatus_t status;
        status = osMessageQueueDelete(this->queue);
        assert(status == osOK);
        this->queue = nullptr;

        status = osThreadTerminate(this->thread);
        assert(status == osOK);
        this->thread = nullptr;

        this->instance = nullptr;
    }

    template <typename event>
    void send(const static_event<event> &evt, uint32_t timeout = osWaitForever)
    {
        auto *msg = &evt;

        assert(msg != nullptr);
        assert(osMessageQueuePut(this->queue, &msg, 0, timeout) == osOK);
    }

    template <typename event>
    void send(const dynamic_event<event> &evt, uint32_t timeout = osWaitForever)
    {
        auto *msg = new(std::nothrow) dynamic_event<event>(std::move(evt));

        assert(msg != nullptr);
        assert(osMessageQueuePut(this->queue, &msg, 0, timeout) == osOK);
    }

    template <typename event>
    void send(event evt, uint32_t timeout = osWaitForever)
    {
        auto *msg = new(std::nothrow) dynamic_event<event>(std::move(evt));

        assert(msg != nullptr);
        assert(osMessageQueuePut(this->queue, &msg, 0, timeout) == osOK);
    }

    /* Used for global access (e.g. from interrupt) */
    static inline active_object *instance;
private:
    static void thread_loop(void *arg)
    {
        auto *this_ = static_cast<active_object*>(arg);

        while (true)
        {
            base_event *msg = nullptr;
            uint8_t msg_prio = 0;

            if (osMessageQueueGet(this_->queue, &msg, &msg_prio, osWaitForever) == osOK)
            {
                msg->dispatch(static_cast<active*>(this_));
                if (msg->dynamic)
                    delete msg;
            }
        }
    }

    osMessageQueueId_t queue;
    osMessageQueueAttr_t queue_attr { 0 };
    osThreadId_t thread;
    osThreadAttr_t thread_attr { 0 };
};

}

#endif /* ACTIVE_OBJECT_HPP_ */
