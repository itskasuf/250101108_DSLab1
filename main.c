/* QuickServe - CSS 2111 Data Structures Lab, Assignment 1
   build: make        run: ./qs          one part only: ./qs 1|2|3|4 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "order.h"
#include "queue.h"
#include "stack.h"
#include "sim.h"

static const char REGNO[] = "250101108";
static int d[4];                 /* d[0] = d1, ..., d[3] = d4 */

static void banner(int n, const char *what)
{
    printf("\n#############  PART %d : %s  #############\n", n, what);
}

#define SHOW_FIELD(T, f) \
    printf("    %-6s offset %2zu  size %2zu\n", #f, offsetof(T, f), sizeof(((T *)0)->f))

static void part1(void)
{
    banner(1, "structures and pointers");

    puts("\n[1a] layout of Order (as given)");
    SHOW_FIELD(Order, type);
    SHOW_FIELD(Order, price);
    SHOW_FIELD(Order, id);
    SHOW_FIELD(Order, item);
    SHOW_FIELD(Order, qty);
    printf("    sizeof(Order)        = %zu\n", sizeof(Order));
    puts("  layout of OrderCompact (reordered)");
    SHOW_FIELD(OrderCompact, price);
    SHOW_FIELD(OrderCompact, id);
    SHOW_FIELD(OrderCompact, qty);
    SHOW_FIELD(OrderCompact, item);
    SHOW_FIELD(OrderCompact, type);
    printf("    sizeof(OrderCompact) = %zu\n", sizeof(OrderCompact));

    puts("\n[1b] 25% discount on a 16.00 AED Wrap");
    Order *w = create_order(1, "Wrap", 'E', 1, 16.00);
    if (!w) { puts("    allocation failed"); return; }
    printf("    before                      : %.2f\n", w->price);
    apply_discount_copy(*w, 25);
    printf("    after apply_discount_copy() : %.2f  (no effect)\n", w->price);
    apply_discount(w, 25);
    printf("    after apply_discount()      : %.2f  (discount applied)\n", w->price);
    free(w);

    const int N = 5 + d[3];
    printf("\n[1c] N = 5 + d4 = 5 + %d = %d orders\n", d[3], N);
    static const char *names[] = {"Biryani", "Mango Lassi", "Samosa", "Club Sandwich",
        "Chai", "Paneer Roll", "Pasta", "Cold Coffee", "Noodles", "Brownie", "Water",
        "Fruit Bowl", "Pizza", "Nachos", "Muffin"};
    static const double cost[] = {18.0, 8.5, 3.0, 14.0, 2.5, 9.0, 15.0, 10.0, 12.5,
        6.0, 1.5, 7.0, 20.0, 11.0, 5.5};

    Order *orders[N];
    Order **cur = orders;
    for (int k = 0; k < N; k++, cur++) {
        *cur = create_order(k + 1, *(names + k), k % 3 == 0 ? 'E' : 'R', 1 + k % 2, *(cost + k));
        if (*cur == NULL) {
            while (cur > orders) free(*--cur);
            puts("    allocation failed - everything freed");
            return;
        }
    }
    for (cur = orders; cur != orders + N; cur++)
        show_order(*cur);
    printf("    total_bill = %.2f AED\n", total_bill(orders, N));
    for (cur = orders; cur != orders + N; cur++)
        free(*cur);
    printf("    freed %d orders\n", N);
}

static void part2(void)
{
    banner(2, "queues - kitchen dispatch");
    const int K = 3 + d[3] % 3;
    printf("K = 3 + (d4 mod 3) = 3 + (%d mod 3) = %d\n", d[3], K);

    Ticket s[STREAM_LEN];
    build_stream(d, s, 0);
    puts("\nderiving the stream:");
    for (int i = 1; i <= STREAM_LEN; i++) {
        int j = i % 4 + 1;
        printf("  i=%-2d j=%d  t = 2(%d) + (%d mod 3) = %2d   p = 3 + ((%d+%d) mod 5) = %d\n",
               i, j, i, d[j - 1], s[i - 1].at, i, d[1], s[i - 1].dur);
    }

    Served log[STREAM_LEN];
    Summary sum;
    printf("\n[2c] FIFO kitchen, rail capacity K = %d\n", K);
    if (!run_fifo(s, STREAM_LEN, K, log, &sum)) { puts("allocation failed"); return; }
    print_log(log, STREAM_LEN);
    printf("   mean waiting time = %.2f min\n", sum.mean_all);

    puts("\n[2d] same stream, different rail sizes");
    puts("    K    mean wait   most tickets on rail   most tickets in line");
    int sizes[] = {1, K, 10};
    for (int k = 0; k < 3; k++) {
        run_fifo(s, STREAM_LEN, sizes[k], log, &sum);
        printf("   %2d     %6.2f            %2d                    %2d\n",
               sizes[k], sum.mean_all, sum.peak_rail, sum.peak_line);
    }
}

static void part3(void)
{
    banner(3, "stacks - undo / redo");
    char juice[24], wrap[24];
    snprintf(juice, sizeof juice, "ADD Juice %d", d[0] % 3 + 1);
    snprintf(wrap,  sizeof wrap,  "ADD Wrap %d",  d[1] % 3 + 1);
    printf("Juice qty = (d1 mod 3)+1 = %d, Wrap qty = (d2 mod 3)+1 = %d\n",
           d[0] % 3 + 1, d[1] % 3 + 1);

    const char *script[] = {"ADD Burger 2", juice, "ADD Fries 1", "UNDO", "UNDO", "REDO",
                            wrap, "REDO", "UNDO", "SEND", "UNDO"};
    Till t;
    puts("\n[3b] required command sequence");
    till_open(&t);
    for (size_t k = 0; k < sizeof script / sizeof *script; k++)
        till_exec(&t, script[k]);
    till_close(&t);

    puts("\n[3c] broken till: ADD does NOT clear redo");
    const char *bad[] = {"ADD Fries 2", "ADD Juice 1", "UNDO", "UNDO", "ADD Burger 1", "REDO", "REDO"};
    till_open(&t);
    t.keep_redo_on_add = 1;
    for (size_t k = 0; k < sizeof bad / sizeof *bad; k++)
        till_exec(&t, bad[k]);
    till_close(&t);

    puts("\n[3d] one key that undoes the last 3 actions (UNDO3)");
    const char *multi[] = {"ADD Burger 1", "ADD Fries 2", "ADD Juice 1", "ADD Wrap 1",
                           "UNDO3", "REDO", "UNDO", "UNDO3", "SEND"};
    till_open(&t);
    for (size_t k = 0; k < sizeof multi / sizeof *multi; k++)
        till_exec(&t, multi[k]);
    till_close(&t);
}

static void part4(void)
{
    banner(4, "express orders with an aging rule");
    const int W = 10 + d[2];
    printf("W = 10 + d3 = 10 + %d = %d min; orders 3, 6, 9 are express\n", d[2], W);

    Ticket s[STREAM_LEN];
    build_stream(d, s, 1);
    Served fifo[STREAM_LEN], edf[STREAM_LEN], late[STREAM_LEN];
    Summary sf, sl, sa;
    run_fifo(s, STREAM_LEN, STREAM_LEN, fifo, &sf);
    run_two_queue(s, STREAM_LEN, W, DEADLINE_RULE, edf, &sl);
    run_two_queue(s, STREAM_LEN, W, LATE_AGING, late, &sa);

    puts("\n[4b] two queues, deadline rule (my design)");
    print_log(edf, STREAM_LEN);
    puts("\n     plain FIFO on the same stream");
    print_log(fifo, STREAM_LEN);
    puts("\n     alternative: late aging (promote only after waiting >= W)");
    print_log(late, STREAM_LEN);

    puts("\n[4b] comparison (minutes)");
    puts("    policy               express  regular   all    worst regular  worst express");
    printf("    plain FIFO           %6.2f   %6.2f  %6.2f       %3d            %3d\n",
           sf.mean_express, sf.mean_regular, sf.mean_all, sf.worst_regular, sf.worst_express);
    printf("    deadline rule        %6.2f   %6.2f  %6.2f       %3d            %3d\n",
           sl.mean_express, sl.mean_regular, sl.mean_all, sl.worst_regular, sl.worst_express);
    printf("    late aging           %6.2f   %6.2f  %6.2f       %3d            %3d\n",
           sa.mean_express, sa.mean_regular, sa.mean_all, sa.worst_regular, sa.worst_express);
}

int main(int argc, char *argv[])
{
    size_t len = strlen(REGNO);
    for (int k = 0; k < 4; k++)
        d[k] = REGNO[len - 4 + k] - '0';
    printf("Registration No. %s -> d1=%d, d2=%d, d3=%d, d4=%d\n", REGNO, d[0], d[1], d[2], d[3]);

    int only = argc > 1 ? atoi(argv[1]) : 0;
    void (*parts[])(void) = {part1, part2, part3, part4};
    for (int k = 0; k < 4; k++)
        if (only == 0 || only == k + 1)
            parts[k]();
    return 0;
}
