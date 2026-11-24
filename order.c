#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "order.h"

int g_fail_next_alloc = 0;

void *xmalloc(size_t bytes)
{
    if (g_fail_next_alloc) {
        g_fail_next_alloc = 0;
        return NULL;
    }
    return malloc(bytes);
}

Order *create_order(int id, const char *item, char type, int qty, double price)
{
    Order *o = xmalloc(sizeof(Order));
    if (!o)
        return NULL;
    memset(o, 0, sizeof *o);
    o->id = id;
    o->type = type;
    o->qty = qty;
    o->price = price;
    if (item)
        snprintf(o->item, sizeof o->item, "%s", item);   /* truncates safely */
    return o;
}

void apply_discount(Order *o, double pct)
{
    if (o)
        o->price = o->price * (1.0 - pct / 100.0);
}

void apply_discount_copy(Order o, double pct)
{
    o.price = o.price * (1.0 - pct / 100.0);
    printf("      inside apply_discount_copy: o.price = %.2f (this is a copy)\n", o.price);
}

double total_bill(Order **orders, int n)
{
    double total = 0.0;
    Order **last = orders + n - 1;
    while (n > 0 && orders <= last) {
        total += (**orders).price * (**orders).qty;
        orders++;
    }
    return total;
}

void show_order(const Order *o)
{
    printf("    [%2d] %-14s %c  %d x %6.2f = %7.2f\n",
           o->id, o->item, o->type, o->qty, o->price, o->qty * o->price);
}
