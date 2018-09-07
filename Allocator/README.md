# More Advanced Memory Allocator

# Due July 25 at 11:59pm

For this assignment, you will be writing another memory allocator and implementing your own malloc and free. The good news is you have already written one. **(You may build or reuse pieces of your previous assignment.)** Now we are going to write a slightly more realistic memory allocator given a few constraints. The goal of this is to get performance as close as possible to that of the actual malloc.  

## Weird Behavior While Testing:
  - I tested my code with original stress test that was posted, and it worked. However, the new test that was posted on 07/27/2018, gave me seg-faults.
  - I have provided the original stress test as test5 in the tests folder. If I am allowed to, may I please request the graders to test my code the aforementioned test. 
    - I understand that asking my code to be tested with a specific test destroys the purpose of testing in the first place, but I really need all the points that I can get.

## Part 1 - Understand mmap

The first thing to know is that brk (or the sbrk() function) is not the only way to request memory. Unix provides the mmap system call as well, which allows us to directly map into memory. There is some overhead in creating this mapping, but once it is created, there is no need for a context switch from the kernel, because the memory is directly mapped to where it needs to go. That is fantastic news for us! The only other major constraint is that allocations with mmap must be made in multiples the size of a page(If you specify this incorrctly, mmap will round up). This means if our page size is 4kb (4096 bytes), and we only use 12 bytes in our program, we have some waste!

I have provided a very brief set of slides here with some notes on mmap: https://docs.google.com/presentation/d/10dSP5JICTjqqoo6l8XN6tGV1whlfFPUOGruOYafQzsQ/edit?usp=sharing

**What you must do** In your allocator, you must have some logic for when to use brk versus mmap. At the end of the day, you are accessing memory with both scenarios, but you will need to decide based on how much memory a user requests whether to extend the data segment(and pay for a context switch), or to make a larger memory mapping (with some initial overhead and risk of fragmentation in return for speed later).

## Part 2 - Multi-threading

In a multi-threaded environment, we cannot simply make requests to our 'malloc' and 'free' functions. We could certainly get an unlucky interleaving of instructions. Lucklily, we have mutexes to enforce mutual exlusion, and thus create a critical section that is executed sequentially when we are performing operations over shared resources.

**What you must do** Have some locking mechanism that takes place whenever their is an allocation or free.

## Part 3 - Multiple free lists

Depending on the number of cores a computer has, it is often optimal to have seperate free lists per cpu. Then, as separate threads run, they can be associated with locks on a per-cpu basis that the thread is running on. Investigate `man sched_getcpu`. You might additionally investigate `nproc`, `lscpu`,`man get_nprocs()`, and `man get_nprocs_conf()` to figure out where these commands may be useful.

**What you must do** Have multiple free lists created based on the number of cpus. Make sure to update your locking mechanism to make sure you have enough locks per cpu.

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

**What you must do** Allocate a pool of at least 256kb of memory total per free list and split it appropriately.

## Rubric

- 100% for a working Memory Allocator
  - You should implement each of the 'what you must do' sections.
    - At a high level, I want to see you have an allocator that works on multi-threaded programs(i.e. locks), has some pooling strategy, and has a mechanism for multiple free lists.
  - You should avoid memory leaks
  - You should avoid data races
  - You should test your allocator on some threaded application.
  - You should use both mmap and sbrk.
  - You should update your first-fit to a 'best-fit' strategy.
  - It will be a good idea to write a small multithreaded application to test this out on.
    - Including those unit tests in your repo is encouraged!

I am going to test this by running your application against malloc on a few multi-threaded programs. An example workload might be loading several images and associated text captions in memory on different threads, putting them into a buffer, and then playing them sequentially.

# Resources to help
- https://blog.molecular-matters.com/2012/09/17/memory-allocation-strategies-a-pool-allocator/ (Pool Allocator overview)

### Glossary and Tips
1. Commit your code changes to the repository often, this is best practice.
2. Do not commit your .o file or your executable file to the repository, this is considered bad practice!
3. On Functionality/Style
	1. You should have comments in your code to explain what is going on, and use good style (80 characters per line maximum for example, functions that are less than 50 lines, well named variables, well named functions etc.).

# Going Further

- Investigate the 'buddy allocator' and 'arena allocators'

# Feedback Loop

(An optional task that will reinforce your learning throughout the semester)

- A nice survey of dynamic memory allocation strategies: http://www.cs.northwestern.edu/~pdinda/ics-s05/doc/dsa.pdf
