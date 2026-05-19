#include "mm.h"
#include <stdio.h>
#include <memory.h>
#include <unistd.h>     /* for getpagesize() */
#include <sys/mman.h>   /* for using mmap() */
#include <assert.h>
#include <stdint.h>


static size_t SYSTEM_PAGE_SIZE = 0;
static vm_page_for_families_t* first_vm_page_for_families = NULL;

void 
mm_init() {
    SYSTEM_PAGE_SIZE = getpagesize();
}

/* Comparator for insertion of free block's metadata based on block-size */
static int
free_blocks_comparision_function(
        void* _block_meta_data_1,
        void* _block_meta_data_2){
    block_meta_data_t* block_meta_data_1 = (block_meta_data_t*) _block_meta_data_1;
    block_meta_data_t* block_meta_data_2 = (block_meta_data_t*) _block_meta_data_2;


    if (block_meta_data_1->block_size > block_meta_data_2->block_size)
        return -1;
    else if (block_meta_data_1->block_size < block_meta_data_2->block_size)
        return 1;

    return 0; // equal block size
}

/* Insertion fn. for free data-block's meta-block into glthread based priority queue */
static void
mm_add_free_block_meta_data_to_free_block_list (
    vm_page_family_t* vm_page_family,
    block_meta_data_t* free_block) {
    
    /* Verify if the data-block represented by this meta-block is actually free */
    assert(free_block->is_free == MM_TRUE);

    glthread_priority_insert( &vm_page_family->free_block_priority_list_head,
                              &free_block->priority_thread_glue,
                              free_blocks_comparision_function,
                              offset_of(block_meta_data_t, priority_thread_glue));
}

/* 
    API to GET the biggest free data block from priority Queue of a given page family 
    Because the current insertion policy keeps the biggest size at the head, it will end up deleting
    the very first node everytime.
*/
static inline block_meta_data_t*
mm_get_biggest_free_block_page_family(vm_page_family_t* vm_page_family){
    
    glthread_t* glthreadptrstart = &vm_page_family->free_block_priority_list_head;
    glthread_t* biggest_glthreadptr = NULL;
    glthread_t* glthreadptr = NULL;

    ITERATE_GLTHREAD_BEGIN(glthreadptrstart, glthreadptr){
        if(!biggest_glthreadptr){
            biggest_glthreadptr = glthreadptr;
        }
        else{
            biggest_glthreadptr = 
                ((glthread_to_block_meta_data(biggest_glthreadptr)->block_size) >= 
                glthread_to_block_meta_data(glthreadptr)->block_size) 
                ? biggest_glthreadptr : glthreadptr;
        }
    }ITERATE_GLTHREAD_END(glthreadptrstart, glthreadptr);

    return glthread_to_block_meta_data(biggest_glthreadptr);
}


/* Prints out all the registered family name and corresponding size */
void
mm_print_registered_page_families() {

    /* Check if there exists an allocated VM page */
    if(!first_vm_page_for_families){
        printf("[INFO] No VM page allocated for the LMM, yet!\n");
        return;
    }

    
    /* If there is/are allocated VM page - select them one by one for iteration */
    vm_page_for_families_t* vm_page_iterator = first_vm_page_for_families;

    ITERATE_VM_FAMILY_PAGES_BEGIN(vm_page_iterator) {
        
        /* Iterate over the page families inside current VM page */
        vm_page_family_t* vm_page_family_curr = NULL;

        ITERATE_PAGE_FAMILIES_BEGIN(vm_page_iterator, vm_page_family_curr) {

            printf("Page Family: %s, Size = %u\n", 
                    vm_page_family_curr->struct_name,
                    vm_page_family_curr->struct_size);

        } ITERATE_PAGE_FAMILIES_END(vm_page_iterator, vm_page_family_curr);

    } ITERATE_VM_FAMILY_PAGES_END(vm_page_iterator);
}


/* 
    Iterates over *all VM pages* hosting page families (vm_page_for_families_t), 
    and returns the pointer to the page family object identified by struct_name passed as an argument 
*/
vm_page_family_t*
lookup_page_family_by_name(char* struct_name) {
    
    /* Check if there exists an allocated VM page */
    if(!first_vm_page_for_families){
        printf("[INFO] No VM page allocated for the LMM, yet!\n");
        return NULL;
    }

    /* If there is/are allocated VM page - select them one by one for iteration */
    vm_page_for_families_t* vm_page_iterator = first_vm_page_for_families;

    ITERATE_VM_FAMILY_PAGES_BEGIN(vm_page_iterator) {
        
        /* Iterate over the page families inside current VM page */
        vm_page_family_t* vm_page_family_curr = NULL;

        ITERATE_PAGE_FAMILIES_BEGIN(vm_page_iterator, vm_page_family_curr) {

            if(strncmp(vm_page_family_curr->struct_name,
                        struct_name, MM_MAX_STRUCT_NAME) == 0) {

                printf("[INFO] Family with name: %s found @ address: %p\n", 
                    vm_page_family_curr->struct_name, vm_page_family_curr);

                return vm_page_family_curr;
            }

        } ITERATE_PAGE_FAMILIES_END(vm_page_iterator, vm_page_family_curr);

    } ITERATE_VM_FAMILY_PAGES_END(vm_page_iterator);

    printf("[INFO] No family with name: %s found!\n", struct_name);
    return NULL;
}


/* API to request VM page from kernel */
static void*
mm_get_new_vm_page_from_kernel (int units) {
    
    // get memory from kernel using mmap() syscall
    char* vm_page = mmap(
        0, // void addr[length]
        units * SYSTEM_PAGE_SIZE, // length
        PROT_READ|PROT_WRITE|PROT_EXEC, // PROTECTION FLAGS for r/w/exec
        MAP_ANON|MAP_PRIVATE, // flags ---> allocated address in processes' private address space w/o any fd
        0, 0); //fd and offset    

    if(vm_page == MAP_FAILED){ // MAP_FAILED (that is, (void *) -1) is returned
        printf("Error: VM page allocation failed\n");
        return NULL;
    }

    // init the entirety of allocated memory region with 0
    memset(vm_page, 0, units * SYSTEM_PAGE_SIZE);

    return (void *)vm_page;
}

void
mm_instantiate_new_page_family (char* struct_name, uint32_t struct_size) {
    
    /*
        STEP 1: create new page family
    */
    vm_page_family_t* vm_page_family_curr = NULL;
    vm_page_for_families_t* new_vm_page_for_families = NULL;

    if(struct_size > SYSTEM_PAGE_SIZE){
        printf("Error: %s() Structure %s Size eceeds system page size\n",
                __FUNCTION__, struct_name);
        return;
    }

    /*
        STEP 2: 
        If LMM has not taken its first "vm_page_for_families_t" VM page,
        allocate one from the kernel, update "first_vm_page_for_families" global pointer
    */
    if(!first_vm_page_for_families) { // that means very first registration is taking place
        first_vm_page_for_families = (vm_page_for_families_t*)mm_get_new_vm_page_from_kernel(1);
        first_vm_page_for_families->next = NULL;
        // instantiate the first family entry's value
        strncpy(first_vm_page_for_families->vm_page_family[0].struct_name, 
            struct_name, MM_MAX_STRUCT_NAME);

        first_vm_page_for_families->vm_page_family[0].struct_size = struct_size;
        first_vm_page_for_families->vm_page_family[0].first_page = NULL;
        init_glthread(&first_vm_page_for_families->vm_page_family[0].free_block_priority_list_head);

        return;
    }

    /*
        STEP 3:
        check if new "vm_page_family_t" can be accomodated info "first_vm_page_for_families" VM page
    */
        uint32_t in_page_family_count = 0;

        ITERATE_PAGE_FAMILIES_BEGIN (first_vm_page_for_families, vm_page_family_curr) {

            // printf("[DEBUG] %s\n", vm_page_family_curr->struct_name);
            // check if a family with same struct_name has been registered before (?)
            if(strncmp(vm_page_family_curr->struct_name,
                    struct_name, MM_MAX_STRUCT_NAME) != 0){
                in_page_family_count++;
                continue;            
            }
            // printf("[DEBUG] %s\n", vm_page_family_curr->struct_name);
            // deliberately fail because same family can't be registered twice
            assert(0);

        } ITERATE_PAGE_FAMILIES_END (first_vm_page_for_families, vm_page_family_curr);

        // If current vm_page is full as for new page and do insertion there
        if(in_page_family_count == MAX_FAMILIES_PER_VM_PAGE) {
            // get new page from the kernel
            new_vm_page_for_families = (vm_page_for_families_t*)mm_get_new_vm_page_from_kernel(1);
            
            // Update the first_vm_page_for_families pointer
            new_vm_page_for_families->next = first_vm_page_for_families;
            first_vm_page_for_families = new_vm_page_for_families;

            vm_page_family_curr = &first_vm_page_for_families->vm_page_family[0];
        }
        
        // At this point whether we are inserting in same VM page or 
        // have requested for a new one -> the vm_page_family_curr pointer points
        // to the location in array withing page where the new page_family entry needs to go.

        strncpy(vm_page_family_curr->struct_name,
                struct_name, MM_MAX_STRUCT_NAME);
        vm_page_family_curr->struct_size = struct_size;
        vm_page_family_curr->first_page = NULL;
        init_glthread(&vm_page_family_curr->free_block_priority_list_head);

        return;
    // }
}

/* API to perform block merge --> Refer to Block Merge diagram in Readme.md */
static void
mm_union_free_blocks
(block_meta_data_t* first, block_meta_data_t* second){
    // Check if both the blocks are marked to be freed
    assert(first->is_free == MM_TRUE &&
            second->is_free == MM_TRUE);
    
    first->block_size += sizeof(block_meta_data_t) + // size of meta block itself + data block size
                         second->block_size;

    first->next_block = second->next_block;
    if(second->next_block != NULL)
        second->next_block->prev_block = first;
}

/* API  to return a page back to the kernel */
static void
mm_return_vm_page_to_kernel (void* vm_page, int units) {

    // return memory back to kernel using munmap() syscall
    if(munmap(vm_page, units * SYSTEM_PAGE_SIZE)) { // returns 0 on success
        printf("Error: VM page de-allocation failed\n");
    }
}

/* Returns the size of Free Data block of an Empty VM Page */
/* For Giant VM pages a unit of 2 or more is passed as arg */
static inline uint32_t
mm_max_page_allocatable_memory (int units){
    return ((SYSTEM_PAGE_SIZE * units) -
            offset_of(vm_page_t, page_memory)); // Equivalent to substracting the sizeof(vm_page_t) struct.
}

/* API returns MM_TRUE if VM_Page has no data block assigned to
the application, else return MM_FALSE */
vm_bool_t
mm_is_vm_page_empty(vm_page_t* vm_page){
    if(vm_page->block_meta_data.next_block == NULL &&
       vm_page->block_meta_data.prev_block == NULL &&
       vm_page->block_meta_data.is_free == MM_TRUE)
        return MM_TRUE;
    else    
        return MM_FALSE;
}



/* API to allocate new data VM page for a family */
vm_page_t*
allocate_vm_page(vm_page_family_t* vm_page_family){
    
    // Get new VM Data Page from the kernel
    vm_page_t* new_vm_data_page = (vm_page_t*)mm_get_new_vm_page_from_kernel(1);
    
    // Init the meta block value in the new VM data page
    /*
        new_vm_data_page->block_meta_data.is_free = MM_TRUE;
        new_vm_data_page->block_meta_data.prev_block = NULL;
        new_vm_data_page->block_meta_data.next_block = NULL;
    */ 
    // OR use macro:
    MARK_VM_PAGE_EMPTY(new_vm_data_page);
    new_vm_data_page->block_meta_data.offset = offset_of(vm_page_t, page_memory);
    new_vm_data_page->block_meta_data.block_size = mm_max_page_allocatable_memory(1);
    // init priority_thread_glue member -> which is a node of a priority queue tracking free block's meta-block
    init_glthread(&new_vm_data_page->block_meta_data.priority_thread_glue);
    new_vm_data_page->prev = NULL;
    new_vm_data_page->next = NULL;
    
    /*Init the glthread of new VM pages meta-block*/
    init_glthread(&new_vm_data_page->block_meta_data.priority_thread_glue);
    /* Set the back pointer of the data VM page to the respective family struct in family page */
    new_vm_data_page->pg_family = vm_page_family;

    if(vm_page_family->first_page == NULL){
        vm_page_family->first_page = new_vm_data_page; // i.e. its the very first allocated page for this process
    }
    else{
        new_vm_data_page->next = vm_page_family->first_page;
        vm_page_family->first_page->prev = new_vm_data_page;

        vm_page_family->first_page = new_vm_data_page;
    }

    return new_vm_data_page;

}


/* API to deallocate and free( i.e. return back to kernel) an allocated empty VM data page */
void
mm_vm_page_delete_and_free(vm_page_t* vm_page){

    // check if its first_page of a family
    vm_page_family_t* vm_page_family = vm_page->pg_family;

    // Scenario 1: The page being deleted is the head of Linked List.
    if(vm_page_family->first_page == vm_page){
        vm_page_family->first_page = vm_page->next;

        // Detach the node to be deleted
        if(vm_page->next)
            vm_page->next->prev = NULL;
        vm_page->next = NULL;
        vm_page->prev = NULL;

        mm_return_vm_page_to_kernel((void*) vm_page, 1);
        return;
    }

    // Scenario 2: The page being deleted any other node apart from the head.
    if(vm_page->next)
        vm_page->next->prev = vm_page->prev;
    vm_page->prev->next = vm_page->next;
    mm_return_vm_page_to_kernel((void*) vm_page, 1); // cast is not necessary but clarifies the function's generic expectation
    return;
}