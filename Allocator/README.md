# Advanced Memory Allocator

## Part 1 - Understand mmap

In your allocator, you must have some logic for when to use brk versus mmap. At the end of the day, you are accessing memory with both scenarios, but you will need to decide based on how much memory a user requests whether to extend the data segment(and pay for a context switch), or to make a larger memory mapping (with some initial overhead and risk of fragmentation in return for speed later).

## Part 2 - Multi-threading

In a multi-threaded environment, we cannot simply make requests to our 'malloc' and 'free' functions. We could certainly get an unlucky interleaving of instructions. Lucklily, we have mutexes to enforce mutual exlusion, and thus create a critical section that is executed sequentially when we are performing operations over shared resources.

## Part 3 - Multiple free lists

Depending on the number of cores a computer has, it is often optimal to have seperate free lists per cpu. Then, as separate threads run, they can be associated with locks on a per-cpu basis that the thread is running on. Investigate `man sched_getcpu`.

## Part 4 - Understanding a pool allocator

A pool allocator, allocates a set amount of memory optimized for some workload. A common way to have a 'pool' of allocated blocks would be to have a fixed set of blocks.

<img align="middle" src="https://3.bp.blogspot.com/-bzESt0SePCg/VriOTcYStBI/AAAAAAAAAIQ/wxPV4jCV644/s640/Segregated%2BFree%2BList.png" width="300px">

Here a free list is initialized with several blocks that are all the same size, one after the other.

Another way is to use a 'buddy system' that splits the blocks up when allocated.

<img align="middle" src="https://3.bp.blogspot.com/-DmVK0LYjtKE/VrilzVwb0_I/AAAAAAAAAIg/A0IQ5ifTF7w/s1600/Buddy-Memory-System.gif" width ="300px">

- A block is allocated, say of 4kb
- Then it is split into chunks of 2kb, 1kb, 1kb
- And then split again chunks of 2kb, 1kb, 512b,512b
- etc.

The proportions of how things are split can be determined if you understand how data is allocated for your application. In this case, you will not know my test cases, so you can try a simple splitting scheme like the buddy system or even have all of the blocks be the same size. The idea is that one big allocation takes place when the program starts, and then allocations take place later in the program as needed.
