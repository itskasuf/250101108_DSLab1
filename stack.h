#ifndef QS_STACK_H
#define QS_STACK_H
#include "order.h"

/* Node exactly as in the brief */
struct Action {
    char op;                 /* 'A' = ADD.  On the redo stack, the top action of a
                                multi-undo batch carries op = '2'..'9' (batch size)
                                and the rest of the batch carry op = 'b'.         */
    Order data;
    struct Action *next;
};

typedef struct {
    struct Action *head;
    int depth;
} ActStack;

void st_reset(ActStack *s);
int  st_push(ActStack *s, char op, Order data);
int  st_pop(ActStack *s, char *op, Order *data);
void st_wipe(ActStack *s);
void st_dump(const char *label, const ActStack *s);

/* Cart is a linked list in the order items were added */
struct CartItem {
    Order o;
    struct CartItem *next;
};

typedef struct {
    struct CartItem *items;
    ActStack undo, redo;
    int counter;
    int keep_redo_on_add;      /* 0 = correct behaviour, 1 = broken (demo only) */
} Till;

void till_open(Till *t);
void till_close(Till *t);
void till_exec(Till *t, const char *cmd);   /* ADD x q | UNDO | REDO | SEND | UNDO3 */
void till_show(const Till *t);
extern int till_echo;

#endif
