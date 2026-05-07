#include <stdlib.h>
#include "glthread.h"

/*

    base_glthread is acting as a sentinel/head node, not as an embedded node inside a 
    user data structure.

*/

void
init_glthread(glthread_t* glthread){
    glthread->left = NULL;
    glthread->right = NULL;
}

void 
glthread_add_next(glthread_t* curr_glthread, glthread_t* new_glthread){
    if(curr_glthread->right == NULL){
        curr_glthread->right = new_glthread;
        new_glthread->left = curr_glthread;        
        return;
    }

    glthread_t* tmp = curr_glthread->right;
    tmp->left = new_glthread;
    new_glthread->right = tmp;
    new_glthread->left = curr_glthread;
    curr_glthread->right = new_glthread;

    return;
}   

void 
glthread_add_before(glthread_t* curr_glthread, glthread_t* new_glthread){
    if(curr_glthread->left == NULL){
        new_glthread->right = curr_glthread;
        curr_glthread->left = new_glthread;

        return;
    }

    glthread_t* tmp = curr_glthread->left;
    tmp->right = new_glthread;
    new_glthread->left = tmp;
    new_glthread->right = curr_glthread;
    curr_glthread->left = new_glthread;

    return;
}

/*
    Responsibility:
    To detach the given node from the thrad, as de-allocation isn't its responsibilty as,
    logically that memory, can be allocated again...maybe by some other struct family or be involved in
    block merging.

    Moreover calling free here might corrupt the struct memory as unlike the traditional DLL node,
    the glthread_t* represents just a part of the memory of the node here.
*/
void
remove_glthread(glthread_t* curr_glthread){
    if(curr_glthread->left == NULL){
        if(curr_glthread->right != NULL){
            curr_glthread->right->left = NULL;
            curr_glthread->right = NULL;
            return;
        }
        return; // because we want to keep atleast one meta block entry in a VM page
    }

    if(curr_glthread->right == NULL){ //node at the end of list -> at higher address
        curr_glthread->left->right = NULL;
        curr_glthread->left = NULL;
        return;
    }

    curr_glthread->left->right = curr_glthread->right;
    curr_glthread->right->left = curr_glthread->left;
    curr_glthread->left = NULL;
    curr_glthread->right = NULL;
}

void
delete_glthread_list(glthread_t* base_glthread){ // points to right before the very first node (??)
    glthread_t* glthreadptr = NULL;
    ITERATE_GLTHREAD_BEGIN(base_glthread, glthreadptr){
        remove_glthread(glthreadptr);
    }ITERATE_GLTHREAD_END(base_glthread, glthreadptr);
}

void
glthread_add_last(glthread_t* base_glthread, glthread_t* new_glthread){

    glthread_t* glthreadptr = NULL;
    glthread_t* prevglthreadptr = NULL;

    ITERATE_GLTHREAD_BEGIN(base_glthread, glthreadptr){
        prevglthreadptr = glthreadptr;
    }ITERATE_GLTHREAD_END(base_glthread, glthreadptr);

    if(prevglthreadptr != NULL)
        glthread_add_next(prevglthreadptr, new_glthread);
    else    // no node present, this is the first node to be added
        glthread_add_next(base_glthread, new_glthread);
}

unsigned int
get_glthread_list_count(glthread_t* base_glthread){
    
    unsigned int node_count = 0;
    glthread_t* glthreadptr = NULL;

    ITERATE_GLTHREAD_BEGIN(base_glthread, glthreadptr){
        node_count++;
    } ITERATE_GLTHREAD_END(base_glthread, glthreadptr);

    return node_count;
}


/*
Comparator's possible vals:

    int (*comp_fn)(void *, void *)

   -1  => first object should come before second
    0  => equal
    1  => first object should come after second
*/
void
glthread_priority_insert(glthread_t *base_glthread, 
                         glthread_t *glthread,
                         int (*comp_fn)(void *, void *),
                         int offset){
    
    // Init the glthread
    init_glthread(glthread);

    // CASE: ZERO NODE If list is empty, this sis the first node, insert directly
    if(IS_GLTHREAD_LIST_EMPTY(base_glthread)){
        glthread_add_next(base_glthread, glthread);
        return;
    }

    // CASE: ONLY ONE NODE i.e. just one data node present right after the 
    // sentinel node, base_glthread
    if(base_glthread->right && 
       base_glthread->right->right == NULL) {
        if(comp_fn(GLTHREAD_GET_USER_DATA_FROM_OFFSET(base_glthread->right, offset),
            GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthread, offset)) == -1){ // base_glthread->right should be before the new node
                glthread_add_next(base_glthread->right, glthread);
        }
        else { // new node should be the one, right next to the sentinel node
                glthread_add_next(base_glthread, glthread);
        }

        return;
    }

    // CASE: MORE THAN JUST ONE NODE
    // sub-case: needs to be inserted right after the base/sentinel node
    if(comp_fn(GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthread, offset),
        GLTHREAD_GET_USER_DATA_FROM_OFFSET(base_glthread->right, offset)) == -1){ 
            glthread_add_next(base_glthread->right, glthread);
        
        return;
    }
    
    // sub-case: needs to be inserted in-between of the list somewhere
    glthread_t* curr = NULL;
    glthread_t* prev = NULL;

    ITERATE_GLTHREAD_BEGIN(base_glthread, curr){
        if(comp_fn(GLTHREAD_GET_USER_DATA_FROM_OFFSET(glthread, offset),
                    GLTHREAD_GET_USER_DATA_FROM_OFFSET(curr, offset)) != -1){ // i.e. needs to be inserted somewhere after current node
            prev = curr;
            continue;
        }

        glthread_add_next(curr, glthread);
        return;

    }ITERATE_GLTHREAD_END(base_glthread, curr)

    // sub-case: needs to be inserted at the END Of List
    glthread_add_next(prev, glthread);
}


#if 0

void*
gl_thread_search(glthread_t* base_glthread,
                void*(*thread_to_struct_fn)(glthread_t*),
                void* key,
                int(*comparision_fn)(void*, void*)){
                    
    return NULL;
}

#endif