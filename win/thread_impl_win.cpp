/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * win/thread_impl_win.cpp
 *
 * ** THOR INTERNAL FILE - NOT FOR APPLICATION USE **
 *
 * Implementation for Windows threads
 */

#include "../thread.h"

#include "../debug.h"

#define WIN32_EXTRA_LEAN 1
#include <Windows.h>

#include <process.h>

namespace thor
{

///////////////////////////////////////////////////////////////////////////////
// thread_id
///////////////////////////////////////////////////////////////////////////////
thread_id::thread_id(const internal::thread_id_base& rhs)
    : internal::thread_id_base(rhs)
{
    THOR_COMPILETIME_ASSERT(sizeof(DWORD) == sizeof(internal::thread_id_base::thread_id_), InvalidSizeAssumption);
}

thread_id thread_id::get_current_thread()
{
    return thread_id(::GetCurrentThreadId());
}

///////////////////////////////////////////////////////////////////////////////
// internal::thread_base
///////////////////////////////////////////////////////////////////////////////
namespace internal
{

thread_base::thread_base()
    : handle_((void*)INVALID_HANDLE_VALUE)
{
    THOR_COMPILETIME_ASSERT(sizeof(handle_) == sizeof(HANDLE), InvalidHandleSizeAssumption);
}

thread_base::~thread_base()
{
    if (handle_ != (void*)INVALID_HANDLE_VALUE)
    {
        ::CloseHandle((HANDLE)handle_);
    }
}

unsigned __stdcall thread_base::start_thread_proc(void* param)
{
    thread_base* p = reinterpret_cast<thread_base*>(param);
    p->begin_run();
    return 0;
}

}

///////////////////////////////////////////////////////////////////////////////
// thread
///////////////////////////////////////////////////////////////////////////////
thread::thread(const char* name, thread_priority priority, size_type stack_size)
    : name_(name)
    , priority_(priority)
    , state_(state_idle)
    , stack_size_(stack_size)
    , stop_requested_(0)
    , suspend_count_(0)
{
}

thread::~thread()
{
}

bool thread::start(bool start_suspended /*=false*/)
{
    if (handle_ == (void*)INVALID_HANDLE_VALUE)
    {
        add_ref();
        stop_requested_ = 0;
        THOR_ASSERT(stack_size_ <= UINT_MAX);
        handle_ = (void*)_beginthreadex(0, (unsigned int)stack_size_, &internal::thread_base::start_thread_proc, static_cast<internal::thread_base*>(this), start_suspended ? CREATE_SUSPENDED : 0, &thread_id_.thread_id_);
        if (handle_ != (void*)INVALID_HANDLE_VALUE)
        {
            debug::set_thread_name(name_.c_str(), thread_id_);
            if (start_suspended) ++suspend_count_;
            return true;
        }
    }
    return false;
}

bool thread::stop(bool auto_join)
{
    if (handle_ != (void*)INVALID_HANDLE_VALUE)
    {
        state_ = state_stopping;
        ++stop_requested_;
        if (auto_join)
        {
            join();
        }
        return true;
    }
    return false;
}

bool thread::join(size_type timeout_ms)
{
    DWORD ms;
    if (timeout_ms == size_type(-1))
    {
        ms = INFINITE;
    }
    else
    {
        ms = (DWORD)timeout_ms;
    }

    return ::WaitForSingleObject((HANDLE)handle_, ms) == WAIT_OBJECT_0;
}

void thread::set_thread_priority(thread_priority priority)
{
    static int priority_convert[] = {
        THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_HIGHEST,
    };
    THOR_COMPILETIME_ASSERT(priority_count == (sizeof(priority_convert)/sizeof(priority_convert[0])), InvalidAssumption);
    ::SetThreadPriority((HANDLE)handle_, priority_convert[priority]);
}

void thread::suspend()
{
    THOR_DEBUG_ASSERT(handle_ != (void*)INVALID_HANDLE_VALUE);
    if (handle_ != (void*)INVALID_HANDLE_VALUE)
    {
        ++suspend_count_;
        ::SuspendThread((HANDLE)handle_);
    }
}

void thread::resume()
{
    THOR_DEBUG_ASSERT(suspend_count_ > 0);
    THOR_DEBUG_ASSERT(handle_ != (void*)INVALID_HANDLE_VALUE);
    if (suspend_count_ > 0 && handle_ != (void*)INVALID_HANDLE_VALUE)
    {
        ::ResumeThread((HANDLE)handle_);
        --suspend_count_;
    }
}

void thread::begin_run()
{
    THOR_DEBUG_ASSERT(state_ == state_idle);
    state_ = state_running;
    execute();
    state_ = state_finished;
    release();
}

}