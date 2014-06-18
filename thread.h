/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * thread.h
 *
 * This file defines a platform-inspecific thread wrapper
 */

#ifndef THOR_THREAD_H
#define THOR_THREAD_H
#pragma once

#ifndef THOR_REF_COUNTED_H
#include "ref_counted.h"
#endif

#ifndef THOR_BASIC_STRING_H
#include "basic_string.h"
#endif

#if defined(_WIN32)
#include "win/thread_base_win.inl"
#else
#error Unsupported platform!
#endif

namespace thor
{

class thread_id : public internal::thread_id_base
{
    friend class thread;
    thread_id(const internal::thread_id_base& rhs);
public:
    bool is_current_thread() const          { return get_current_thread() == *this; }
    static thread_id get_current_thread();
};


class thread : public ref_counted<policy::thread_safe_ref_count, policy::delete_this_on_zero_refcount, 0>,
               public internal::thread_base
{
public:
    enum thread_priority
    {
        very_low_priority,
        low_priority,
        normal_priority,
        high_priority,
        very_high_priority,

        priority_count
    };

    enum thread_state
    {
        state_idle,
        state_running,
        state_stopping,
        state_finished,

        state_count
    };
    
    thread(const char* thread_name = 0, thread_priority priority = normal_priority, size_type stack_size = 0);

    bool start(bool start_suspended = false);
    
    bool stop(bool join = false);
    bool join(size_type timeout_ms = size_type(-1));
    bool wait(size_type timeout_ms = size_type(-1)) { return join(timeout_ms); }

    thread_id get_thread_id() const;

    bool is_stop_requested() const;
    thread_state get_thread_state() const;

    thread_priority get_thread_priority() const;
    void set_thread_priority(thread_priority priority);

    void suspend();
    void resume();
    bool is_suspended() const;

protected:
    virtual ~thread();

    virtual void begin_run();
    virtual void execute() = 0;

private:
    basic_string<char, 64> name_;
    atomic_integer<thread_priority> priority_;
    thread_state    state_;
    size_type       stack_size_;
    atomic_integer<size_type> suspend_count_;
    atomic_integer<size_type> stop_requested_;
    internal::thread_id_base thread_id_;
};

inline thread_id thread::get_thread_id() const
{
    return thread_id(thread_id_);
}

inline bool thread::is_stop_requested() const
{
    return stop_requested_ > 0;
}

inline thread::thread_state thread::get_thread_state() const
{
    return state_;
}

inline thread::thread_priority thread::get_thread_priority() const
{
    return priority_;
}

inline bool thread::is_suspended() const
{
    return suspend_count_ != 0;
}

}

#if defined(_WIN32)
// #include "win/thread_impl.h"
#else
#error Unsupported platform!
#endif

#endif