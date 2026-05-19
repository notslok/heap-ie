#ifndef __GLUETHREAD_H__
#define __GLUETHREAD_H__

typedef struct glthread_{
    struct glthread_* left;
    struct glthread_* right;
} glthread_t;

void 
glthread_add_next(glthread_t* base_glthread, glthread_t* new_glthread);

void 
glthread_add_before(glthread_t* base_glthread, glthread_t* new_glthread);

void
glthread_add_last(glthread_t* base_glthread, glthread_t* new_glthread);

void
remove_glthread(glthread_t* glthread);

void
init_glthread(glthread_t* glthread);

#define IS_GLTHREAD_LIST_EMPTY(glthreadptr) \
    ((glthreadptr)->right == 0 && (glthreadptr)->left == 0)


/*
The macro performs a pointer subtraction based on memory offset:
-> &(((structure_name *)0)->field_name): This trick pretends there is a struct at memory address 0 and 
    finds the address of field_name. This effectively gives you the offset (number of bytes) from the 
    start of the struct to that field.

-> (char *)(glthreadptr): Converts your current pointer to a byte-address.

-> Current Pointer - Offset: By subtracting the offset from your current address, 
   you land exactly at the memory address where the Player struct begins.
*/
#define GLTHREAD_TO_STRUCT(fn_name, structure_name, field_name, glthreadptr)    \
    static inline structure_name* fn_name(glthread_t* glthreadptr) {            \
        return (structure_name*)((char*)(glthreadptr) - (char*)&(((structure_name*)0)->field_name));    \
    }



/* Delete Safe Loop */
#define BASE(glthreadptr)   ((glthreadptr)->right)

#define ITERATE_GLTHREAD_BEGIN(glthreadptrstart, glthreadptr)   \
{                                                               \
    glthread_t* _glthread_ptr = NULL;                           \
    glthreadptr = BASE(glthreadptrstart);                       \
    for(; glthreadptr!=NULL; glthreadptr = _glthread_ptr){      \
        _glthread_ptr = (glthreadptr)->right;      

#define ITERATE_GLTHREAD_END(glthreadptrstart, glthreadptr)   \
    }}


/*
    EXAMPLE:

    Address: 1004 | [ glthread_t node ]  <-- glthreadptr                <<HIGHER ADDRESS>>
    Address: 1000 | [ int age ]          (Start of Struct)              <<LOWER ADDRESS>>
    
    glthreadptr = 1004
    offset = 4 (because node starts 4 bytes in)
    Result = 1004 - 4 = 1000 (The starting address of the struct)
*/
#define GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthreadptr, offset) \
    (void*)((char*)(glthreadptr) - offset)


/* Utils for Intrusive LL */
void
delete_glthread_list(glthread_t* base_glthread);

unsigned int
get_glthread_list_count(glthread_t* base_glthread);

void
glthread_priority_insert(glthread_t* base_glthread,     // ref to base gl node
                         glthread_t* glthread,          // ref to gl node to be inserted
                         int (*comp_fn)(void*, void*),  // fn ptr to comparator for priority checking
                         int offset);                   // offset of the glthread_t ptr

#if 0
void*
gl_thread_search(glthread_t* base_glthread,
                 void*(*thread_to_struct_fn)(glthread_t*),
                 void* key,
                 int (*comparision_fn)(void*, void*));
#endif

#endif /* __GLUETHREAD_H__ */