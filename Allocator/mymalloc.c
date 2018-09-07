#define _GNU_SOURCE
#include <stdio.h> // Any other headers we need here
#include <malloc.h> // We bring in the old malloc function
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sched.h>
#include <semaphore.h>

// Globals:
#define PAGESIZE  sysconf(_SC_PAGESIZE)// Page size = 4KB

sem_t mutex[64];

pthread_mutex_t cpu_lock;

// The Node in the linked list
struct block
{
	size_t size;
	int is_free;
	struct block *next;
};
static struct block *heads[64];
static struct block *tails[64];

// Function Definations
void* mymalloc(size_t);
void free(void* someBlock);
struct block* getFreeBlock(size_t size, int cpu_id);
void malloc_init(int cpu_id);
struct block* split(struct block*, size_t size);
void insert(struct block*, int cpu_id);

// this mmaps a 256KB block and divives it into 256 equal 1024B pieces.
// So initially we have 256 1024B memory blocks. If more memory is needed, it
// will be allocated appropriately(using sbrk() or mmap())
// All the blocks are sorted using insertion sort, to ensure best fit, after
// init.
void malloc_init(int cpu_id)
{
		void *block;
		struct block *header, *temp;
		size_t total_size;
		int i;
		// 256KB + headers for each of the 256 blocks
		total_size = (256*1024)+(sizeof(struct block)*256);
		block = mmap(
			NULL,
			total_size,
			PROT_READ|PROT_WRITE|PROT_EXEC,
			MAP_ANON|MAP_PRIVATE,
			-1,
			0
		);
		if(block == MAP_FAILED)
		{
			printf("Malloc Failed\n");
			return;
		}
		header = block;
		header->next=NULL;
		header->size = 256*1024;
		header->is_free=1;
		heads[cpu_id] = header;
		tails[cpu_id] = header;
		temp = header;
		// 255 because block ^ is the 256th block.
		for(i=0;i<255;i++)
		{
			temp = split(temp, 1024);
			temp->is_free=1;
			tails[cpu_id]->next = temp;
			tails[cpu_id] = temp;
		}
}

// This shrinks the given block b to size, and returns the pointer to
// the next block with the remaining memory.
struct block* split(struct block* b, size_t size)
{
		void *mem_block = (void*)(b + 1);
		struct block *newptr = (struct block*) ((unsigned long)mem_block + size);
		newptr->size = b->size - size;
		b->size = size;
		b->is_free = 1;
		return newptr;
}

// Insterts the given block before the @param: beforeThis, on the @param:cpu_id
// free list. This is primarily a insert before 'x' in a linked list.
void insertBefore(struct block* toInsert, struct block* beforeThis, int cpu_id)
{
	struct block *ptr;
	ptr = heads[cpu_id];
	if(beforeThis == heads[cpu_id])
	{
		toInsert->next = heads[cpu_id];
		heads[cpu_id] = toInsert;
		return;
	}
	if(!beforeThis)
	{
		tails[cpu_id]->next = toInsert;
		tails[cpu_id] = toInsert;
		return;
	}
	while((ptr->next) != beforeThis)
		ptr = ptr->next;
	toInsert->next = ptr->next;
	ptr->next=toInsert;
}

// inserts block b in the appropriate position in the list, sorted by the
// size of the blocks.
void insert(struct block* b, int cpu_id)
{
		struct block* temp;
		temp = heads[cpu_id];
		while(temp && temp->size < b->size)
		{
				temp = temp->next;
		}
		insertBefore(b, temp, cpu_id);
}

// Allocates a block of @param: size and returns the said block.
void* mymalloc(size_t size)
{
		size_t total_size; //total_size = size + header_size
		void* block;
		struct block *header, *temp;
		if(!size)
			return NULL;
		int current_cpu = sched_getcpu();

		if(!heads[current_cpu])
		{
			sem_init(&mutex[current_cpu], 0, 1);
			sem_wait(&mutex[current_cpu]);
			malloc_init(current_cpu);
			sem_post(&mutex[current_cpu]);
		}
		sem_wait(&mutex[current_cpu]);
		header = getFreeBlock(size, current_cpu);
		if(header)
		{
			header->is_free = 0;
			sem_post(&mutex[current_cpu]);
			//This is the first byte of the actual memory block.
			return (void*)(header+1);
		}
		// We have not found a block of the required size.
		total_size = sizeof(struct block) + size;
		if(total_size > PAGESIZE)
		{
			block = mmap(
				NULL,
				total_size,
				PROT_READ|PROT_WRITE|PROT_EXEC,
				MAP_ANON|MAP_PRIVATE,
				-1,
				0
			);
			if(block == MAP_FAILED)
			{
				printf("Malloc Failed\n");
				sem_post(&mutex[current_cpu]);
				return NULL;
			}
		}
		else
		{
			block = sbrk(total_size);
			if(block == (void*)-1)
			{
				sem_post(&mutex[current_cpu]);
				printf("Malloc Failed!\n");
				return NULL;
			}
		}
		header = block;
		header->size = size;
		header->is_free = 0;
		header->next = NULL;
		insert(header, current_cpu);
		sem_post(&mutex[current_cpu]);
		return (void*)(header+1); //header+1 = memory block
}

// Returns a free block of the specified @param: size in the @param: cpu_id
// free-list.
struct block* getFreeBlock(size_t size, int cpu_id)
{
		struct block *current = heads[cpu_id];
		while(current)
		{
			if(current->is_free)
			{
				if(current->size >= size)
					return current;
			}
			current = current->next;
		}
		return NULL;
}

// Sets the give @param: block to free to that it can be assigned to another
// requesting process.
void myfree(void *block)
{
		struct block *header;
		if(!block)
			return;
		header = (struct block*)(block - 1);
		header->is_free = 1;
}
