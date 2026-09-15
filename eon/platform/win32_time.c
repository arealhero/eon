#if !EON_PLATFORM_TIME_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/time.h>" instead.
#endif

#include <eon/conversions.h>
#include <eon/assert.h>

#include "win32_declarations.h"

internal Timestamp
platform_get_current_monotonic_timestamp(void)
{
    LARGE_INTEGER ticks;
    ASSERT(QueryPerformanceCounter(&ticks) != 0);

    LARGE_INTEGER ticks_per_second;
    ASSERT(QueryPerformanceFrequency(&ticks_per_second) != 0);

    return SECONDS_TO_MICROSECONDS(ticks.QuadPart) / ticks_per_second.QuadPart;
}
