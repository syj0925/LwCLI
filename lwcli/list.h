

#ifndef __LIST_H__
#define __LIST_H__

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

/*
 * Get offset of a member variable.
 *
 * @param[in]  type    the type of the struct this is embedded in.
 * @param[in]  member  the name of the variable within the struct.
 */
#define GetOffsetOf(type, member) ((size_t)&(((type *)0)->member))

/*
 * Get the struct for this entry.
 *
 * @param[in]  ptr     the list head to take the element from.
 * @param[in]  type    the type of the struct this is embedded in.
 * @param[in]  member  the name of the variable within the struct.
 */
#define GetContainerOf(ptr, type, member) \
        ((type *)((char *)(ptr) - GetOffsetOf(type, member)))

typedef struct dlist
{
    struct dlist *prev;
	struct dlist *next;
} dlist_t;

/*
 * initialize one node.
 *
 * @param[in]  node    the data node to be initialized.
 */
static inline void DlistInit(dlist_t *node)
{
    node->next = node->prev = node;
}

/*
 * add one node into data list.
 *
 * @param[in]  node    the data node to be inserted.
 * @param[in]  prev    previous data node address.
 * @param[in]  next    next data node address.
 */
static inline void dlistAdd(dlist_t *node, dlist_t *prev, dlist_t *next)
{
    node->next = next;
    node->prev = prev;

    prev->next = node;
    next->prev = node;
}

/*
 * add one node just behind specified node.
 *
 * @param[in]  node    the data node to be inserted.
 * @param[in]  queue   the specified last node.
 */
static inline void DlistAdd(dlist_t *node, dlist_t *queue)
{
    dlistAdd(node, queue, queue->next);
}

/*
 * add one node just before specified node.
 *
 * @param[in]  node    the data node to be inserted.
 * @param[in]  queue   the specified next node before which new node should be inserted.
 */
static inline void DlistAddTail(dlist_t *node, dlist_t *queue)
{
    dlistAdd(node, queue->prev, queue);
}

/*
 * delete one node from the data list.
 *
 * @param[in]  node    the data node to be deleted.
 */
static inline void DlistDel(dlist_t *node)
{
    dlist_t *prev = node->prev;
    dlist_t *next = node->next;

    prev->next = next;
    next->prev = prev;
}

/*
 * Judge whether data list is empty.
 *
 * @param[in]  head    the head node of data list.
 *
 * @return 1 on empty, 0 FALSE.
 */
static inline int DlistIsEmpty(const dlist_t *head)
{
    return head->next == head;
}

/*
 * Get the struct for this entry.
 *
 * @param[in]  addr    the list head to take the element from.
 * @param[in]  type    the type of the struct this is embedded in.
 * @param[in]  member  the name of the xdlist_t within the struct.
 */
#define DlistEntry(addr, type, member) \
        ((type *)((long)addr - GetOffsetOf(type, member)))

/*
 * Get the first element from a list
 *
 * @param[in]  ptr     the list head to take the element from.
 * @param[in]  type    the type of the struct this is embedded in.
 * @param[in]  member  the name of the xdlist_t within the struct.
 */
#define DlistFirstEntry(ptr, type, member) DlistEntry((ptr)->next, type, member)

/*
 * Get the last element from a list
 *
 * @param[in]  ptr     the list head to take the element from.
 * @param[in]  type    the type of the struct this is embedded in.
 * @param[in]  member  the name of the xdlist_t within the struct.
 */
#define DlistLastEntry(ptr, type, member) DlistEntry((ptr)->prev, type, member)

/*
 * Iterate over list of given type.
 *
 * @param[in]  queue   he head for your list.
 * @param[in]  node    the &struct xdlist_t to use as a loop cursor.
 * @param[in]  type    the type of the struct this is embedded in.
 * @param[in]  member  the name of the xdlist_t within the struct.
 */
#define DlistForEachEntry(queue, node, type, member)            \
        for (node = GetContainerOf((queue)->next, type, member); \
            &node->member != (queue);                              \
            node = GetContainerOf(node->member.next, type, member))

/*
 * Get the list length.
 *
 * @param[in]  queue  the head for your list.
 *
 * @return list length.
 */
static inline int DlistEntryNumber(dlist_t *queue)
{
    int num;

    dlist_t *cur = queue;

    for (num = 0; cur->next != queue; cur = cur->next, num++)
        ;

    return num;
}

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __LIST_H__


