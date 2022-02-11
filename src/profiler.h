#pragma once

#include "monotime.h"

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
      fprintf(stderr, "Max duration: %.1f ms\n", maxDuration * 1000.0);
    }
  }

  const double t1;
};
