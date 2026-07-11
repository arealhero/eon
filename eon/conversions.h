#pragma once

#include <eon/types.h>

#define TO_MILLI(value) ((s64)(value) * 1000)
#define TO_MICRO(value) (TO_MILLI(value) * 1000)
#define TO_NANO(value) (TO_MICRO(value) * 1000)

#define TO_KIBI(value) ((s64)(value) << 10)
#define TO_MEBI(count) ((s64)(count) << 20)
#define TO_GIBI(count) ((s64)(count) << 30)

// TODO(vlad): Support floating-point conversions.

#define MICRO_TO_MILLI(value) ((s64)(value) / 1000)

#define NANO_TO_MICRO(value) ((s64)(value) / 1000)
#define NANO_TO_MILLI(value) (NANO_TO_MICRO(value) / 1000)

// NOTE(vlad): Memory conversions.

#define KiB(count) TO_KIBI(count)
#define MiB(count) TO_MEBI(count)
#define GiB(count) TO_GIBI(count)

// NOTE(vlad): Time conversions.

#define SECONDS_TO_MICROSECONDS(value) TO_MICRO(value)
#define NANOSECONDS_TO_MICROSECONDS(value) NANO_TO_MICRO(value)

// NOTE(vlad): Compile-time tests.

STATIC_ASSERT(KiB(1) == (s64) 1*1024);
STATIC_ASSERT(MiB(2) == (s64) 2*1024*1024);
STATIC_ASSERT(GiB(3) == (s64) 3*1024*1024*1024);

STATIC_ASSERT(TO_MILLI(1) == (s64) 1000);
STATIC_ASSERT(TO_MICRO(2) == (s64) 2*1000*1000);
STATIC_ASSERT(TO_NANO(3) == (s64) 3*1000*1000*1000);

STATIC_ASSERT(MICRO_TO_MILLI(TO_MICRO(1)) == TO_MILLI(1));
STATIC_ASSERT(NANO_TO_MILLI(TO_NANO(1)) == TO_MILLI(1));
