#ifndef STATS_H
#define STATS_H

#include <time.h>
typedef struct TimeData{
    struct timespec start;
    struct timespec end;
} timer_data;

#define ALLOCATE_TIMERS(N) (timer_data*)malloc(N*sizeof(timer_data));

#endif