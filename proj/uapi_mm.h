#ifndef __UAPI_MM__
#define __UAPI_MM__

#include <stdint.h>

void*
xcalloc(char* struct_name, int units);

void
xfree(void *app_ptr);

#define XCALLOC(units, struct_name) \
    (xcalloc(#struct_name, units))      // the # operator is the stringizing operator in the C preprocessor.
                                        // It converts the macro argument into a string literal.

#define XFREE(ptr)  \
   xfree(ptr)

/* LMM Initialization Function */
void
mm_init();

/* new page family entry init */
// void
// mm_instantiate_new_page_family (char*, uint32_t); ---> MUST BE PRIVATE API


/* Prints out all the registered family name and corresponding size */
void
mm_print_registered_page_families();


/* Registration Function */
// CONCEPTUAL NOTE: #struct_name does the string substitution
#define MM_REG_STRUCT(struct_name)  \
    (mm_instantiate_new_page_family(#struct_name, sizeof(struct_name)))

/* Tests */
void 
mm_print_memory_usage(char* struct_name);   // TODO

void
mm_print_block_usage();     // TODO

#endif /* __UAPI_MM__*/