#ifndef LIST_SORT_H
#define LIST_SORT_H

// from <linux/compiler.h>
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

struct list_head;

typedef int (*list_cmp_func_t)(const struct list_head *,
                               const struct list_head *);

void list_sort(struct list_head *head, list_cmp_func_t cmp);
#endif