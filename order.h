#ifndef QS_ORDER_H
#define QS_ORDER_H
#include <stddef.h>

/* Given structure - must not be changed */
typedef struct {
    char   type;      /* 'R' regular, 'E' express */
    double price;
    int    id;
    char   item[18];
    int    qty;
} Order;

/* Same fields sorted by alignment (8 -> 4 -> 1) */
typedef struct {
    double price;
    int    id;
    int    qty;
    char   item[18];
    char   type;
} OrderCompact;

/* xmalloc: malloc wrapper. Setting g_fail_next_alloc = 1 makes the next
   single call return NULL (used by the tests). */
extern int g_fail_next_alloc;
void *xmalloc(size_t bytes);

Order *create_order(int id, const char *item, char type, int qty, double price);
void   apply_discount(Order *o, double pct);
void   apply_discount_copy(Order o, double pct);
double total_bill(Order **orders, int n);
void   show_order(const Order *o);

#endif
