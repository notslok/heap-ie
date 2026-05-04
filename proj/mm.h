#ifndef __MM_H__
#define __MM_H__

#include <stddef.h> /*for size_t*/
#include <stdint.h>

#define MM_MAX_STRUCT_NAME 32
#define SIZEOF_META_DATA_BLOCK 28 // bytes

typedef enum {
    MM_FALSE,
    MM_TRUE
} vm_bool_t;

typedef struct vm_page_family_ {

    char struct_name[MM_MAX_STRUCT_NAME];
    uint32_t struct_size;
    struct vm_page_* first_page; // <--------> pointer to User defined data structure to represent a single unit of VM page

} vm_page_family_t;


typedef struct vm_page_for_families_ {

    struct vm_page_for_families_* next;
    vm_page_family_t vm_page_family[0];


} vm_page_for_families_t;


typedef struct block_meta_data_ {

    /* [4 bytes] Is the corresponding Data block Free or Allocated? */
    vm_bool_t is_free;
    
    /* [4 bytes] Size of the corresponding Data Block */
    uint32_t block_size;

    /* [8 bytes] ptr to the next meta block - downward in Data VM page */
    struct block_meta_data_* prev_block;

    /* [8 bytes] ptr to the next meta block - upward in Data VM page */
    struct block_meta_data_* next_block;

    /* [4 bytes] Offset of this data block w.r.t the start of this VM page */
    uint32_t offset;
    
} block_meta_data_t;

typedef struct vm_page_{
    struct vm_page_* next;
    struct vm_page_* prev;
    struct vm_page_family_* pg_family; /* back pointer to vm_page_for families_t entry */
    block_meta_data_t block_meta_data;
    char page_memory[0]; /* first data block in the VM page */
} vm_page_t;


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
#define ITERATE_PAGE_FAMILIES_BEGIN(vm_page_for_families_ptr, curr)                \
{                                                                                  \
    uint32_t _in_page_family_count = 0;                                            \
    for(curr = (vm_page_family_t*)&vm_page_for_families_ptr->vm_page_family[0];    \
        curr->struct_size && _in_page_family_count < MAX_FAMILIES_PER_VM_PAGE;     \
        curr++, _in_page_family_count++) {                                         

#define ITERATE_PAGE_FAMILIES_END(vm_page_for_families_ptr, curr)}}                \


/*
Looping macro to iterate over nodes of vm_page_for_families_t linked list in allocated VM Pages
*/
#define ITERATE_VM_FAMILY_PAGES_BEGIN(vm_page_iterator)            \
{                                                          \
    for(;                                                  \
        vm_page_iterator!=NULL;                            \
        vm_page_iterator = vm_page_iterator->next){        \

#define ITERATE_VM_FAMILY_PAGES_END(vm_page_iterator)}}            \


/* 
    Meta Block and Data Block related Macros 
*/

// Generic macro to obtain offset of any field in a structure
#define offset_of(container_structure, field_name)          \
        ((size_t)&(((container_structure*)NULL)->field_name)) \


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111[[!!!USE OFFSET FIELD!!!]]
// Macro to obtain the start address of a VM page, on being 
// given any pointer pointing to the start of the meta block
#define MM_GET_PAGE_FROM_META_BLOCK(block_meta_data_ptr)                 \
((void*)((char*)(block_meta_data_ptr) - block_meta_data_ptr->offset))    \

// Returns pointer pointing to the start of next meta block's
// starting i.e. Towards higher address. 
#define NEXT_META_BLOCK(block_meta_data_ptr)                \
        if(!block_meta_data_ptr->next_block){               \
            printf("[INFO] Pointer already pointing to the last meta block in the VM page!\n");     \
        }                                                   \
        (block_meta_data_ptr->next_block;)                  \

        
// Returns pointer pointing to the start of next meta block's
// starting i.e. Towards higher address. BUT this macro leverages
// the block_size attribute AND NOT the _next_block pointer
/* STEPS: */
// first get to the start of data block using (block_meta_data_ptr+1)
// then conver to typecast to char* which is 1 byte long and jump
// by block_meta_data_ptr->block_size bytes to get to the end of data block 
// then typecast back to block_meta_data_ptr*
#define NEXT_META_BLOCK_BY_SIZE(block_meta_data_ptr)                    \
        (block_meta_data_ptr*)((char*)(block_meta_data_ptr+1)           \
        + block_meta_data_ptr->block_size)                              \
         

// Returns pointer pointing to the start of preveious meta block's
// starting i.e. Towards lower address.
#define PREV_META_BLOCK(block_meta_data_ptr)                \
        if(!block_meta_data_ptr->prev_block){               \
            printf("[INFO] Pointer already pointing to the first meta block in the VM page!\n");    \
        }                                                   \
        (block_meta_data_ptr->prev_block;)                  \

// Macro to take the responsibility to update all P and N pointers of all meta blocks that needs an update.
// first argument is pointer to meta block of allocated data block and 2nd argument is pointer to Meta block 
// of free data block that is formed as a result of split.
/*
        STEPS:
        1) Join the free block to the blocks in the higher memory side
        2) Join the free block to the new allocated block, towards the lower memory side
*/
#define mm_bind_blocks_for_allocation(allocated_meta_block, free_meta_block)    \
        free_meta_block->next_block = allocated_meta_block->next_block;         \
        if(allocated_meta_block->next_block != NULL) {                          \
            allocated_meta_block->next_block->prev_block-> free_meta_block;     \
        }                                                                       \
        free_meta_block->prev_block = allocated_meta_block;                     \
        allocated_meta_block->next = free_meta_block;                           \


// MACRO "for loop" which iterates over all meta blocks present in a VM page, 
// from lower memory towards higher memory. The "first_meta_block" is a pointer
// to the very fisrt meta_block in a VM page @ LA
#define ITERATE_VM_PAGE_META_BLOCKS_BEGIN(first_meta_block)     \
{                                                               \
    uint32_t free_block_count = 0;                              \
    uint32_t allocated_block_count = 0;                         \
    uint32_t max_alloc_blk_size = 0;                            \
    uint32_t max_free_blk_size = 0;                             \
    uintptr_t biggest_alloc_blk_addr = 0;                       \
    uintptr_t biggest_free_blk_addr = 0;                        \
                                                                \
    block_meta_data_t* block_iterator = NULL;               \
    for(block_iterator = first_meta_block;                  \
        block_iterator->next_block != NULL;                 \
        block_iterator = block_iterator->next_block){       \
                                                                            \
            if(block_iterator->is_free){                                    \
                ++free_block_count;                                         \
                if(max_free_blk_size < block_iterator->block_size){         \
                    max_free_blk_size =  block_iterator->block_size;        \
                    biggest_free_blk_addr = (uintptr_t)&block_iterator;     \
                }                                                           \
            } else {                                                        \
                ++allocated_block_count;                                    \
                if(max_alloc_blk_size < block_iterator->block_size){        \
                    max_alloc_blk_size =  block_iterator->block_size;       \
                    biggest_alloc_blk_addr = (uintptr_t)&block_iterator;    \
                }                                                           \
            }                                                               \
if(block_iterator->prev_block != NULL &&                            \
           block_iterator->prev_block->is_free                      \
           && block_iterator->is_free)                              \
           assert(0);                                               \

#define ITERATE_VM_PAGE_META_BLOCKS_END(first_meta_block)}}         \

/* Macro to iterate over all the VM data pages */
#define ITERATE_VM_PAGE_BEGIN(vm_page_family_ptr, curr) \
{                                                       \
    for(curr = vm_page_family_ptr;                      \
        curr->next != NULL;                             \
        curr = curr->next;){                            

#define ITERATE_VM_PAGE_END(vm_page_family_ptr, curr)}} \

/* Iterative macro to loop over all Meta blocks of a given VM Data type */
#define ITERATE_VM_PAGE_ALL_BLOCKS_BEGIN(vm_page_ptr, curr)     \
{                                                               \
    for(curr = vm_page_ptr.block_meta_data;                     \
        curr->next_block != NULL;                               \
        curr = curr->next_block){                               \
    
#define ITERATE_VM_PAGE_ALL_BLOCKS_END(vm_page_ptr, curr)}}     \


/* Return MM_TRUE if VM_Page has no data block assigned to
the application, else return MM_FALSE */
vm_bool_t
mm_is_vm_page_empty(vm_page_t* vm_page);


/* Write a macro which marks VM data page as empty */
#define MARK_VM_PAGE_EMPTY(vm_page_t_ptr)                      \
    vm_page_t_ptr->block_meta_data.is_free = MM_TRUE;          \
    vm_page_t_ptr->block_meta_data.prev_block = NULL;          \
    vm_page_t_ptr->block_meta_data.next_block = NULL;          \
    // vm_page_t_ptr->block_meta_data.offset = 0;                 
    // vm_page_t_ptr->block_meta_data.block_size = SYSTEM_PAGE_SIZE -                          \
    //                                                 offset_of(block_meta_data_t, offset) -  \
    //                                                 offset_of(vm_page_t, page_memory);      \

/* API to allocate new data VM page for a family */
vm_page_t*
allocate_vm_page(vm_page_family_t* vm_page_family);


/* API to deallocate and free( i.e. return back to kernel) an allocated empty VM data page */
void
mm_vm_page_delete_and_free(vm_page_t* vm_page);

#endif /* __MM_H__ */