/*
 * =====================================================================================
 *
 *	Filename:  		sma.c
 *
 *  Description:	Base code for Assignment 3 for ECSE-427 / COMP-310
 *
 *  Version:  		1.0
 *  Created:  		6/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *      
 *  Instructions:   Please address all the "TODO"s in the code below and modify 
 * 					them accordingly. Feel free to modify the "PRIVATE" functions.
 * 					Don't modify the "PUBLIC" functions (except the TODO part), unless
 * 					you find a bug! Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
#include "sma.h" // Please add any libraries you plan to use inside this file

/* Definitions*/
#define MAX_TOP_FREE (128 * 1024) // Max top free block size = 128 Kbytes
//	TODO: Change the Header size if required
#define FREE_BLOCK_HEADER_SIZE sizeof(block_header) // Size of the Header in a free memory block
//	TODO: Add constants here
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef enum //	Policy type definition
{
    WORST,
    NEXT
} Policy;

char *sma_malloc_error;
block_header *freeListHead = NULL;    //	The pointer to the HEAD of the doubly linked free memory list
block_header *freeListTail = NULL;    //	The pointer to the TAIL of the doubly linked free memory list
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;      //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;         //	Current Policy
//	TODO: Add any global variables here

/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: sma_malloc
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates a memory block of input size from the heap, and returns a 
 * 					pointer pointing to it. Returns NULL if failed and sets a global error.
 */
void *sma_malloc(int size)
{
    void *pMemory = NULL;

    // Checks if the free list is empty
    if (freeListHead == NULL)
    {
        // Allocate memory by increasing the Program Break
        pMemory = allocate_pBrk(size);
    }
    // If free list is not empty
    else
    {
        // Allocate memory from the free memory list
        pMemory = allocate_freeList(size);

        // If a valid memory could NOT be allocated from the free memory list
        if (pMemory == (void *)-2)
        {
            // Allocate memory by increasing the Program Break
            pMemory = allocate_pBrk(size);
        }
    }

    // Validates memory allocation
    if (pMemory < 0 || pMemory == NULL)
    {
        sma_malloc_error = "Error: Memory allocation failed!";
        return NULL;
    }

    // Updates SMA Info
    totalAllocatedSize += size;

    return pMemory;
}

/*
 *	Funcation Name: sma_free
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Deallocates the memory block pointed by the input pointer
 */
void sma_free(void *ptr)
{
    //	Checks if the ptr is NULL
    block_header *block;
    block = (block_header *)ptr;
    char buffer[60];
    if (ptr == NULL)
    {
        puts("Error: Attempting to free NULL!");
    }
    //	Checks if the ptr is beyond Program Break
    else if (ptr > sbrk(0))
    {
        puts("Error: Attempting to free unallocated space!");
    }
    else
    {
        //	Adds the block to the free memory list
        sprintf(buffer, "Freeing block of %p\n", block);
        puts(buffer);
        add_block_freeList(ptr);
    }
}

/*
 *	Funcation Name: sma_mallopt
 *	Input type:		int
 * 	Output type:	void
 * 	Description:	Specifies the memory allocation policy
 */
void sma_mallopt(int policy)
{
    // Assigns the appropriate Policy
    if (policy == 1)
    {
        currentPolicy = WORST;
    }
    else if (policy == 2)
    {
        currentPolicy = NEXT;
    }
}

/*
 *	Funcation Name: sma_mallinfo
 *	Input type:		void
 * 	Output type:	void
 * 	Description:	Prints statistics about current memory allocation by SMA.
 */
void sma_mallinfo()
{
    //	Finds the largest Contiguous Free Space (should be the largest free block)
    int largestFreeBlock = get_largest_freeBlock();
    char str[60];

    //	Prints the SMA Stats
    sprintf(str, "Total number of bytes allocated: %lu", totalAllocatedSize);
    puts(str);
    sprintf(str, "Total free space: %lu", totalFreeSize);
    puts(str);
    sprintf(str, "Size of largest contigious free space (in bytes): %d", largestFreeBlock);
    puts(str);
    list_details();
}

/*
 *	Funcation Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void *sma_realloc(void *ptr, int size)
{
    // TODO: 	Should be similar to sma_malloc, except you need to check if the pointer address
    //			had been previously allocated.
    // Hint:	Check if you need to expand or contract the memory. If new size is smaller, then
    //			chop off the current allocated memory and add to the free list. If new size is bigger
    //			then check if there is sufficient adjacent free space to expand, otherwise find a new block
    //			like sma_malloc.
    //			Should not accept a NULL pointer, and the size should be greater than 0.
}

/*
 * =====================================================================================
 *	Private Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: allocate_pBrk
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory by increasing the Program Break
 */
void *allocate_pBrk(int size)
{
    void *newBlock = NULL;
    int excessSize = 8 * 1024; // 2KB
    int newSize;
    block_header *new_block;
    puts("Increasing program break --->\n");

    //	TODO: 	Allocate memory by incrementing the Program Break by calling sbrk() or brk()
    //	Hint:	Getting an exact "size" of memory might not be the best idea. Why?
    //			Also, if you are getting a larger memory, you need to put the excess in the free list

    //	Allocates the Memory Block
    newSize = ALIGN(size + FREE_BLOCK_HEADER_SIZE + excessSize);
    new_block = sbrk(newSize);
    new_block->is_free = 0;
    new_block->size = ALIGN(FREE_BLOCK_HEADER_SIZE + size);
    new_block->address = new_block + FREE_BLOCK_HEADER_SIZE;

    newBlock = new_block;
    allocate_block(newBlock, size, excessSize, 0);

    return newBlock;
}

/*
 *	Funcation Name: allocate_freeList
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory from the free memory list
 */
void *allocate_freeList(int size)
{
    void *pMemory = NULL;

    if (currentPolicy == WORST)
    {
        // Allocates memory using Worst Fit Policy
        pMemory = allocate_worst_fit(size);
    }
    else if (currentPolicy == NEXT)
    {
        // Allocates memory using Next Fit Policy
        pMemory = allocate_next_fit(size);
    }
    else
    {
        pMemory = NULL;
    }

    return pMemory;
}

/*
 *	Funcation Name: allocate_worst_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Worst Fit from the free memory list
 */
void *allocate_worst_fit(int size)
{
    void *worstBlock = NULL;
    int excessSize;
    int blockFound = 0;
    block_header *chosen_block, *curr;
    int max_size = 0;
    char buffer[200];

    list_details();
    sleep(1);

    //	TODO: 	Allocate memory by using Worst Fit Policy
    //	Hint:	Start off with the freeListHead and iterate through the entire list to
    //			get the largest block

    curr = freeListHead;
    if (!curr->is_free && !curr->next)
    {
        blockFound = 0;
    }
    else
    {
        while (curr)
        {
            if (!curr->next)
            {
                if (curr->is_free)
                {
                    max_size = curr->size;
                    chosen_block = curr;
                    break;
                }
                else
                {
                    max_size = 0;
                    break;
                }
            }
            if (curr->size > max_size && curr->is_free)
            {
                max_size = curr->size;
                chosen_block = curr;
            }

            curr = curr->next;
        }

        blockFound = (max_size > size) ? 1 : 0;
    }
    // then we jump to allocate_block and then to replace_block_freelist

    chosen_block->size = ALIGN(FREE_BLOCK_HEADER_SIZE + size);
    chosen_block->is_free = 0;

    //	Checks if appropriate block is found.
    if (blockFound)
    {
        //	Allocates the Memory Block
        excessSize = max_size - size - FREE_BLOCK_HEADER_SIZE;
        worstBlock = (void *)chosen_block;
        sprintf(buffer, "Trying to allocate worst block of %p size %d and add free block of size %d", chosen_block, chosen_block->size, excessSize);
        puts(buffer);
        allocate_block(worstBlock, size, excessSize, 1);
    }
    else
    {
        //	Assigns invalid address if appropriate block not found in free list
        puts("NO BLOCK FOUND\n");
        worstBlock = (void *)-2;
    }

    return worstBlock;
}

/*
 *	Funcation Name: allocate_next_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Next Fit from the free memory list
 */
void *allocate_next_fit(int size)
{
    void *nextBlock = NULL;
    int excessSize;
    int blockFound = 0;

    //	TODO: 	Allocate memory by using Next Fit Policy
    //	Hint:	You should use a global pointer to keep track of your last allocated memory address, and
    //			allocate free blocks that come after that address (i.e. on top of it). Once you reach
    //			Program Break, you start from the beginning of your heap, as in with the free block with
    //			the smallest address)

    //	Checks if appropriate found is found.
    if (blockFound)
    {
        //	Allocates the Memory Block
        allocate_block(nextBlock, size, excessSize, 1);
    }
    else
    {
        //	Assigns invalid address if appropriate block not found in free list
        nextBlock = (void *)-2;
    }

    return nextBlock;
}

/*
 *	Funcation Name: allocate_block
 *	Input type:		void*, int, int, int
 * 	Output type:	void
 * 	Description:	Performs routine operations for allocating a memory block
 */
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList)
{
    void *excessFreeBlock; //	pointer for any excess free block
    int addFreeBlock;
    block_header *free_block, *new_block;
    char buffer[60];
    new_block = (block_header *)newBlock;

    // 	Checks if excess free size is big enough to be added to the free memory list
    //	Helps to reduce external fragmentation

    //	TODO: Adjust the condition based on your Head and Tail size (depends on your TAG system)
    //	Hint: Might want to have a minimum size greater than the Head/Tail sizes
    addFreeBlock = excessSize > FREE_BLOCK_HEADER_SIZE;
    sprintf(buffer, "addFreeBlock %d\n", excessSize);
    puts(buffer);

    //	If excess free size is big enough
    if (addFreeBlock)
    {
        //	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block
        free_block = newBlock + ALIGN(FREE_BLOCK_HEADER_SIZE + size);
        free_block->next = NULL;
        free_block->prev = NULL;
        free_block->is_free = 1;
        free_block->size = ALIGN(FREE_BLOCK_HEADER_SIZE + excessSize);
        free_block->address = free_block + FREE_BLOCK_HEADER_SIZE;
        excessFreeBlock = (void *)free_block;

        //	Checks if the new block was allocated from the free memory list
        if (fromFreeList)
        {
            //	Removes new block and adds the excess free block to the free list
            sprintf(buffer, "Allocating block of size %d and adding block of size %d\n", new_block->size, free_block->size);
            puts(buffer);
            replace_block_freeList(newBlock, excessFreeBlock);
        }
        else
        {
            //	Adds excess free block to the free list
            sprintf(buffer, "Adding block %p to freeList\n", free_block);
            puts(buffer);
            add_block_freeList(excessFreeBlock);
        }
    }
    //	Otherwise add the excess memory to the new block
    else
    {
        //	TODO: Add excessSize to size and assign it to the new Block
        new_block->size = ALIGN(new_block->size + excessSize);
        newBlock = (void *)new_block;

        //	Checks if the new block was allocated from the free memory list
        if (fromFreeList)
        {
            //	Removes the new block from the free list
            sprintf(buffer, "Allocating block %p\n", new_block);
            puts(buffer);
            remove_block_freeList(newBlock);
        }
    }
}

/*
 *	Funcation Name: replace_block_freeList
 *	Input type:		void*, void*
 * 	Output type:	void
 * 	Description:	Replaces old block with the new block in the free list
 */
void replace_block_freeList(void *oldBlock, void *newBlock)
{
    //	TODO: Replace the old block with the new block

    //	Updates SMA info
    totalAllocatedSize += (get_blockSize(oldBlock) - get_blockSize(newBlock));
    totalFreeSize += (get_blockSize(newBlock) - get_blockSize(oldBlock));
    char buffer[60];
    sprintf(buffer, "Replacing block %p; adding block %p\n", oldBlock, newBlock);
    puts(buffer);

    block_header *alloc_block, *free_block;

    alloc_block = (block_header *)oldBlock;
    if (alloc_block->is_free)
    {
        remove_block_freeList;
    }

    add_block_freeList(newBlock);
}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block to the the free memory list
 */
void add_block_freeList(void *block)
{
    //	TODO: 	Add the block to the free list
    //	Hint: 	You could add the free block at the end of the list, but need to check if there
    //			exits a list. You need to add the TAG to the list.
    //			Also, you would need to check if merging with the "adjacent" blocks is possible or not.
    //			Merging would be tideous. Check adjacent blocks, then also check if the merged
    //			block is at the top and is bigger than the largest free block allowed (128kB).

    //	Updates SMA info
    totalAllocatedSize -= get_blockSize(block);
    totalFreeSize += get_blockSize(block);
    char buffer[60];

    block_header *curr, *new_block;

    new_block = (block_header *)block;
    new_block->is_free = 1;

    if (!freeListHead ||
        (unsigned long)freeListHead > (unsigned long)new_block) // checks if empty list or address smaller than head address
    {
        if (freeListHead)
        {
            if (freeListHead->is_free) // if head exists and is free
            {
                sprintf(buffer, "Free List Head %p, size: %d\n", freeListHead, freeListHead->size);
                puts(buffer);
                freeListHead->size = ALIGN(freeListHead->size + new_block->size - FREE_BLOCK_HEADER_SIZE);
                freeListHead = (void *)freeListHead;
                sprintf(buffer, "After merge ->  %p; size %d\n", freeListHead, freeListHead->size);
                puts(buffer);
            }
            else
            { // if head exists and is allocated
                sprintf(buffer, "Allocated List Head %p", freeListHead);
                puts(buffer);
                freeListHead->prev = new_block;
                new_block->next = freeListHead;
                freeListHead = (void *)new_block;
            }
        } // if there is no free list head
        new_block->next = freeListHead;
        freeListHead = (void *)new_block;
        freeListTail = freeListHead;
        sprintf(buffer, "No list head --> new head %p, size: %d\n", new_block, new_block->size);
        puts(buffer);
    }
    else
    {
        curr = freeListHead;
        while (curr->next &&
               (unsigned long)curr->next < (unsigned long)new_block)
        {
            // keep iterating long as new_block's address is larger than current address
            curr = curr->next;
        }

        if (!curr->next) // if curr is list tail
        {
            if (curr->is_free)
            { // if tail is free
                sprintf(buffer, "Free List Tail %p, size: %d\n", curr, curr->size);
                puts(buffer);
                new_block->size = ALIGN(curr->size + new_block->size - FREE_BLOCK_HEADER_SIZE);
                new_block->prev = curr->prev;
                curr->prev->next = new_block;
                freeListTail = new_block; // should not need to do this but let's be extra safe
                sprintf(buffer, "After merge: %p; size %d\n", curr, curr->size);
                puts(buffer);
            }
            else
            { // if tail block is not free
                sprintf(buffer, "Allocated List Tail %p\n", curr);
                puts(buffer);
                curr->next = new_block;
                new_block->prev = curr;
                freeListTail = new_block;
                sprintf(buffer, "After merge, tail is %p\n", freeListTail);
                puts(buffer);
            }
        }
        else
        {
            if (!curr->is_free && !curr->next->is_free)
            {
                sprintf(buffer, "Both Allocated\n");
                puts(buffer);
                new_block->next = curr->next;
                new_block->prev = curr;
                curr->next->prev = new_block;
                curr->next = new_block;
            }
            else if (curr->is_free && !curr->next->is_free)
            {
                sprintf(buffer, "Prev Free with size %d, Next Alloc\n", curr->size);
                puts(buffer);
                curr->size = ALIGN(curr->size + new_block->size - FREE_BLOCK_HEADER_SIZE);
                sprintf(buffer, "Prev size after merge %d\n", curr->size);
                puts(buffer);
            }
            else if (!curr->is_free && curr->next->is_free)
            {
                sprintf(buffer, "Prev Alloc, Next Free with size %d\n", curr->next->size);
                puts(buffer);
                curr->next->size = ALIGN(curr->next->size + new_block->size - FREE_BLOCK_HEADER_SIZE);
                sprintf(buffer, "Next size after merge %d\n", curr->next->size);
                puts(buffer);
            }
            else
            { // if both neighbors are free;
                sprintf(buffer, "Both Free, sizes %d and %d\n", curr->size, curr->next->size);
                puts(buffer);
                new_block->size = ALIGN(curr->size + new_block->size + curr->next->size - FREE_BLOCK_HEADER_SIZE);
                new_block->next = curr->next->next;
                new_block->prev = curr->prev;
                curr->next->next->prev = new_block;
                curr->prev->next = new_block;
                curr->prev = NULL;
                curr->next->next = NULL;
                sprintf(buffer, "Size after merge %d\n", new_block->size);
                puts(buffer);
            }
        }
    }
}

/*
 *	Funcation Name: remove_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Removes a memory block from the the free memory list
 */
void remove_block_freeList(void *block)
{
    //	TODO: 	Remove the block from the free list
    //	Hint: 	You need to update the pointers in the free blocks before and after this block.
    //			You also need to remove any TAG in the free block.

    //	Updates SMA info
    totalAllocatedSize += get_blockSize(block);
    totalFreeSize -= get_blockSize(block);
    char buffer[60];
    block_header *dead_block;

    dead_block = (block_header *)block;
    dead_block->is_free = 0;

    if (!dead_block->next)
    {
        dead_block->prev->next = NULL;
        freeListTail = dead_block->prev;
    }

    else if (!dead_block->prev)
    {
        dead_block->next->prev = NULL;
        freeListHead = dead_block->next;
    }
    else
    {
        dead_block->next->prev = dead_block->prev;
        dead_block->prev->next = dead_block->next;
    }

    sprintf(buffer, "Removing block of size %d from list\n", dead_block->size);
    puts(buffer);
}

/*
 *	Funcation Name: get_blockSize
 *	Input type:		void*
 * 	Output type:	int
 * 	Description:	Extracts the Block Size
 */
int get_blockSize(void *ptr)
{
    int *pSize;

    //	Points to the address where the Length of the block is stored
    pSize = (int *)ptr;

    //	Returns the deferenced size
    return *(int *)pSize;
}

/*
 *	Funcation Name: get_largest_freeBlock
 *	Input type:		void
 * 	Output type:	int
 * 	Description:	Extracts the largest Block Size
 */
int get_largest_freeBlock()
{
    int largestBlockSize = 0;
    block_header *curr;
    char buffer[60];

    curr = freeListHead;
    largestBlockSize = curr->size;
    while (curr->next)
    {
        if (curr->size > largestBlockSize)
        {
            largestBlockSize = curr->size;
        }
        curr = curr->next;
    }

    //	TODO: Iterate through the Free Block List to find the largest free block and return its size

    return largestBlockSize;
}

int list_details()
{
    int num_blocks;
    block_header *curr;
    char buffer[100];
    puts("FREE LIST DETAILS:\n------------------------------------------------");

    if (!freeListHead)
    {
        return 0;
    }
    else
    {
        curr = freeListHead;
        num_blocks = 1;

        sprintf(buffer, "HEAD BLOCK %d: %p; size %d; is_free %d\n", num_blocks, curr, curr->size, curr->is_free);
        puts(buffer);
        while (curr->next)
        {
            if (!curr->next->next)
            {
                num_blocks++;
                sprintf(buffer, "TAIL BLOCK %d: %p; size %d; is_free %d\n", num_blocks, curr->next, curr->next->size, curr->next->is_free);
                puts(buffer);
                break;
            }
            num_blocks++;
            sprintf(buffer, "BLOCK %d: %p; size %d; is_free %d\n", num_blocks, curr->next, curr->next->size, curr->next->is_free);
            puts(buffer);
            curr = curr->next;
        }
    }
    sprintf(buffer, "Number of blocks %d\n", num_blocks);
    puts(buffer);
    puts("------------------------------------------------\n");
    return num_blocks;
}