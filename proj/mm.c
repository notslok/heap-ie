#include <stdio.h>
#include <memory.h>
#include <unistd.h>     /* for getpagesize() */
#include <sys/mman.h>   /* for using mmap() */

static size_t SYSTEM_PAGE_SIZE = 0;

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


/* API  to return a page back to the kernel */
static void
mm_return_vm_page_to_kernel (void* vm_page, int units) {

    // return memory back to kernel using munmap() syscall
    if(munmap(vm_page, units * SYSTEM_PAGE_SIZE)) { // returns 0 on success
        printf("Error: VM page de-allocation failed\n");
    }
}


int
main(int argc, char **argv){
    mm_init();

    printf("VM Page Size = %lu\n", SYSTEM_PAGE_SIZE);
    void *addr1 = mm_get_new_vm_page_from_kernel(1);
    void *addr2 = mm_get_new_vm_page_from_kernel(1);
    printf("page 1 addr = %p,, page 2 addr = %p\n", addr1, addr2); // in c %p specifier is used to format pointer holding memory address
    
    // If page 1 and page 2 are allocation in contiguous fashion this diff. should
    // be equal to the page size used by the system
    printf("difference in page 1 addr and page 2 addr: %ld\n", (long int)addr1 - (long int)addr2);
    return 0;
}