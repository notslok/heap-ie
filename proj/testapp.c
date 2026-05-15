#include "uapi_mm.h"
#include <stdio.h>

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
    
    // memory alloc reuests w.r.t emp_t and student_t objects
    emp_t *emp1 = xcalloc("emp_t", 1);
    emp_t *emp2 = xcalloc("emp_t", 1);
    student_t *stud1 = xcalloc("student_t", 1);
    student_t *stud2 = xcalloc("student_t", 1);
    student_t *stud3 = xcalloc("student_t", 1);
    
    mm_print_registered_page_families();
    scanf("\n");
    // mm_print_memory_usage(0);   // TODO
    // mm_print_block_usage();     // TODO

    // mm_print_registered_page_families();
    
    // test
    // lookup_page_family_by_name("emp_t");
    // lookup_page_family_by_name("student_t");
    // lookup_page_family_by_name("emptyyy");

    XFREE(emp1);
    XFREE(emp2);
    
    XFREE(stud1);
    XFREE(stud2);
    XFREE(stud3);

    return 0;
    
}