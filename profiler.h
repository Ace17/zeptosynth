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
    static double maxMs = 0;
    auto durationMs = timespec_diff(t1, t2);

    if(durationMs > maxMs)
    {
      maxMs = durationMs;
      fprintf(stderr, "Max duration: %.3f ms\n", maxMs);
    }

    if(0)
    {
      fprintf(stderr, "% 4.0f |", durationMs);

      for(int i=0; i < int(durationMs);++i)
        fprintf(stderr, "*");
      fprintf(stderr, "\n");
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

