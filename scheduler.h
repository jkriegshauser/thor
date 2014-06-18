/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * scheduler.h
 *
 * Scheduler implementation for real-time callbacks
 */

#ifndef THOR_SCHEDULER_H
#define THOR_SCHEUDLER_H
#pragma once

#include "basetypes.h"

namespace thor
{

class scheduler
{
    THOR_DECLARE_NOCOPY(scheduler);
public:
    scheduler();
    ~scheduler();
};

}

#endif