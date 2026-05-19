# The Abstraction: Address Spaces

## Legacy Systems:

-> OS was more of a routine/library  that was loaded into one end of the physical memory and rest of the physical memory was dedicated for running a program(aka process) \

<pic>

## Multiprogramming and Time Sharing:

-> For the sake of better **utilization** and **efficiency** of expensive computers resulted in sharing of maching resources and led to an era of **multiprogramming**. \

-> As demand grew and batch computing was no longer enough to meet the user needs, **time sharing** was born. It aimed to solve the **interactivity** issue of the machines. In trivial fashion, in orderly manner one the process would be given the entire resource for a given time-slice followed by switching with another process. **BUT** as the size of memory grew this sytem beacame slower. \

-> Soon enough due to the nature of the bare layout of processes in the memory (potentially from different users too), demands for **protection** (of one process's memory from another in. OS itself) arose. So that no two diffent processes loaded into the memory could read/write the memory content of other process \

<pic> 

## The Address:

-> To address all the above issue of **efficiency**, **utilization**, **interactivity** and **protection**. People came up with the abstraction called: **Address Space**. \

-> Address Space of a process contains all of the memory state of the running program i.e.
**The Code** (the instructions), **a stack** (to keep track of function calls and local variables) and **a heap** (used for dynamically-allocated memory and user-managed memory). \

<pic> \

-> Code is static in size hence is kept in the beginning of the virtual memory/ address space layout of a process. Followed by heap and stack(starting at the very end of the address space). Giving each other the room to grow dynamically. \

-> For example, whenever there is a malloc() call made the heap grows downwards and whenever a new function call is made the stack grows upwards in the allocated address space. \

-> **NOTE: ** (in further sections we will see that) When multiple threads co-exist in an address space, there is no nice way of dividing the address spacwe into code, heap and stack.

-> So, the *Address Space* is merely an abstraction provided by the OS to the user as the actual program lies somewhere else in some arbitrary physical address(es). \

**Question :** *How does  OS build an abstraction of a private and potentially large address space for  multiple running processes on top of a single physical process?* \
\
**Answer :** *OS facilitates this by virtualizing memory provided by the machine, in that way the process thinks thatit has 16 KB exclusivaly to itself, whereas OS takes help of hardware to traslate the virtual address to the actual arbitrary physical addresses where the content of the process actually reside* \


## Goals:

-> To virtualize memory in a desirable manner we need certain goals to guide us:

1) **Transparency:** OS should implement virtual memory in a way that is invisible to the running program. \

2) **Efficiency:** In terms of *time* (i.e. not making programs run slower) and *space* (i.e. not needing too much memory for the structures supporting virtualization)

3) **Protection:** Of one process to another as well as the OS from other process(es). So that when one process performs **a load, a store or an instruction fetch** it should not be able to affect the memory content of other processes or the OS itself. This enables us to deliver the property of **isolation**.

## Summary:

*Each virtual address space contains all of a program’s instructions and data, which can be referenced by the program via virtual addresses. The OS, with some serious hardware help, will take each of these virtual memory references and turn them into physical addresses* \


# p-set:

