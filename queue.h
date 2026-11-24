#ifndef QS_QUEUE_H
#define QS_QUEUE_H

typedef struct {
    int  num;     /* order number i      */
    char kind;    /* 'R' or 'E'           */
    int  at;      /* arrival minute       */
    int  dur;     /* cooking time, mins   */
} Ticket;

/* Rail: circular array that always keeps ONE slot empty.
   empty <=> head == tail,  full <=> (tail + 1) % size == head        */
typedef struct {
    Ticket *slot;
    int size;     /* K + 1 slots allocated */
    int head;     /* next ticket to leave  */
    int tail;     /* next free slot        */
} Rail;

int  rail_create(Rail *r, int K);
void rail_destroy(Rail *r);
int  rail_empty(const Rail *r);
int  rail_full(const Rail *r);
int  rail_length(const Rail *r);
int  rail_push(Rail *r, Ticket t);   /* enqueue: 0 if full  */
int  rail_pop(Rail *r, Ticket *t);   /* dequeue: 0 if empty */

/* Waiting line: singly linked list with first and last pointers */
struct LineNode {
    Ticket t;
    struct LineNode *next;
};

typedef struct {
    struct LineNode *first;
    struct LineNode *last;
    int len;
} Line;

void line_init(Line *q);
int  line_empty(const Line *q);
int  line_push(Line *q, Ticket t);   /* 0 if allocation failed */
int  line_pop(Line *q, Ticket *t);   /* 0 if empty */
const Ticket *line_front(const Line *q);
void line_clear(Line *q);

#endif
