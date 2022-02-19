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

    for (struct list_head *p = l->next; p != l;) {
        element_t *el = container_of(p, element_t, list);
        p = p->next;
        free(el->value);
        free(el);
    }
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
        // free(el->value);
        free(el);
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
        free(el->value);
        free(el);
        return false;
    }
    memcpy(el->value, s, strlen(s) + 1);

    list_add_tail(&el->list, head);

    return true;
}

/*
 * Get smaller number between two integer without checking.
 */
#define min(x, y) (x) < (y) ? (x) : (y)

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

    element_t *el = container_of(node, element_t, list);
    // TODO: handle when bufsize < sizeof(el->value)
    int len = min(strlen(el->value), bufsize - 1);
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

    element_t *el = container_of(node, element_t, list);
    // TODO: handle when bufsize < sizeof(el->value)
    int len = min(strlen(el->value), bufsize - 1);
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

#define del_node(node)                                       \
    {                                                        \
        element_t *el = container_of(node, element_t, list); \
        list_del(node);                                      \
        q_release_element(el);                               \
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

    // hp: head pointer, tp: tail pointer
    // squeeze to find the middle-node
    struct list_head *hp = head->next;
    struct list_head *tp = head->prev;
    int hindex = 0, tindex = q_size(head) - 1;  // O(n)

    for (; hindex < tindex; hindex++, tindex--) {
        hp = hp->next;
        tp = tp->prev;
    }

    /*
     * odd nodes: head -> ... -> hp == tp -> ...
     * even nodes: head -> ... -> tp -> hp -> ...
     * thus delete "hp"
     */
    del_node(hp);  // delete middle node

    return true;
}

#define get_value(node) container_of(node, element_t, list)->value

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

    for (; p->next != head;) {
        if (strcmp(get_value(p), get_value(p->next)) == 0) {
            struct list_head *np = p->next;
            char *str = strdup(get_value(np));
            for (; np->next != head && strcmp(str, get_value(np->next)) == 0;) {
                np = np->next;
                del_node(np->prev);
            }
            del_node(np);
        }
        p = p->next;
        del_node(p->prev);
    }

    return true;
}

/*
 * head -> ... -> node_a -> node_b -> ...
 */
#define swap_node_adjacent(node_a, node_b)     \
    {                                          \
        struct list_head *prev = node_a->prev; \
        struct list_head *next = node_b->next; \
        prev->next = node_b;                   \
        node_b->prev = prev;                   \
        node_b->next = node_a;                 \
        node_a->prev = node_b;                 \
        node_a->next = next;                   \
        next->prev = node_a;                   \
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
        swap_node_adjacent(p, p->next);
    }
}

/*
 * before: head -> ... -> p -> a -> c -> ... -> d -> b -> n -> ...
 * result: head -> ... -> p -> b -> c -> ... -> d -> a -> n -> ...
 * TODO: debug
 */
#define swap_node_nonadjacent(a, b)    \
    {                                  \
        struct list_head *p = a->prev; \
        struct list_head *c = a->next; \
        struct list_head *d = b->prev; \
        struct list_head *n = b->next; \
        p->next = b;                   \
        b->prev = p;                   \
        b->next = c;                   \
        c->prev = b;                   \
        d->next = a;                   \
        a->prev = d;                   \
        a->next = n;                   \
        n->prev = a;                   \
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

    // TODO: debug
    // hp: head pointer, tp: tail pointer
    struct list_head *hp = head->next;
    struct list_head *tp = head->prev;
    int hindex = 0, tindex = q_size(head) - 1;  // O(n)

    for (; hindex < tindex; hindex++, tindex--) {
        struct list_head *a = hp;
        struct list_head *b = tp;
        hp = hp->next;
        tp = tp->prev;
        swap_node_nonadjacent(a, b);
    }
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head) {}
