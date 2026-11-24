/* Event-driven kitchen: the clock jumps straight to the next interesting
   moment (a finish time or the next arrival) instead of ticking minutes. */
#include <stdio.h>
#include "sim.h"

void build_stream(const int d[4], Ticket s[STREAM_LEN], int express_every_third)
{
    for (int i = 1; i <= STREAM_LEN; i++) {
        int j = i % 4 + 1;
        s[i - 1].num  = i;
        s[i - 1].at   = 2 * i + d[j - 1] % 3;
        s[i - 1].dur  = 3 + (i + d[1]) % 5;
        s[i - 1].kind = (express_every_third && i % 3 == 0) ? 'E' : 'R';
    }
}

static void summarise(const Served *log, int n, Summary *sum)
{
    int all = 0, ex = 0, re = 0, nex = 0, nre = 0;
    sum->worst_regular = sum->worst_express = 0;
    for (const Served *p = log; p < log + n; p++) {
        all += p->wait;
        if (p->t.kind == 'E') {
            ex += p->wait; nex++;
            if (p->wait > sum->worst_express) sum->worst_express = p->wait;
        } else {
            re += p->wait; nre++;
            if (p->wait > sum->worst_regular) sum->worst_regular = p->wait;
        }
    }
    sum->mean_all     = n   ? (double)all / n   : 0;
    sum->mean_express = nex ? (double)ex  / nex : 0;
    sum->mean_regular = nre ? (double)re  / nre : 0;
}

static void cook(Served *log, Ticket t, int now)
{
    Served *row = &log[t.num - 1];
    row->t = t;
    row->start = now;
    row->finish = now + t.dur;
    row->wait = now - t.at;
}

int run_fifo(const Ticket *s, int n, int K, Served *log, Summary *sum)
{
    Rail rail;
    Line line;
    if (!rail_create(&rail, K))
        return 0;
    line_init(&line);
    sum->peak_rail = sum->peak_line = 0;

    int clock = 0, next = 0, served = 0, ok = 1;
    while (served < n) {
        /* everyone who has arrived by now gets a ticket on the rail or in the line */
        while (next < n && s[next].at <= clock) {
            int placed = line_empty(&line) && rail_push(&rail, s[next]);
            if (!placed && !line_push(&line, s[next])) { ok = 0; goto done; }
            next++;
            if (rail_length(&rail) > sum->peak_rail) sum->peak_rail = rail_length(&rail);
            if (line.len > sum->peak_line)           sum->peak_line = line.len;
        }
        if (rail_empty(&rail)) {          /* chef idle: jump to the next arrival */
            clock = s[next].at;
            continue;
        }
        Ticket t, moved;
        rail_pop(&rail, &t);
        cook(log, t, clock);
        if (line_pop(&line, &moved))      /* freed slot is filled from the line */
            rail_push(&rail, moved);
        clock += t.dur;                   /* jump to the finish time */
        served++;
    }
    summarise(log, n, sum);
done:
    rail_destroy(&rail);
    line_clear(&line);
    return ok;
}

/* DEADLINE_RULE (main design, earliest deadline first):
     every order gets a "serve-by" time:
        express : its arrival time         (it wants the chef right away)
        regular : its arrival time + W     (it may be kept waiting up to W)
     Both queues are FIFO, so each queue's head already has that queue's earliest
     deadline -> comparing the two heads is enough. Ties go to the regular order.
   LATE_AGING (alternative for comparison): express first, but the regular head
     is promoted once it has already waited >= W. */
int pick_next(Line *express, Line *regular, int now, int W, Policy p, Ticket *out)
{
    const Ticket *e = line_front(express);
    const Ticket *r = line_front(regular);

    if (!e) return line_pop(regular, out);
    if (!r) return line_pop(express, out);

    int regular_first;
    if (p == DEADLINE_RULE)
        regular_first = r->at + W <= e->at;
    else
        regular_first = now - r->at >= W;

    return regular_first ? line_pop(regular, out) : line_pop(express, out);
}

int run_two_queue(const Ticket *s, int n, int W, Policy p, Served *log, Summary *sum)
{
    Line ex, re;
    line_init(&ex);
    line_init(&re);
    sum->peak_rail = sum->peak_line = 0;

    int clock = 0, next = 0, served = 0, ok = 1;
    while (served < n) {
        while (next < n && s[next].at <= clock) {
            if (!line_push(s[next].kind == 'E' ? &ex : &re, s[next])) { ok = 0; goto done; }
            next++;
        }
        Ticket t;
        if (!pick_next(&ex, &re, clock, W, p, &t)) {
            clock = s[next].at;
            continue;
        }
        cook(log, t, clock);
        clock += t.dur;
        served++;
    }
    summarise(log, n, sum);
done:
    line_clear(&ex);
    line_clear(&re);
    return ok;
}

void print_log(const Served *log, int n)
{
    puts("    i  kind  arrive  cook  start  finish  wait");
    puts("   ---------------------------------------------");
    for (const Served *p = log; p < log + n; p++)
        printf("   %2d    %c    %4d   %4d   %4d    %4d  %4d\n",
               p->t.num, p->t.kind, p->t.at, p->t.dur, p->start, p->finish, p->wait);
    puts("   ---------------------------------------------");
}
