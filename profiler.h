#pragma once

#include <time.h> // clock_gettime

struct ProfileScope
{
  ProfileScope()
  {
    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  }

  ~ProfileScope()
  {
    timespec t2;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
    static double maxSecs = 0;
    auto deltaSecs = timespec_diff(t1, t2);

    if(deltaSecs > maxSecs)
    {
      maxSecs = deltaSecs;
      fprintf(stderr, "Max duration: %.1f ms\n", maxSecs * 1000);
    }
  }

  timespec t1;

  static double timespec_diff(timespec start, timespec stop)
  {
    timespec result;

    if((stop.tv_nsec - start.tv_nsec) < 0)
    {
      result.tv_sec = stop.tv_sec - start.tv_sec - 1;
      result.tv_nsec = stop.tv_nsec - start.tv_nsec + 1000000000;
    }
    else
    {
      result.tv_sec = stop.tv_sec - start.tv_sec;
      result.tv_nsec = stop.tv_nsec - start.tv_nsec;
    }

    return result.tv_nsec / 1000000.0 + result.tv_sec * 1000.0;
  }
};

