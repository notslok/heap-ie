# Memory API

Two types of memory that are allocated: \

1) **STACK** memory, allocations and deallocations of it are managed by the compiler for the programmer, also known as **automatic** memory. \

Eg: int x; \

compilaer takes care of allocation and deallocation of the memory. \

2) **HEAP** memory, where all allocations and deallocations are explicitly programmer's responsibility. \

Eg: int* x = (int *)malloc(sizeof(int));

**NOTE:** In this example both stack and heap alocation takes place. First compiler makes room for [int* x] on the stack. Followed by heap allocation done by [malloc()]
which returns address of an integer on success OR NULL on failure, which is AGAIN stored on the STACK. \


## malloc()

-> You simply pass it the size of the room you want to acquire on the heap and, it returns the pointer to the newly-allocated space on success OR NULL on failure. \

**Function signature:** *void *malloc(size_t size);* \

-> In the following example: \

    double* d = (double*)malloc(sizeof(double)) \

User attempts to reserve space for double precision floating point number on the heap. The *sizeof()* gets the required size during compile-time and provides it to *malloc()*. Due to this behaviour of *sizeof()*, its often thought of as *operator and not a function call*. \

-> In case of allocating space for string like variable, following thumb-rule is followed to accomodate string with end-of-string character. \

**malloc(strlen(s) + 1)** --> adds 1 to make room for end-of-string character.

-> Explicitly casting the void pointer returned by malloc, is just for signalling the compiler and the other programmers reassurance about the intent of the malloc.


## free()

-> To free the allocated heap memory that is no longer in use, simply call **free()**. \

*int* x = malloc(10 * sizeof(int));* \
... \
*free(x)* \

-> free() simply take the pointer returned by malloc() as the only argument. If you notice, it doesnt accept the size of allocated space, thats because its the responsibilty of the underlying memory-allocation library to keep track of that. \

## Common Errors:

-> Correct memory management has been such a problem that newer, languages ahve support for **automatic memory management**. In such languages one you allocate dynamic memory the **garbage-collector** takes up the responsibility of clean-up of un-used resources. \

### Forgetting to allocate memory [Segmentation fault]:

```
char *src = "hello";
char *dst; // oops! unallocated
strcpy(dst, src); // segfault and die
```
It will likely lead to **Segmentation fault**. \

**FIX:** \

```
char* src = "hello";
char* dst = (char*)malloc(sizeof(src) + 1);
strcpy(dst, src);
```


### Not allocating enough Memory [Buffer Overflow]:

-> Leads to common error a.k.a **buffer overflow**. \

```
char *src = "hello";
char *dst = (char *) malloc(strlen(src)); // too small! No space for end-of-string char
strcpy(dst, src); // work properly
```

-> Occurence of error is dependent on the implementation of the malloc library, as in some cases the malloca might allocate extra space before hand to avoid overwriting illegal memory. \


### Forgetting to Initialize Allocated Memory [uninitialized Read]

-> Not a good practice as the newly allocated memory can have any sort of **garbage data**, leading to *uninitialized read* which may be harmful.


### Forgetting to Free Memory [memory leak]

-> Leads to the issue of **memory leak**, in long running application like OS its very dangerous as the system may eventually run out of memory, leaving reboot as the only option. \

-> Even though in shourt lifetime processes its ok to not explicitly free up the memory as once the process terminates the OS will reclaim the entire address space, but its a BAD PRACTICE. \

-> Memory leak remains a problem evenin modern languages with Garbage Collection.


### Freeing Memory Before You are Done with it [Danglin Pointer]

-> Such a mistake cause **Dangling pointer** and can crash the program OR worse overwrite valid memory.

### Freeing memory repeatedly [Double Free]

-> If a program tries to free amemory more than once, this is known as **Double Free**. Crash is the most common outcome. \


### Calling free() Incorrectly [Invalid Frees]

-> When you pass some value other than the pointer retured from a malloc call, it could do bad things like attempting to free invalid memory.


### Tooling [meory safety]:

-> Tools like **purify** and **valgrind** exist to deal with above memory-related problems.


## ASIDE: WHY NO MEMORY IS LEAKED ONCE YOUR PROCESS EXITS?
```
When you write a short-lived program, you might allocate some space
using malloc(). The program runs and is about to complete: is there
need to call free() a bunch of times just before exiting? While it seems
wrong not to, no memory will be “lost” in any real sense. The reason is
simple: there are really two levels of memory management in the system.

The first level of memory management is performed by the OS, which hands out memory to processes when they run, and takes it back when processes exit (or otherwise die). 

The second level of management is within each process, for example within the heap when you call malloc() and free(). Even if you fail to call free() (and thus leak
memory in the heap), the operating systemwill reclaim all the memory of the process (including those pages for code, stack, and, as relevant here, heap) when the program is finished running. No matter what the state
of your heap in your address space, the OS takes back all of those pages when the process dies, thus ensuring that no memory is lost despite the fact that you didn’t free it.


When you write a long-running server (such as a web server or database management system, which never exit), leaked memory is a much bigger issue, and will eventually lead to a crash when the application runs out of memory
```


 
 ## Underlying OS support:

 -> malloc() and free() themselves are not system calls, rather they are a memory management library API calls which manages space within your **virtual address space**.

 -> Internally mallo() and free() calls system calls like **brk** and **sbrk** which are responsible for changing the program's **break (i.e. heap bopundary line)** by shrinking or expanding it. (check man pages of brk and sbrk !) \

 <pic> \

 -> One can also obtain memory from OS via mmap()call, on passing certain args it provides a (anonymous)memory region withing our address apce which isn't related to any particular file descriptor but rather a *swap space*. \


 ## Other Calls

-> **calloc()** allocates memory and also zeroes it before returning; this prevents some errors where you assume that memory is zeroed and forget to initialize it yourself. \

-> **realloc()** can also be useful, when you’ve allocated space for something (say, an array), and then need to add something to it: realloc() makes a new larger region of memory,copies the old region into it, and returns the pointer to the new region. \


## Aside:

-> Read the C book [KR88] and Stevens [SR05] (Chapter 7)for more information.


## p-set: