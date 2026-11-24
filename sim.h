#ifndef QS_SIM_H
#define QS_SIM_H
#include "queue.h"

#define STREAM_LEN 10

typedef struct {
    Ticket t;
    int start, finish, wait;
} Served;

typedef struct {
    double mean_all, mean_express, mean_regular;
    int worst_regular, worst_express;
    int peak_rail, peak_line;
} Summary;

/* Policies for the two-queue kitchen (Task 4) */
typedef enum { DEADLINE_RULE, LATE_AGING } Policy;

void build_stream(const int d[4], Ticket s[STREAM_LEN], int express_every_third);
int  run_fifo(const Ticket *s, int n, int K, Served *log, Summary *sum);
int  run_two_queue(const Ticket *s, int n, int W, Policy p, Served *log, Summary *sum);
int  pick_next(Line *express, Line *regular, int now, int W, Policy p, Ticket *out);
void print_log(const Served *log, int n);

#endif
