# QuickServe (CSS 2111 – Lab Assignment 1)

**Registration number:** 250101108  ·  d1 = 1, d2 = 1, d3 = 0, d4 = 8
**Parameters:** N = 13 orders · rail K = 5 · max regular wait W = 10 min

## Layout
```
order.[ch]   Order struct, OrderCompact, create/discount/total_bill, xmalloc hook
queue.[ch]   Rail  (circular array, one slot always left empty)
             Line  (linked list with first/last pointers)
stack.[ch]   ActStack of struct Action + the Till (ADD, UNDO, REDO, UNDO3, SEND)
sim.[ch]     order-stream builder, event-driven FIFO kitchen, two-queue kitchen
main.c       runs Parts 1–4 (./qs or ./qs <part>)
tests.c      14 edge-case tests (./qs_tests)
outputs/     captured program output, test results, Valgrind and ASan logs
```

## Commands
| | |
|---|---|
| `make` | build `qs` and `qs_tests` |
| `./qs` / `./qs 3` | run everything / only Part 3 |
| `make test` | edge-case tests |
| `make memcheck` | Valgrind on both programs |
| `make sanitize` | AddressSanitizer + UBSan build and run |
| `make clean` | remove build files |
