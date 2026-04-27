#include "uapi_mm.h"

typedef struct emp_ {
    char name[32];
    uint32_t emp_id;
} emp_t;

typedef struct student_ {
    char name[32];
    uint32_t marks_phy;
    uint32_t marks_chem;
    uint32_t marks_maths;
    struct student_* next;
} student_t;

int
main(int argc, char** argv) {
    
    mm_init();
    
    mm_print_registered_page_families();
    
    MM_REG_STRUCT(emp_t);
    MM_REG_STRUCT(student_t);

    mm_print_registered_page_families();
    
    // test
    lookup_page_family_by_name("emp_t");
    lookup_page_family_by_name("student_t");
    lookup_page_family_by_name("emptyyy");
    
    return 0;
    
}