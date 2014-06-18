#ifndef THOR_JOB_QUEUE_H
#define THOR_JOB_QUEUE_H
#pragma once

#include "basetypes.h"
#include "thread.h"
#include "list.h"
#include "priority_queue.h"
#include "embedded_hash_multimap.h"
#include "semaphore.h"
#include "mutex.h"

namespace thor
{

///////////////////////////////////////////////////////////////////////////////
namespace policy
{

struct job_id_default
{
    typedef size_type job_id_type;

    job_id_default() : next_job_id_(1) {}

    job_id_type next_job_id() { return next_job_id_++; }

private:
    job_id_type next_job_id_;
};

}

///////////////////////////////////////////////////////////////////////////////
class job : public ref_counted<>
{
    THOR_DECLARE_NOCOPY(job);
public:
    job() {}

    virtual void run() = 0;

protected:
    virtual ~job() {}
};

///////////////////////////////////////////////////////////////////////////////
template
<
    typename T_PRIORITY = size_type,
    class T_JOB_ID_POLICY = policy::job_id_default
>
class job_queue : protected T_JOB_ID_POLICY
{
    typedef job_queue<T_PRIORITY, T_JOB_ID_POLICY> job_queue_type;
public:
    job_queue();
    ~job_queue();

    void start_threads(size_type count, const char* name, thread::thread_priority priority = thread::normal_priority);
    void stop_threads();
    size_type num_threads() const;

    job_id_type add_job(ref_pointer<job> job, T_PRIORITY priority);
    bool reprioritize(job_id_type job_id, T_PRIORITY new_priority);
    bool remove_job(job_id_type job);

private:
    using T_JOB_ID_POLICY::job_id_type;

    ref_pointer<job> wait_for_job();
    void job_finished(ref_pointer<job> job);

    class worker_thread : public thread
    {
    public:
        worker_thread(job_queue_type* queue, const char* name, thread::thread_priority priority)
            : thread(name, priority), queue_(queue) {}

        void execute()
        {
            while (!is_stop_requested())
            {
                ref_pointer<job> job = queue_->wait_for_job();
                if (job)
                {
                    job->run();
                    queue_->job_finished(job);
                }
            }
        }
    private:
        job_queue_type* queue_;
    };

    struct entry
    {
        T_PRIORITY priority;
        ref_pointer<job> job;
        embedded_hash_multimap_link<job_id_type, entry> link_;

        entry(const T_PRIORITY& prio, const ref_pointer<job>& j) : priority(prio), job(j) {}
    };
    struct select_priority
    {
        bool operator () (const entry* rhs, const entry* lhs) const
        {
            return lhs->priority < rhs->priority;
        }
    };
    typedef embedded_hash_multimap<job_id_type, entry, &entry::link_> entries_map;
    entries_map entries_;
    priority_queue<entry*, vector<entry*>, select_priority> priorities_;
    vector<ref_pointer<worker_thread>, 8> threads_;
    mutex mutex_;
    semaphore sem_;
};

///////////////////////////////////////////////////////////////////////////////
template<typename T_PRIORITY, class T_JOB_ID_POLICY> inline job_queue<T_PRIORITY, T_JOB_ID_POLICY>::job_queue()
{}

template<typename T_PRIORITY, class T_JOB_ID_POLICY> inline job_queue<T_PRIORITY, T_JOB_ID_POLICY>::~job_queue()
{
    stop_threads();
    entries_.remove_all();
    while (!priorities_.empty())
    {
        delete priorities_.top();
        priorities_.pop();
    }
}

template<typename T_PRIORITY, class T_JOB_ID_POLICY> inline 
void job_queue<T_PRIORITY, T_JOB_ID_POLICY>::start_threads(size_type count, const char* name, thread::thread_priority priority)
{
    basic_string<char, 64> fullname;
    while (count--)
    {
        fullname.format("%s%u", name, (unsigned)threads_.size());
        threads_.push_back(new worker_thread(this, fullname.c_str(), priority));
    }
}

template<typename T_PRIORITY, class T_JOB_ID_POLICY> inline 
void job_queue<T_PRIORITY, T_JOB_ID_POLICY>::stop_threads()
{
    for (size_type i = 0; i < threads_.size(); ++i)
    {
        threads_[i]->stop(false);
    }
    for (size_type i = 0; i < threads_.size(); ++i)
    {
        threads_[i]->join();
    }
    threads_.clear();
}

template<typename T_PRIORITY, class T_JOB_ID_POLICY> inline 
size_type job_queue<T_PRIORITY, T_JOB_ID_POLICY>::num_threads() const
{
    return threads_.size();
}

template<typename T_PRIORITY, class T_JOB_ID_POLICY> inline 
job_queue<T_PRIORITY, T_JOB_ID_POLICY>::job_id_type job_queue<T_PRIORITY, T_JOB_ID_POLICY>::add_job(ref_pointer<job> job, T_PRIORITY priority)
{
    scope_locker lock(mutex_);   
    job_id_type job_id = next_job_id();
    entry* e = new entry(priority, job);
    entries_.insert(job_id, e);
    priorities_.push(e);
    sem_.release();
    return job_id;
}

template<typename T_PRIORITY, class T_JOB_ID_POLICY> inline 
bool job_queue<T_PRIORITY, T_JOB_ID_POLICY>::reprioritize(job_id_type job_id, T_PRIORITY new_priority)
{
    scope_locker lock(mutex_);
    entries_map::iterator iter((entries_.find(job_id));
    if (iter != entries_.end())
    {
        entry* e = new entry(new_priority, iter->job);
        iter->job = 0;
        *iter = e;
        priorities_.push(e);
        return true;
    }
    return false;
}

template<typename T_PRIORITY, class T_JOB_ID_POLICY> inline 
bool job_queue<T_PRIORITY, T_JOB_ID_POLICY>::remove_job(job_id_type job_id)
{
    scope_locker lock(mutex_);
    entries_map::iterator iter(entries_.find(job_id));
    if (iter != entries_.end())
    {
        iter->job = 0;
        entries_.remove(iter);
        return true;
    }
    return false;
}


template<typename T_PRIORITY, class T_JOB_ID_POLICY> inline 
ref_pointer<job> job_queue<T_PRIORITY, T_JOB_ID_POLICY>::wait_for_job()
{
    sem_.wait();
    scope_locker lock(mutex_);
    if (!priorities_.empty())
    {
        entry* e = priorities_.top();
        if (e->link_.is_contained())
        {
            entries_.remove(e);
        }
        ref_counted<job> job = e->job;
        priorities_.pop();
        lock.unlock();
        delete e;
        return job;
    }
}

template<typename T_PRIORITY, class T_JOB_ID_POLICY> inline 
void job_queue<T_PRIORITY, T_JOB_ID_POLICY>::job_finished(ref_pointer<job> job)
{
    // Do nothing...
}

}


#endif