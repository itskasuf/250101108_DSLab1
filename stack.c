#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

/* ------------------------- linked-list stack ------------------------- */

void st_reset(ActStack *s) { s->head = NULL; s->depth = 0; }

int st_push(ActStack *s, char op, Order data)
{
    struct Action *node = xmalloc(sizeof *node);
    if (!node)
        return 0;
    node->op = op;
    node->data = data;
    node->next = s->head;
    s->head = node;
    s->depth++;
    return 1;
}

int st_pop(ActStack *s, char *op, Order *data)
{
    struct Action *node = s->head;
    if (!node)
        return 0;
    *op = node->op;
    *data = node->data;
    s->head = node->next;
    s->depth--;
    free(node);
    return 1;
}

void st_wipe(ActStack *s)
{
    char op;
    Order skip;
    while (st_pop(s, &op, &skip))
        ;
}

void st_dump(const char *label, const ActStack *s)
{
    printf("      %-4s| ", label);
    if (!s->head) { puts("(empty)"); return; }
    for (const struct Action *a = s->head; a; a = a->next) {
        int batch_top = a->op >= '2' && a->op <= '9';
        int in_batch = batch_top || a->op == 'b';
        if (batch_top) printf("<batch of %c: ", a->op);
        printf("%s %d", a->data.item, a->data.qty);
        if (in_batch && (!a->next || a->next->op != 'b')) printf(">");
        if (a->next) printf(" | ");
    }
    printf("   (top on the left)\n");
}

/* ------------------------------ till ------------------------------ */

static double unit_price(const char *name)
{
    static const struct { const char *n; double p; } menu[] = {
        {"Burger", 13.50}, {"Juice", 7.00}, {"Fries", 6.00}, {"Wrap", 11.00},
    };
    for (size_t k = 0; k < sizeof menu / sizeof menu[0]; k++)
        if (!strcmp(menu[k].n, name)) return menu[k].p;
    return 9.00;
}

void till_open(Till *t)
{
    t->items = NULL;
    st_reset(&t->undo);
    st_reset(&t->redo);
    t->counter = 0;
    t->keep_redo_on_add = 0;
}

static void cart_append(Till *t, Order o)
{
    struct CartItem *c = xmalloc(sizeof *c), **pp = &t->items;
    if (!c) { puts("      !! no memory for cart item"); return; }
    c->o = o;
    c->next = NULL;
    while (*pp) pp = &(*pp)->next;
    *pp = c;
}

static void cart_drop(Till *t, int id)
{
    for (struct CartItem **pp = &t->items; *pp; pp = &(*pp)->next)
        if ((*pp)->o.id == id) {
            struct CartItem *gone = *pp;
            *pp = gone->next;
            free(gone);
            return;
        }
}

static void cart_empty_out(Till *t)
{
    while (t->items) cart_drop(t, t->items->o.id);
}

void till_close(Till *t)
{
    cart_empty_out(t);
    st_wipe(&t->undo);
    st_wipe(&t->redo);
}

static void do_add(Till *t, const char *name, int qty)
{
    Order o;
    memset(&o, 0, sizeof o);
    o.id = ++t->counter;
    o.type = 'R';
    o.qty = qty;
    o.price = unit_price(name);
    snprintf(o.item, sizeof o.item, "%s", name);
    if (!st_push(&t->undo, 'A', o)) { puts("      !! no memory, ADD rejected"); return; }
    cart_append(t, o);
    if (!t->keep_redo_on_add)
        st_wipe(&t->redo);
}

static void do_undo(Till *t)
{
    char op; Order o;
    if (!st_pop(&t->undo, &op, &o)) { puts("      -- UNDO ignored: nothing to undo"); return; }
    cart_drop(t, o.id);
    st_push(&t->redo, 'A', o);
}

/* Multi-undo: undo up to 3 actions. They go onto redo as one batch:
   lower members tagged 'b', the top member tagged with the batch size. */
static void do_undo3(Till *t)
{
    char op; Order o;
    int k = 0;
    while (k < 3 && st_pop(&t->undo, &op, &o)) {
        cart_drop(t, o.id);
        st_push(&t->redo, 'b', o);
        k++;
    }
    if (k == 0) { puts("      -- UNDO3 ignored: nothing to undo"); return; }
    if (k > 1) t->redo.head->op = (char)('0' + k);   /* label the batch */
    else       t->redo.head->op = 'A';              /* a batch of one is a normal undo */
}

static void do_redo(Till *t)
{
    char op; Order o;
    if (!st_pop(&t->redo, &op, &o)) { puts("      -- REDO ignored: nothing to redo"); return; }
    int count = (op >= '2' && op <= '9') ? op - '0' : 1;
    for (;;) {
        cart_append(t, o);
        st_push(&t->undo, 'A', o);
        if (--count == 0 || !st_pop(&t->redo, &op, &o))
            break;
    }
}

static void do_send(Till *t)
{
    double bill = 0;
    printf("      ** kitchen ticket: ");
    if (!t->items) printf("(nothing)");
    for (struct CartItem *c = t->items; c; c = c->next) {
        printf("%s x%d%s", c->o.item, c->o.qty, c->next ? " + " : "");
        bill += c->o.qty * c->o.price;
    }
    printf("   total %.2f AED\n", bill);
    cart_empty_out(t);
    st_wipe(&t->undo);
    st_wipe(&t->redo);
}

void till_show(const Till *t)
{
    printf("      cart| ");
    if (!t->items) printf("(empty)");
    for (const struct CartItem *c = t->items; c; c = c->next)
        printf("%s %d%s", c->o.item, c->o.qty, c->next ? ", " : "");
    putchar('\n');
    st_dump("undo", &t->undo);
    st_dump("redo", &t->redo);
}

int till_echo = 1;   /* 0 = print only warnings (used by the tests) */

void till_exec(Till *t, const char *cmd)
{
    char name[24];
    int qty;
    if (till_echo) printf("  $ %s\n", cmd);
    if (sscanf(cmd, "ADD %23s %d", name, &qty) == 2) do_add(t, name, qty);
    else if (!strcmp(cmd, "UNDO"))  do_undo(t);
    else if (!strcmp(cmd, "UNDO3")) do_undo3(t);
    else if (!strcmp(cmd, "REDO"))  do_redo(t);
    else if (!strcmp(cmd, "SEND"))  do_send(t);
    else puts("      -- unknown command");
    if (till_echo) till_show(t);
}
