#include "mm.h"
#include <stdio.h>
#include <memory.h>
#include <unistd.h>     /* for getpagesize() */
#include <sys/mman.h>   /* for using mmap() */
#include <assert.h>
#include <stdint.h>


static size_t SYSTEM_PAGE_SIZE = 0;
static vm_page_for_families_t* first_vm_page_for_families = NULL;

void mm_init() {
    SYSTEM_PAGE_SIZE = getpagesize();
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

        return;
    // }
}


/* API  to return a page back to the kernel */
static void
mm_return_vm_page_to_kernel (void* vm_page, int units) {

    // return memory back to kernel using munmap() syscall
    if(munmap(vm_page, units * SYSTEM_PAGE_SIZE)) { // returns 0 on success
        printf("Error: VM page de-allocation failed\n");
    }
}