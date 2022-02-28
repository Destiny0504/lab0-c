#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    if (l == NULL)
        return;

    element_t *cur_node = NULL, *next_node = NULL;
    list_for_each_entry_safe (cur_node, next_node, l, list)
        q_release_element(cur_node);
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

    element_t *tmp_node = malloc(sizeof(element_t));
    if (!tmp_node)
        return false;

    int s_len = sizeof(char) * strlen(s) + 1;
    tmp_node->value = malloc(s_len);
    if (!tmp_node->value) {
        q_release_element(tmp_node);
        return false;
    }

    memset(tmp_node->value, 0, s_len);
    strncpy(tmp_node->value, s, strlen(s) + 1);
    INIT_LIST_HEAD(&tmp_node->list);
    list_add(&tmp_node->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.delete_mid
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *tmp_node = malloc(sizeof(element_t));

    if (!tmp_node)
        return false;

    int s_len = sizeof(char) * strlen(s) + 1;
    tmp_node->value = malloc(s_len);
    if (!tmp_node->value) {
        q_release_element(tmp_node);
        return false;
    }

    memset(tmp_node->value, 0, s_len);
    strncpy(tmp_node->value, s, strlen(s) + 1);
    INIT_LIST_HEAD(&tmp_node->list);
    list_add_tail(&tmp_node->list, head);
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
    if (!head || list_empty(head))
        return NULL;

    element_t *tmp_node = container_of(head->next, element_t, list);
    if (sp) {
        /* avoid to delete a string which is longer than the given bufsize */
        strncpy(sp, tmp_node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->next);
    return tmp_node;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (list_empty(head))
        return NULL;

    element_t *tmp_node = container_of(head->prev, element_t, list);
    if (sp) {
        /* avoid to delete a string which is longer than the given bufsize */
        strncpy(sp, tmp_node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->prev);
    return tmp_node;
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
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
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
    if (head->next == head)
        return false;

    struct list_head *tmp = head->next;
    int counter = 0;
    if (q_size(head) % 2)
        counter = (q_size(head) - 1) / 2;
    else
        counter = q_size(head) / 2;
    for (; counter > 0; counter--) {
        tmp = tmp->next;
    }
    list_del(tmp);
    element_t *tmp_node = container_of(tmp, element_t, list);
    q_release_element(tmp_node);
    return true;
}

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

    if (list_is_singular(head))
        return true;

    struct list_head *cur = head->next, *cur_next = cur->next;
    element_t *cur_node = NULL;
    for (;;) {
        if (strcmp(list_entry(cur, element_t, list)->value,
                   list_entry(cur_next, element_t, list)->value) == 0) {
            list_del(cur);
            cur_node = list_entry(cur, element_t, list);
            q_release_element(cur_node);
        }
        cur = cur_next;
        if (cur->next == head)
            return true;
        cur_next = cur->next;
    }
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    struct list_head *cur = head->next, *cur_next = cur->next;
    // swapping the data
    for (; cur_next != head && cur != head;
         cur = cur->next, cur_next = cur->next) {
        cur->prev->next = cur_next;
        cur_next->next->prev = cur;
        cur_next->prev = cur->prev;
        cur->next = cur_next->next;
        cur_next->next = cur;
        cur->prev = cur_next;
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

    struct list_head *tmp = NULL, *next = head;
    // swapping the data
    for (; tmp != head; next = tmp) {
        tmp = next->next;
        next->next = next->prev;
        next->prev = tmp;
    }
}
struct list_head *merge(struct list_head *l1, struct list_head *l2)
{
    struct list_head *tmp = NULL, *head = NULL;

    if (strcmp(list_entry(l1, element_t, list)->value,
               list_entry(l2, element_t, list)->value) < 0) {
        tmp = l1;
        head = l1;
        l1 = l1->next;
    } else {
        tmp = l2;
        head = l2;
        l2 = l2->next;
    }

    while (l1 && l2) {
        if (strcmp(list_entry(l1, element_t, list)->value,
                   list_entry(l2, element_t, list)->value) < 0) {
            tmp->next = l1;
            tmp = tmp->next;
            l1 = l1->next;
        } else {
            tmp->next = l2;
            tmp = tmp->next;
            l2 = l2->next;
        }
    }
    if (l1)
        tmp->next = l1;
    if (l2)
        tmp->next = l2;
    return head;
}

struct list_head *mergesort(struct list_head *head)
{
    // merge sort
    if (!head || !head->next)
        return head;

    struct list_head *fast = head->next;
    struct list_head *slow = head;

    // split list
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    fast = slow->next;
    slow->next = NULL;

    // sort each list
    struct list_head *l1 = mergesort(head);
    struct list_head *l2 = mergesort(fast);

    // merge sorted l1 and sorted l2
    return merge(l1, l2);
}
/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_is_singular(head) || list_empty(head))
        return;

    struct list_head *cur = NULL;
    head->prev->next = NULL;
    head->next = mergesort(head->next);
    head->next->prev = head;
    list_for_each (cur, head) {
        if (!cur->next) {
            cur->next = head;
            cur->next->prev = cur;
        } else
            cur->next->prev = cur;
    }
}

void q_shuffle(struct list_head *head)
{
    srand(time(NULL));

    // First, we have to know how long is the linked list
    int len = q_size(head);
    struct list_head **indirect = &head->next;

    while (len) {
        int random = rand() % len;
        indirect = &head->next;

        while (random--)
            indirect = &(*indirect)->next;

        list_move_tail(*indirect, head);
        len--;
    }
}
