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

struct ProfileScope
{
  ProfileScope()
    : t1(get_monotonic_time())
  {
  }

  ~ProfileScope()
  {
    const double t2 = get_monotonic_time();
    static double maxDuration = -1;
    const auto duration = t2 - t1;

    if(duration > maxDuration)
    {
      maxDuration = duration;
      fprintf(stderr, "Max duration: %.1f ms\n", maxDuration*1000.0);
    }
  }

  const double t1;
};

