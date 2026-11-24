#include <stdlib.h>
#include "queue.h"
#include "order.h"

/* ---------------- rail (circular array, one slot kept empty) ---------------- */

int rail_create(Rail *r, int K)
{
    r->size = K + 1;
    r->head = r->tail = 0;
    r->slot = xmalloc(sizeof(Ticket) * (size_t)r->size);
    return r->slot != NULL;
}

void rail_destroy(Rail *r)
{
    free(r->slot);
    r->slot = NULL;
}

int rail_empty(const Rail *r) { return r->head == r->tail; }
int rail_full(const Rail *r)  { return (r->tail + 1) % r->size == r->head; }
int rail_length(const Rail *r){ return (r->tail - r->head + r->size) % r->size; }

int rail_push(Rail *r, Ticket t)
{
    if (rail_full(r))
        return 0;
    r->slot[r->tail] = t;
    r->tail = (r->tail + 1) % r->size;
    return 1;
}

int rail_pop(Rail *r, Ticket *t)
{
    if (rail_empty(r))
        return 0;
    *t = r->slot[r->head];
    r->head = (r->head + 1) % r->size;
    return 1;
}

/* ---------------- waiting line (linked list) ---------------- */

void line_init(Line *q)
{
    q->first = q->last = NULL;
    q->len = 0;
}

int line_empty(const Line *q) { return q->len == 0; }

int line_push(Line *q, Ticket t)
{
    struct LineNode *node = xmalloc(sizeof *node);
    if (!node)
        return 0;
    node->t = t;
    node->next = NULL;
    if (line_empty(q))
        q->first = node;
    else
        q->last->next = node;
    q->last = node;
    q->len++;
    return 1;
}

int line_pop(Line *q, Ticket *t)
{
    if (line_empty(q))
        return 0;
    struct LineNode *gone = q->first;
    *t = gone->t;
    q->first = gone->next;
    if (--q->len == 0)
        q->last = NULL;
    free(gone);
    return 1;
}

const Ticket *line_front(const Line *q)
{
    return q->first ? &q->first->t : NULL;
}

void line_clear(Line *q)
{
    Ticket skip;
    while (line_pop(q, &skip))
        ;
}
