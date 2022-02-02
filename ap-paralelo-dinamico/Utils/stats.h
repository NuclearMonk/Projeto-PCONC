#ifndef PROJETO_PCONC_STATS_H
#define PROJETO_PCONC_STATS_H

#include <time.h>
typedef struct TimeData
{
    struct timespec start;
    struct timespec end;
} timer_data;

#define ALLOCATE_TIMERS(size) (timer_data *)malloc(size * sizeof(timer_data))

#define LOG_START                         \
    int logsize = 0;                      \
    char logs_aux[512] = "";              \
    char *realoc_holder = NULL;           \
    char *logs = NULL;                    \
    timer_data thread_timer, image_timer; \
    clock_gettime(CLOCK_REALTIME, &(thread_timer.start));

#define LOG_IMAGE_START clock_gettime(CLOCK_REALTIME, &(image_timer.start));
#define LOG_THREAD(OPERATION)                                                                                           \
    clock_gettime(CLOCK_REALTIME, &(thread_timer.end));                                                                 \
    snprintf(logs_aux, 512, "thread %s %d,%ld.%ld,%ld.%ld\n", (OPERATION), targs->thread_id, thread_timer.start.tv_sec, \
             thread_timer.start.tv_nsec, thread_timer.end.tv_sec, thread_timer.end.tv_nsec);                            \
    realoc_holder = logs;                                                                                               \
    logs = realloc(logs, logsize + 512);                                                                                \
    if (logs == NULL)                                                                                                   \
    {                                                                                                                   \
        freeNew(realoc_holder);                                                                                            \
        if (write(targs->ret_pipe[1], &(targs->thread_id), sizeof(int)) != sizeof(int))                                 \
            help(ERR_USING_PIPE, NULL);                                                                                 \
    }                                                                                                                   \
    logs = strncat(logs, logs_aux, 512);

#define LOG_IMAGE(OPERATION)                                                                            \
    clock_gettime(CLOCK_REALTIME, &(image_timer.end));                                                  \
    snprintf(logs_aux, 512, "%s %s,%ld.%ld,%ld.%ld\n", filename, (OPERATION), image_timer.start.tv_sec, \
             image_timer.start.tv_nsec, image_timer.end.tv_sec, image_timer.end.tv_nsec);               \
    if (logs != NULL)                                                                                   \
    {                                                                                                   \
        realoc_holder = logs;                                                                           \
        logs = realloc(logs, logsize + 512);                                                            \
    }                                                                                                   \
    else                                                                                                \
    {                                                                                                   \
        logs = calloc(512, sizeof(char));                                                               \
    }                                                                                                   \
    if (logs == NULL)                                                                                   \
    {                                                                                                   \
        freeNew(realoc_holder);                                                                            \
        if (write(targs->ret_pipe[1], &(targs->thread_id), sizeof(int)) != sizeof(int))                 \
            help(ERR_USING_PIPE, NULL);                                                                 \
    }                                                                                                   \
    logsize += 512;                                                                                     \
    logs = strncat(logs, logs_aux, 512);

#endif //PROJETO_PCONC_STATS_H
