/* Edge-case tests (Task 4c).  make test */
#include <stdio.h>
#include <stdlib.h>
#include "order.h"
#include "queue.h"
#include "stack.h"
#include "sim.h"

static int run = 0, passed = 0;

#define CHECK(desc, expect, cond, ...)                                       \
    do {                                                                     \
        char got[48];                                                        \
        snprintf(got, sizeof got, __VA_ARGS__);                              \
        int ok_ = (cond);                                                    \
        run++; passed += ok_;                                                \
        printf("T%-2d %-36s | expect: %-27s | got: %-27s | %s\n",            \
               run, desc, expect, got, ok_ ? "ok" : "FAILED");               \
    } while (0)

static Ticket tk(int num, char kind, int at, int dur)
{
    Ticket t = {num, kind, at, dur};
    return t;
}

int main(void)
{
    puts("Registration No. 250101108 -> d1=1, d2=1, d3=0, d4=8");
    puts("Edge-case tests for QuickServe\n");
    till_echo = 0;

    {   Rail r; Line l; Ticket x;
        rail_create(&r, 5); line_init(&l);
        int a = rail_pop(&r, &x), b = line_pop(&l, &x);
        CHECK("Dequeue from both empty queues", "rail 0, line 0", a == 0 && b == 0, "rail %d, line %d", a, b);
        rail_destroy(&r); }

    {   Rail r; int K = 5, accepted = 0;
        rail_create(&r, K);
        for (int i = 1; i <= K + 1; i++) accepted += rail_push(&r, tk(i, 'R', 0, 1));
        CHECK("Full rail: push K+1 tickets (K=5)", "5 accepted, full=1",
              accepted == 5 && rail_full(&r), "%d accepted, full=%d", accepted, rail_full(&r));
        rail_destroy(&r); }

    {   Rail r; Ticket x; int out = 0;
        rail_create(&r, 2);
        rail_push(&r, tk(1,'R',0,1)); rail_push(&r, tk(2,'R',0,1));
        rail_pop(&r, &x); rail_push(&r, tk(3,'R',0,1));
        rail_pop(&r, &x); out = out * 10 + x.num;
        rail_pop(&r, &x); out = out * 10 + x.num;
        CHECK("Rail index wraps past the end", "comes out 2 then 3", out == 23, "comes out %d then %d", out / 10, out % 10);
        rail_destroy(&r); }

    {   Ticket s[STREAM_LEN]; int dg[4] = {1, 1, 0, 8};
        Served a[STREAM_LEN], b[STREAM_LEN]; Summary sa, sb;
        build_stream(dg, s, 0);
        for (int k = 0; k < STREAM_LEN; k++) s[k].kind = 'E';
        run_two_queue(s, STREAM_LEN, 10, DEADLINE_RULE, a, &sa);
        run_fifo(s, STREAM_LEN, 5, b, &sb);
        CHECK("Every order express", "same as FIFO (14.60)",
              sa.mean_all == sb.mean_all, "%.2f vs FIFO %.2f", sa.mean_all, sb.mean_all); }

    {   /* regular arrives at 0, chef busy until 10, express arrives at 10 = 0 + W */
        Ticket s[3] = {tk(1,'R',0,10), tk(2,'R',0,2), tk(3,'E',10,2)};
        Served g[3]; Summary sm;
        run_two_queue(s, 3, 10, DEADLINE_RULE, g, &sm);
        CHECK("Regular reaches exactly W=10", "R2 starts 10, waits 10",
              g[1].start == 10 && g[1].wait == 10, "R2 starts %d, waits %d", g[1].start, g[1].wait); }

    {   /* same, but the express arrives one minute earlier: it may go first */
        Ticket s[3] = {tk(1,'R',0,10), tk(2,'R',0,2), tk(3,'E',9,2)};
        Served g[3]; Summary sm;
        run_two_queue(s, 3, 10, DEADLINE_RULE, g, &sm);
        CHECK("Express arrives before R's deadline", "E3 starts 10, R2 at 12",
              g[2].start == 10 && g[1].start == 12, "E3 starts %d, R2 at %d", g[2].start, g[1].start); }

    {   Ticket s[2] = {tk(1,'R',5,3), tk(2,'E',20,1)};
        Served g[2]; Summary sm;
        run_two_queue(s, 2, 10, DEADLINE_RULE, g, &sm);
        CHECK("Idle kitchen (gaps between orders)", "starts 5 and 20, waits 0",
              g[0].start == 5 && g[1].start == 20 && sm.mean_all == 0,
              "starts %d and %d, waits %.0f", g[0].start, g[1].start, sm.mean_all); }

    {   Till t; till_open(&t);
        printf("     program prints:"); fflush(stdout);
        till_exec(&t, "UNDO");
        CHECK("UNDO on an empty stack", "nothing changes", !t.items && !t.undo.depth && !t.redo.depth,
              "cart %s, undo %d, redo %d", t.items ? "full" : "empty", t.undo.depth, t.redo.depth);
        till_close(&t); }

    {   Till t; till_open(&t);
        till_exec(&t, "ADD Burger 1"); till_exec(&t, "UNDO"); till_exec(&t, "SEND");
        printf("     program prints:"); fflush(stdout);
        till_exec(&t, "REDO");
        CHECK("REDO right after SEND", "nothing to redo", !t.items && !t.redo.depth,
              "cart %s, redo %d", t.items ? "full" : "empty", t.redo.depth);
        till_close(&t); }

    {   Till t; till_open(&t);
        till_exec(&t, "ADD Fries 1"); till_exec(&t, "ADD Wrap 1");
        till_exec(&t, "UNDO3");
        int r1 = t.redo.depth;
        till_exec(&t, "REDO");
        CHECK("UNDO3 with only 2 actions", "2 undone, 1 REDO restores 2",
              r1 == 2 && t.undo.depth == 2 && t.redo.depth == 0,
              "%d undone, undo now %d", r1, t.undo.depth);
        till_close(&t); }

    {   g_fail_next_alloc = 1;
        Order *o = create_order(1, "Burger", 'R', 1, 13.5);
        CHECK("malloc returns NULL in create_order", "NULL returned, no crash", o == NULL,
              "%s returned", o ? "pointer" : "NULL");
        free(o); }

    {   Line l; line_init(&l); line_push(&l, tk(1,'R',0,1));
        g_fail_next_alloc = 1;
        int r = line_push(&l, tk(2,'R',0,1));
        CHECK("malloc returns NULL in line_push", "rejected, length stays 1", r == 0 && l.len == 1,
              "returned %d, length %d", r, l.len);
        line_clear(&l); }

    {   Rail r;
        g_fail_next_alloc = 1;
        int ok = rail_create(&r, 5);
        CHECK("malloc returns NULL in rail_create", "create fails cleanly", ok == 0 && r.slot == NULL,
              "returned %d", ok); }

    {   Order *one = create_order(1, "Chai", 'R', 1, 2.5);
        double z = total_bill(&one, 0);
        CHECK("total_bill with n = 0", "0.00", z == 0.0, "%.2f", z);
        free(one); }

    printf("\n%d of %d tests passed\n", passed, run);
    return passed == run ? EXIT_SUCCESS : EXIT_FAILURE;
}
