#pragma once

#include <time.h> // clock_gettime

inline double get_monotonic_time()
{
  struct timespec spec;
  clock_gettime(CLOCK_MONOTONIC, &spec);

  auto s  = spec.tv_sec;
  long ms = spec.tv_nsec / 1000000;
  if (ms > 999) {
    s++;
    ms = 0;
  }

  static long init = s;

  return (s-init) + ms / 1000.0;
}

