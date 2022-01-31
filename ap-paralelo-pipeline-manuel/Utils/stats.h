#ifndef PROJETO_PCONC_STATS_H
#define PROJETO_PCONC_STATS_H

#include <time.h>
typedef struct TimeData {
    struct timespec start;
    struct timespec end;
} timer_data;

#define ALLOCATE_TIMERS(size) (timer_data *) malloc(size * sizeof(timer_data))

#endif //PROJETO_PCONC_STATS_H
