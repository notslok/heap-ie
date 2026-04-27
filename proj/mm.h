#ifndef MM_H
#define MM_H

#include <stddef.h> /*for size_t*/
#include <stdint.h>

#define MM_MAX_STRUCT_NAME 32

typedef struct vm_page_family_ {

    char struct_name[MM_MAX_STRUCT_NAME];
    uint32_t struct_size;

} vm_page_family_t;


typedef struct vm_page_for_families_ {

    struct vm_page_for_families_* next;
    vm_page_family_t vm_page_family[0];

} vm_page_for_families_t;


/* 
    Iterates over *all VM pages* hosting page families (vm_page_for_families_t), 
    and returns the pointer to the page family object identified by struct_name passed as an argument 
*/
struct vm_page_family_*
lookup_page_family_by_name(char* struct_name);


/* (Total system page size - size of next pointer) / (size of one page family entry)*/
#define MAX_FAMILIES_PER_VM_PAGE    \
    (SYSTEM_PAGE_SIZE - sizeof(vm_page_for_families_t *)) / (sizeof(vm_page_family_t))



/* 
Looping macro to iterate over all page families (vm_page_family_t) objects
stored in a VM page (vm_page_for_families_t) in bottom-up fashion
*/

/*  MORE EFFICIENT ITERATIVE MACRO:
    for (init the curr to the first vm_page_family_t in the first vm page;
        check if vm_page_family_t exist by checking its size  && check if curr has iterated all the family entries;
        increment the curr ptr towards next family entry, also increment the in_page_family counter)
*/
#define ITERATE_PAGE_FAMILIES_BEGIN(vm_page_for_families_ptr, curr)               \
{                                                                                 \
    uint32_t _in_page_family_count = 0;                                            \
    for(curr = (vm_page_family_t*)&vm_page_for_families_ptr->vm_page_family[0];    \
        curr->struct_size && _in_page_family_count < MAX_FAMILIES_PER_VM_PAGE;     \
        curr++, _in_page_family_count++) {                                         \

#define ITERATE_PAGE_FAMILIES_END(vm_page_for_families_ptr, curr)}}                \


/*
Looping macro to iterate over nodes of vm_page_for_families_t linked list in allocated VM Pages
*/
#define ITERATE_VM_PAGE_BEGIN(vm_page_iterator)            \
{                                                          \
    for(;                                                  \
        vm_page_iterator!=NULL;                            \
        vm_page_iterator = vm_page_iterator->next){        \

#define ITERATE_VM_PAGE_END(vm_page_iterator)}}            \


#endif