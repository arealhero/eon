#if !EON_PLATFORM_TIME_INCLUDED
#    error Do not use this file directly. Include "<eon/platform/time.h>" instead.
#endif

#include <eon/conversions.h>

#include <time.h> // NOTE(vlad): For 'struct timespec' and 'clock_gettime'.

internal Timestamp
platform_get_current_monotonic_timestamp(void)
{
    struct timespec spec;
    ASSERT(clock_gettime(CLOCK_MONOTONIC, &spec) != -1);
    return SECONDS_TO_MICROSECONDS(spec.tv_sec) + NANOSECONDS_TO_MICROSECONDS(spec.tv_nsec);
}
