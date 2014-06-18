/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * thread_base_win.inl
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * This file defines the thread base for windows systems
 */

#ifndef THOR_BASETYPES_H
#include "../basetypes.h"
#endif

namespace thor
{

namespace internal
{

struct thread_id_base
{
    thread_id_base(uint32 thread_id = 0) : thread_id_(thread_id) {}
    bool operator == (const thread_id_base& rhs) const { return rhs.thread_id_ == thread_id_; }
    bool operator != (const thread_id_base& rhs) const { return rhs.thread_id_ != thread_id_; }
    bool operator <  (const thread_id_base& rhs) const { return rhs.thread_id_ <  thread_id_; }
    thread_id_base& operator = (const thread_id_base& rhs) { thread_id_ = rhs.thread_id_; return *this; }
    uint32 thread_id_;
};

struct thread_base
{
    void* handle_;

    thread_base();
    ~thread_base();

    static unsigned __stdcall start_thread_proc(void* param);
    virtual void begin_run() = 0;
};

}

}