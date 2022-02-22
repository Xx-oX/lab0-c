#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));

    if (!head)
        return NULL;

    INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *entry, *safe;

    list_for_each_entry_safe (entry, safe, l, list)
        q_release_element(entry);

    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *el = malloc(sizeof(element_t));
    if (!el)
        return false;

    el->value = malloc(sizeof(char) * (strlen(s) + 1));
    if (!el->value) {
        q_release_element(el);
        return false;
    }
    memcpy(el->value, s, strlen(s) + 1);

    list_add(&el->list, head);

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *el = malloc(sizeof(element_t));
    if (!el)
        return false;

    el->value = malloc(sizeof(char) * (strlen(s) + 1));
    if (!el->value) {
        q_release_element(el);
        return false;
    }
    memcpy(el->value, s, strlen(s) + 1);

    list_add_tail(&el->list, head);

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || !sp)
        return NULL;

    struct list_head *node = head->next;
    if (node == head)
        return NULL;

    element_t *el = list_entry(node, element_t, list);
    size_t len = strnlen(el->value, bufsize - 1);
    memcpy(sp, el->value, len);
    *(sp + len * sizeof(char)) = '\0';

    list_del(node);

    return el;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || !sp)
        return NULL;

    struct list_head *node = head->prev;
    if (node == head)
        return NULL;

    element_t *el = list_entry(node, element_t, list);
    size_t len = strnlen(el->value, bufsize - 1);
    memcpy(sp, el->value, len);
    *(sp + len * sizeof(char)) = '\0';

    list_del(node);

    return el;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *pos;
    list_for_each (pos, head)
        len++;
    return len;
}

/*
 * Delete a node.
 * i.e. remove and release the corresponding element
 */
#define del_node(node)                                     \
    {                                                      \
        element_t *el = list_entry(node, element_t, list); \
        list_del(node);                                    \
        q_release_element(el);                             \
    }

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (!head || list_empty(head))
        return false;

    // squeeze to find the middle-node
    struct list_head *p = head->next;
    int hindex = 0, tindex = q_size(head) - 1;  // O(n)

    for (; hindex < tindex; hindex++, tindex--, p = p->next)
        ;

    /*
     * hp: go from head to tail
     * tp: go from tail to head
     * odd nodes: head -> ... -> hp == tp -> ...
     * even nodes: head -> ... -> tp -> hp -> ...
     * thus delete "hp"
     * => simplify to one pointer p
     */
    del_node(p);  // delete middle node

    return true;
}

/*
 * Get the value of corresponding element of node.
 */
#define get_value(node) list_entry(node, element_t, list)->value

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/

    if (!head)
        return false;

    if (list_empty(head))
        return true;

    struct list_head *p = head->next;

    for (; p != head && p->next != head;) {
        if (strcmp(get_value(p), get_value(p->next)) == 0) {
            struct list_head *np = p->next;
            char *str = strdup(get_value(np));
            for (; np->next != head && strcmp(str, get_value(np->next)) == 0;) {
                np = np->next;
                del_node(np->prev);
            }
            del_node(np);
            free(str);  // release momery allocated by strdup
            p = p->next;
            del_node(p->prev);
        } else {
            p = p->next;
        }
    }

    return true;
}

/*
 * For internel use.
 * Swap two nodes(whether adjacnt or not)
 * before: head -> ... -> m -> a -> c -> ... -> d -> b -> n -> ...
 * middle: head -> ... -> m -> c -> ... -> d -> n -> ..., a, b
 * result: head -> ... -> m -> b -> c -> ... -> d -> a -> n -> ...
 */

void swap_node(struct list_head *a, struct list_head *b)
{
    struct list_head *m = a->prev;
    struct list_head *n = b->next;
    /*remove a*/
    m->next = a->next;
    a->next->prev = m;
    /*remove b*/
    b->prev->next = n;
    n->prev = b->prev;
    /*insert b*/
    m->next->prev = b;
    b->next = m->next;
    m->next = b;
    b->prev = m;
    /*insert a*/
    n->prev->next = a;
    a->prev = n->prev;
    n->prev = a;
    a->next = n;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/

    if (!head || list_empty(head))
        return;

    struct list_head *p = head->next;

    for (; p != head && p->next != head; p = p->next) {
        swap_node(p, p->next);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    // hp: head pointer, tp: tail pointer
    struct list_head *hp = head->next;
    struct list_head *tp = head->prev;
    int hindex = 0, tindex = q_size(head) - 1;  // O(n)

    for (; hindex < tindex; hindex++, tindex--) {
        struct list_head *a = hp;
        struct list_head *b = tp;
        hp = hp->next;
        tp = tp->prev;
        swap_node(a, b);
    }
}

/*
 * Merge r and l into a circular doubly-linked list by ascending order.
 */
// #define MERGE_SORT_DEBUG

struct list_head *merge(struct list_head *r, struct list_head *l)
{
    /*
     * flatten
     * NULL -> r -> ... -> rt -> NULL
     * NULL -> l -> ... -> lt -> NULL
     */
    r->prev->next = NULL;  // rt->next = NULL
    r->prev = NULL;
    l->prev->next = NULL;  // lt->next = NULL
    l->prev = NULL;

#ifdef MERGE_SORT_DEBUG
    printf("\nmerge! ");
    struct list_head *a;
    for (a = r; a != NULL; a = a->next)
        printf("%s ", get_value(a));
    printf("<> ");

    for (a = l; a != NULL; a = a->next)
        printf("%s ", get_value(a));
    printf("\n-----\n");
#endif

    // decide head
    struct list_head *rp = r, *lp = l;
    struct list_head **cmp;
    cmp = (strcmp(get_value(rp), get_value(lp)) < 0) ? &rp : &lp;
    struct list_head *h = *cmp;
    struct list_head *tail = h;
    *cmp = (*cmp)->next;

#ifdef MERGE_SORT_DEBUG
    printf("head: %s\n", get_value(h));
    int de = 0;
#endif

    for (cmp = NULL; rp && lp; *cmp = (*cmp)->next, tail = tail->next) {
        cmp = (strcmp(get_value(rp), get_value(lp)) < 0) ? &rp : &lp;
        tail->next = *cmp;
        (*cmp)->prev = tail;
#ifdef MERGE_SORT_DEBUG
        printf("%d *cmp = %s new tail = %s\n", ++de, get_value(*cmp),
               get_value(tail->next));
#endif
    }

    // connect the last(rest) node
    cmp = rp ? &rp : &lp;
    for (; *cmp; *cmp = (*cmp)->next, tail = tail->next) {
        tail->next = *cmp;
        tail->next->prev = tail;
#ifdef MERGE_SORT_DEBUG
        printf("add %s\n", get_value(*cmp));
#endif
    }

    // circulate the list
    tail->next = h;
    h->prev = tail;

#ifdef MERGE_SORT_DEBUG
    printf("ret: ");
    for (a = h; a != h->prev; a = a->next)
        printf("%s ", get_value(a));
    printf("%s\n-----\n\n", get_value(a));
#endif

    return h;
}

struct list_head *merge_sort(struct list_head *h)
{
    // single
    if (h->next == h)
        return h;

    int hindex = 0, tindex = 0;
    struct list_head *p;
    for (p = h->next; p != h; tindex++, p = p->next)
        ;

    // find middle node
    p = h;
    for (; hindex < tindex; hindex++, tindex--, p = p->next)
        ;

    /*
     * Split into two circular-doubly-linked list.
     * before: -> h -> ... -> ht -> p -> ... -> pt ->
     * result: -> h -> ... -> ht -> , -> p -> ... -> pt ->
     */

    struct list_head *ht = p->prev;
    struct list_head *pt = h->prev;
    ht->next = h;
    h->prev = ht;
    pt->next = p;
    p->prev = pt;

#ifdef MERGE_SORT_DEBUG
    struct list_head *a;
    printf("h: ");
    for (a = h; a != h->prev; a = a->next)
        printf("%s ", get_value(a));
    printf("%s\np: ", get_value(a));
    for (a = p; a != p->prev; a = a->next)
        printf("%s ", get_value(a));
    printf("%s\n", get_value(a));
#endif

    // recurision
    h = merge_sort(h);
    p = merge_sort(p);

    // merge into one list
    return merge(h, p);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    // dehead, and remake the circle
    struct list_head *p = head->next;
    p->prev = head->prev;
    head->prev->next = p;

    p = merge_sort(p);

    // conect result p to head
    head->next = p;
    head->prev = p->prev;
    p->prev->next = head;
    p->prev = head;

#ifdef MERGE_SORT_DEBUG
    element_t *e;
    list_for_each_entry (e, head, list)
        printf("%s ", e->value);
    printf("\n");
#endif
}
