#ifndef __UAPI_MM__
#define __UAPI_MM__

#include <stdint.h>

/* LMM Initialization Function */
void
mm_init();

/* new page family entry init */
void
mm_instantiate_new_page_family (char*, uint32_t); 


/* Registration Function */
// CONCEPTUAL NOTE: #struct_name does the string substitution
#define MM_REG_STRUCT(struct_name)  \
    (mm_instantiate_new_page_family(#struct_name, sizeof(struct_name)))

#endif /* __UAPI_MM__*/