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
#define FREE_BLOCK_HEADER_SIZE 2 * sizeof(char *) + sizeof(int) // Size of the Header in a free memory block
//	TODO: Add constants here

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
    int excessSize = 2048;

    //	TODO: 	Allocate memory by incrementing the Program Break by calling sbrk() or brk()
    //	Hint:	Getting an exact "size" of memory might not be the best idea. Why?
    //			Also, if you are getting a larger memory, you need to put the excess in the free list

    //	Allocates the Memory Block
    newBlock = sbrk(size + excessSize);
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

    //	TODO: 	Allocate memory by using Worst Fit Policy
    //	Hint:	Start off with the freeListHead and iterate through the entire list to
    //			get the largest block

    //	Checks if appropriate block is found.
    if (blockFound)
    {
        //	Allocates the Memory Block
        allocate_block(worstBlock, size, excessSize, 1);
    }
    else
    {
        //	Assigns invalid address if appropriate block not found in free list
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
    block_header free_block;

    // 	Checks if excess free size is big enough to be added to the free memory list
    //	Helps to reduce external fragmentation

    //	TODO: Adjust the condition based on your Head and Tail size (depends on your TAG system)
    //	Hint: Might want to have a minimum size greater than the Head/Tail sizes
    addFreeBlock = excessSize > FREE_BLOCK_HEADER_SIZE;

    //	If excess free size is big enough
    if (addFreeBlock)
    {
        //	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block
        free_block.size = excessSize;
        free_block.next = NULL;
        free_block.prev = NULL;
        free_block.is_free = 1;
        excessFreeBlock = &free_block;
        //	Checks if the new block was allocated from the free memory list
        if (fromFreeList)
        {
            //	Removes new block and adds the excess free block to the free list
            replace_block_freeList(newBlock, excessFreeBlock);
        }
        else
        {
            //	Adds excess free block to the free list
            add_block_freeList(excessFreeBlock);
        }
    }
    //	Otherwise add the excess memory to the new block
    else
    {
        //	TODO: Add excessSize to size and assign it to the new Block

        //	Checks if the new block was allocated from the free memory list
        if (fromFreeList)
        {
            //	Removes the new block from the free list
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
    block_header *added_block;
    char buffer[50];

    added_block = (block_header *)block;

    sprintf(buffer, "Trying to add block of size %d and is_free %d\n", added_block->size, added_block->is_free);
    puts(buffer);

    if (!freeListHead ||
        (unsigned long)freeListHead > (unsigned long)added_block) // if list empty or block address smaller than head address
    {
        if (freeListHead) // if block address smaller than head address
        {

            if (freeListHead->is_free) // if there is a freeListHead and it's free
            {
                sprintf(buffer, "MERGING HEAD\n");
                puts(buffer);
                freeListHead->size = freeListHead->size + added_block->size;
            }
            else
            {                                     // if there is a free list head but it's not free
                freeListHead->prev = added_block; // update list head
                added_block->next = freeListHead;
                freeListHead = added_block;
            }
        }
        // if there is no free List Head
        added_block->next = freeListHead;
        freeListHead = added_block;
    }
    else
    {
        block_header *curr = freeListHead;
        while (curr->next &&
               (unsigned long)curr->next < (unsigned long)added_block)
        {
            curr = curr->next; // keep iterating as long as added block address is larger than current address
        }

        if (!curr->next) // current block is tail
        {
            if (curr->is_free) // if tail block is free
            {
                sprintf(buffer, "MERGING TAIL\n");
                puts(buffer);
                curr->size = curr->size + added_block->size;
                freeListTail = curr;
            }
            else // if tail block is not free -> adjust tail accordingly
            {
                curr->next = added_block;
                added_block->prev = curr;
                freeListTail = added_block;
            }
        }
        else
        {
            if (!curr->is_free && !curr->next->is_free) // if both neighbors are already allocated
            {
                added_block->next = curr->next;
                added_block->prev = curr;
                curr->next->prev = added_block;
                curr->next = added_block;
            }
            else if (curr->is_free && !curr->next->is_free) // if prev neighbor is free but not next
            {
                curr->size = curr->size + added_block->size;
            }
            else if (!curr->is_free && curr->next->is_free) // if next neighbor is free but not prev
            {
                curr->next->size = curr->next->size + added_block->size;
            }
            else // if both neighbors are free
            {
                added_block->size = curr->size + added_block->size + curr->next->size;
                added_block->next = curr->next->next;
                added_block->prev = curr->prev;
                curr->next->next->prev = added_block;
                curr->prev->next = added_block;
                curr->prev = NULL;
                curr->next->next = NULL;
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
    pSize--;

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
    char buffer[50];

    sprintf(buffer, "%d\n", get_blockSize(freeListHead));

    puts(buffer);

    return largestBlockSize;
}

block_header *get_block(void *ptr)
{
    return (block_header *)ptr - 1;
}
