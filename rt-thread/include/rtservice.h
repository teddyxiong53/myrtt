#ifndef __RT_SERVICE_H__
#define __RT_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define rt_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define RT_LIST_OBJECT_INIT(object) {&(object), &(object)}

rt_inline void rt_list_init(rt_list_t *l)
{
	l->next = l->prev = l;
}

rt_inline void rt_list_insert_after(rt_list_t *l, rt_list_t *n)
{
	l->next->prev = n;
	n->next = l->next;
	l->next = n;
	n->prev = l;
}

rt_inline void rt_list_insert_before(rt_list_t *l, rt_list_t *n)
{
	l->prev->next = n;
	n->prev = l->prev;
	l->prev = n;
	n->next = l;
}

rt_inline void rt_list_remove(rt_list_t *n)
{
	n->next->prev = n->prev;
	n->prev->next = n->next;
	n->next = n->prev = n;
}

rt_inline int rt_list_isempty(const rt_list_t *l)
{
	return l->next == l;
}

rt_inline unsigned int rt_list_len(const rt_list_t *l)
{
	unsigned int len;
	const rt_list_t *p = l;
	while(p->next != l) {
		p = p->next;
		len ++;
	}
	return len;
}

#define rt_list_entry(node, type, member) \
	rt_container_of(node, type, member)
	

#define rt_list_for_each_entry(pos, head, member) \
    for (pos = rt_list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = rt_list_entry(pos->member.next, typeof(*pos), member))

#define rt_list_for_each_entry_safe(pos, n, head, member) \
    for (pos = rt_list_entry((head)->next, typeof(*pos), member), \
         n = rt_list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = n, n = rt_list_entry(n->member.next, typeof(*n), member))

#define rt_list_first_entry(ptr, type, member) \
    rt_list_entry((ptr)->next, type, member)		 
	
	
// slist
#define RT_SLIST_OBJECT_INIT(object) {RT_NULL}

rt_inline void rt_slist_init(rt_slist_t *l)
{
	l->next = RT_NULL;
}

rt_inline void rt_slist_append(rt_slist_t *l, rt_slist_t *n)
{
	struct rt_slist_node *node;
	node = l;
	while(node->next) {
		node = node->next;
	}
	node->next = n;
	n->next = RT_NULL;
}

rt_inline void rt_slist_insert(rt_slist_t *l, rt_slist_t *n)
{
	n->next = l->next;
	l->next = n;
}

rt_inline unsigned int rt_slist_len(const rt_list_t *l)
{
	unsigned int len = 0;
	const rt_list_t *list = l->next;
	while(list!=RT_NULL) {
		list = list->next;
		len ++;
	}
	return len;
}

rt_inline rt_slist_t *rt_slist_remove(rt_slist_t *l, rt_slist_t *n)
{
	struct rt_slist_node *node = l;
	while(node->next && node->next != n) {
		node = node->next;
	}
	if(node->next != (rt_slist_t*)0) {
		node->next = node->next->next;
	}
	return l;
}

rt_inline int rt_slist_isempty(rt_slist_t *l) {
	return l->next == RT_NULL;
}

#define rt_slist_entry(node, type, member) \
    rt_container_of(node, type, member)

	
#define rt_slist_fore_each_entry(pos, head, member) \
    for (pos = rt_slist_entry((head)->next, typeof(*pos), member); \
         &pos->member != (RT_NULL); \
         pos = rt_slist_entry(pos->member.next, typeof(*pos), member))
		 
		 
#define rt_slist_first_entry(ptr, type, member) \
    rt_slist_entry((ptr)->next, type, member)
#ifdef __cplusplus
}
#endif
#endif