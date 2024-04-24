#include "kernel/list.h"
#include "stdio.h"
#include "threads/malloc.h"
#include "listpop.h"

struct element
{
    struct list_elem elem;
    int priority;
};

void populate(struct list * l, int * a, int n) {
    int i;
    struct element * new_element;
    for (i = 0; i < n; i++) {
        new_element = malloc(sizeof(struct element));
        new_element->priority = *(a + i);
        list_push_back(l, &(new_element->elem));
    }
}

void clean_list(struct list * l) {
    struct list_elem * pos;
    
}

 
bool compare_elements(const struct list_elem *a,
                   const struct list_elem *b, void * aux)
{
    struct element *ia = list_entry(a, struct element, elem);
    struct element *ib = list_entry(b, struct element, elem);
    return (ia->priority < ib->priority);
}

void print_sorted(struct list *l)
{
    list_sort(l, compare_elements, NULL);

    struct list_elem * pos;

    for (pos = list_begin(l); pos != list_end(l); pos = list_next(pos)) {
        struct element * elem_tmp; 
        elem_tmp = list_entry(pos, struct element, elem);
        printf(" %d", elem_tmp->priority);
    }
    printf("\n");
}

void test_list()
{
    struct list elem_list;
    list_init(&elem_list);

    populate(&elem_list, ITEMARRAY, ITEMCOUNT);
    print_sorted(&elem_list);
    // clean_list(&elem_list);
}