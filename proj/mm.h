typedef struct vm_page_family_ {

    char struct_name[MM_MAX_STRUCT_NAME];
    uint32_t struct_size;

} vm_page_family_t;


typedef struct vm_page_for_families_ {

    struct vm_page_for_families_* next;
    vm_page_family_t vm_page_family[0];

} vm_page_for_families_t;


/* (Total system page size - size of next pointer) / (size of one page family entry)*/
#define MAX_FAMILIES_PER_VM_PAGE    \
    (SYSTEM_PAGE_SIZE - sizeof(vm_page_for_families_t *)) / (sizeof(vm_page_family_t))



/* 
Looping macro to iterate over all page families (vm_page_family_t) objects
stored in a VM page (vm_page_for_families_t) in bottom-up fashion
*/
#define ITERATE_PAGE_FAMILIES_BEGIN(vm_page_for_families_ptr, curr) \
// {
//     vm_page_family_t* curr = vm_page_for_families_ptr->vm_page_family[0];
//     uint32_t in_page_family_count = 0;
    
//     for(; curr->struct_size != 0; vm_page_for_families_ptr = curr) {
//         ++curr;
//         ++in_page_family_count;

//         if(in_page_family_count >= MAX_FAMILIES_PER_VM_PAGE){
//             break;
//        }


/*  MORE EFFICIENT ITERATIVE MACRO:
    for (init the curr to the first vm_page_family_t in the first vm page;
        check if vm_page_family_t exist by checking its size  && check if curr has iterated all the family entries;
        increment the curr ptr towards next family entry, also increment the in_page_family counter)
*/
{                                                                                                   \
    for(vm_page_family_t* curr = (vm_page_family_t*)vm_page_for_families_ptr->vm_page_family[0];    \
        curr->struct_size && in_page_family_count < MAX_FAMILIES_PER_VM_PAGE;                       \
        curr++, in_page_family_count++) {                                                           \

#define ITERATE_PAGE_FAMILIES_END(vm_page_for_families_ptr, curr)}} \