/*
 * mm-modified.c - The memory-efficient malloc package.
 * Uses explicit free lists with a first fit approach.
 * To find a fit, the program will look at the "start" of the free blocks
 * in the heap, then move backwards towards the heap pointer until it can 
 * find a block that can fit the requested space. 
 *
 * A block is payload with header, footer and in the case of free blocks, 
 * prev and next block pointer respectively. the minimum block size is 24. 
 *
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/

team_t team = {
    /* Team name */
    "Brian & Manil",
    /* First member's full name */
    "Brian Lam",
    /* First member's email address */
    "brian.lam@wustl.edu",
    /* Second member's full name (leave blank if none) */
    "Manil Bastola",
    /* Second member's email address (leave blank if none) */
    "mbastola@wustl.edu"
};

/* Helper Macros */

/* Referenced from page 830 in A Programmer's Perspective 
   textbook, Randal E. Bryant and David R. O'Hallaron  */

/* Basic constants and macros */
#define WSIZE       4    /* Word and header/footer size (bytes) */
#define DSIZE       8    /* Double word size (bytes) */
#define CHUNKSIZE   1<<12 //Extend heap by this amount (bytes)

#define MAX(x, y) (x > y ? x : y)

/*Each block needs at least 8 bytes for header, 8 bytes for prologue,
  4 bytes for next pointer, 4 bytes for previous pointer */
#define MIN_BLOCKSIZE    24

/* Rounds up to the nearest multiple of DSIZE */
#define ALIGN(p) (((size_t)(p) + (DSIZE-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block pointer bp, computer arithmetical next and previous blocks */
#define RIGHT_BLKP(bp)  ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define LEFT_BLKP(bp)  ((char *)(bp) - GET_SIZE(HDRP(bp) - WSIZE))

/* Given block ptr bp, compute address of next and previous free blocks */
#define PREV_BLKP(bp)(*(char **)(bp + DSIZE))
#define NEXT_BLKP(bp)(*(char **)(bp))

#define SETPREV_BLKP(bp, ptrToSet) (PREV_BLKP(bp) = ptrToSet)
#define SETNEXT_BLKP(bp, ptrToSet) (NEXT_BLKP(bp) = ptrToSet)

static char *heap_listp;
static char *start;

static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_freeblock(size_t asize);
static void *coalesce(void *bp);
static void insertBlock(void *bp);
static void removeBlock(void *bp);
void mm_checkBlock(void* bp);
void mm_check (void);

/*
 * mm_init
 *
 * Requests memory for the heap, and sets up the space in memory
 * for use with malloc. Returns -1 if fails, 0 otherwise
 */
int mm_init(void)
{
  /*return -1 if unable to get heap space*/
  if ((heap_listp = mem_sbrk(2*MIN_BLOCKSIZE)) == NULL) {
    return -1;
    }

  /* Alignment padding */
  PUT(heap_listp, 0);

  /* Prologue Header */
  PUT(heap_listp + WSIZE, PACK(MIN_BLOCKSIZE, 1));

  /* Logical previous pointer */
  PUT(heap_listp + DSIZE, 0);

  /* Logical next pointer */
  PUT(heap_listp + DSIZE + WSIZE, 0);

  /* Prologue Footer */
  PUT(heap_listp + MIN_BLOCKSIZE, PACK(MIN_BLOCKSIZE, 1));

  /* Epilogue Footer */
  PUT(heap_listp +WSIZE + MIN_BLOCKSIZE, PACK(0, 1));

  /* Set the free list pointer to the Logical previous position */
  start = heap_listp + DSIZE;
 
  /*return -1 if unable to get heap space*/
  if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
    return -1;
  }
  return 0;
}

/*
 * extend_heap
 * @param: size_t words: the numbers of words to add to memory
 *
 * Calculate the size in memory to request based off of
 * the parameter words. Returns NULL if this fails. Otherwise,
 * it also calls coalesce on the new memory to add it to our heap
 */
static void *extend_heap(size_t words)
{
  char *bp;
  size_t size;
 
  /* Ensure that an even number of words are requested for alignment */
  if (words % 2 == 1) {
    size = (words + 1) * WSIZE;
  } else {
    size = words * WSIZE;
  }

  /* Request memory, return NULL if fails */
  if ((int)(bp = mem_sbrk(size)) == -1) {
    return NULL;
  }

  /* Set up the header and footer information in our new heap space */
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  PUT(HDRP(RIGHT_BLKP(bp)), PACK(0, 1));

  /* Coalesce the new memory to our heap*/
  return coalesce(bp);
}

/*
 * mm_malloc
 * @param: size_t size: the size (bytes) to allocate
 *
 * Calculates the size needed to allocate, then tries to
 * find a block in the heap for it. If it cannot, then it
 * extends the heap.
 */
void *mm_malloc(size_t size) {
  /* Calculate the size that we need to
     allocate for proper alignment */
  size_t newsize = MAX(ALIGN(size) + DSIZE, MIN_BLOCKSIZE);
  size_t extendsize;
  char *bp;

  /* Ignore spurious requests */
  if (size == 0) {
      return NULL;
  }

  /* Try to find a place in the heap to fit this requested space*/
  if ((bp = find_freeblock(newsize))) {
    place(bp, newsize);
    
    return bp;
  }
  else {
    /*If there was no space in the heap, then extend the heap*/
    extendsize = MAX(newsize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL) {
      return NULL;
    }
    place(bp, newsize);
    
    return bp;
  }
}

/*
 * find_freeblock
 * @param size_t asize : the size of block needed
 *
 * Returns the first block of at least asize that
 * it finds in the heap. Returns NULL if unable.
 */
static void *find_freeblock(size_t asize)
{
  char *bp = start;
  size_t size;
 
  /* Go down the heap until we hit a block that can fit asize */
  while (GET_ALLOC(HDRP(bp)) == 0) {
    size = (size_t) GET_SIZE(HDRP(bp));
    if ( asize <= size){
      return bp;
    }
    
    /* InsertBlock puts new blocks before the start pointer */
    bp = PREV_BLKP(bp);
  }
  return NULL;
}

/*
 * place
 * @param: void* bp: Address to place in
 *         asize: size needed from bp
 *
 * Place the requested block at the beginning of
 * the free block, and splits if the remaining space
 * is equal to or greater than the minimum block size
 */
static void place(void *bp, size_t asize)
{
  size_t size = GET_SIZE(HDRP(bp));
  size_t remblockSize = size - asize;
 
  /* If we have room to make another new block, separate this
       block into two blocks after placing */
  if (remblockSize >= MIN_BLOCKSIZE) {
    /* Pack the header and footer only in the space that we need */
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    removeBlock(bp);
    bp = RIGHT_BLKP(bp);
    /* Clear the header and footer information in the remaining space */
    PUT(HDRP(bp), PACK(remblockSize, 0));
    PUT(FTRP(bp), PACK(remblockSize, 0));
    coalesce(bp);
  } else {
    /*Otherwise, just insert the block and pack the header/footer info as needed */
    PUT(HDRP(bp), PACK(size, 1));
    PUT(FTRP(bp), PACK(size, 1));
    removeBlock(bp);
  }
}

/* Free a pointer from the heap by
   clearing its header and footer information,
   and then calling coalesce on it  */
void mm_free(void *bp)
{
  size_t size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(size, 0)); //Free header
  PUT(FTRP(bp), PACK(size, 0)); //Free footer
  /*Coalesce this new freed space */
  coalesce(bp);
}

/*
 * coalesce
 * @param: void* bp: The address in memory to coalesce
 *                   with its adjacent blocks
 *
 * If there are any adjacent free blocks in memory,
 * free the necessary header/footer information to
 * combine it with the current pointer */
static void *coalesce(void *bp)
{
  size_t left_alloc = GET_ALLOC(FTRP(LEFT_BLKP(bp)));
  left_alloc = (left_alloc || LEFT_BLKP(bp) == bp);
  size_t right_alloc = GET_ALLOC(HDRP(RIGHT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));
 
  /* Only right block is free */
  if (left_alloc && !right_alloc)
    {           
      size += GET_SIZE(HDRP(RIGHT_BLKP(bp)));
      removeBlock(RIGHT_BLKP(bp));
      PUT(HDRP(bp), PACK(size, 0));
      PUT(FTRP(bp), PACK(size, 0));
    }
 
  /* Only left block is free */
  else if (!left_alloc && right_alloc)
    {       
      size += GET_SIZE(HDRP(LEFT_BLKP(bp)));
      /* Pack starting from the left pointer since there is free space */
      bp = LEFT_BLKP(bp);
      removeBlock(bp);
      PUT(HDRP(bp), PACK(size, 0));
      PUT(FTRP(bp), PACK(size, 0));
    }
 
  /* Both left and right blocks are free */
  else if (!left_alloc && !right_alloc)
    {       
      removeBlock(LEFT_BLKP(bp));
      removeBlock(RIGHT_BLKP(bp));
      size += GET_SIZE(HDRP(LEFT_BLKP(bp))) 
	+ GET_SIZE(HDRP(RIGHT_BLKP(bp)));
      bp = LEFT_BLKP(bp);
      PUT(HDRP(bp), PACK(size, 0));
      PUT(FTRP(bp), PACK(size, 0));
    }
 
  insertBlock(bp);
  return bp;
}

/*
 * insertBlock
 * @param: void *bp: The address to insert onto the heap
 *
 * Inserts block into whereever the start pointer is pointing,
 * and set this as the start pointer
 */

static void insertBlock(void *bp)
{
  SETPREV_BLKP(bp, start);
  SETNEXT_BLKP(start,bp);
  SETNEXT_BLKP(bp, NULL); 
  start = bp;
}

/* removeBlock
 * @param void* bp: address of the pointer to be removed
 *
 * Removes a block (bp)  by setting its next pointer to point to
 * bp's previous, then pointing bp's previous pointer's next to
 * bp's next. If there was no next pointer, then the previous block is
 * set as the start of the heap
 */
static void removeBlock(void *bp)
{
  /* If there is a next logical pointer, pointer bp's previous to it */
  if (NEXT_BLKP(bp) != NULL){
    SETPREV_BLKP(NEXT_BLKP(bp),PREV_BLKP(bp));
  }
   /* Otherwise, set the start of the heap to the logical previous of bp */
  else
    start = PREV_BLKP(bp); 
    /* Connect the two adjacent nodes so bp is logically removed */
   SETNEXT_BLKP(PREV_BLKP(bp),NEXT_BLKP(bp));
}

/*
 * mm_realloc
 * @param: void *ptr: address of pointer to reallocate
 *         size_t size: size to reallocate 
 *
 * Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  size_t ptr_size;
  /* Calculate the size of the block we need for realloc */
  size_t asize = MAX(ALIGN(size) + DSIZE, MIN_BLOCKSIZE);

  /* If the pointer is null, the call is equivalent to mm_malloc(size) */
  if (ptr == NULL) {
    mm_malloc(size);
  }

  /* If the size is 0, then the call is equivalent to mm_free(ptr) */
  if (size == 0) {
    mm_free(ptr);
  }
  
  /* Keep track of the original size of ptr */
  ptr_size = (GET_SIZE(HDRP(ptr)));
 
  /* Realloc would have no effect so just return */
  if (asize == ptr_size) {
    return ptr;
  } else if (asize < ptr_size) {

    size = asize;
    /* If the amount remaining can't make a block, just return original ptr */
    if (ptr_size - size <= MIN_BLOCKSIZE){
      return ptr;
    }
    else {
      /* Pack the space with our newsize */
      PUT(HDRP(ptr), PACK(size, 1));
      PUT(FTRP(ptr), PACK(size, 1));
      PUT(HDRP(NEXT_BLKP(ptr)), PACK(ptr_size-size, 1));
      /* Free the remaining block */
      mm_free(NEXT_BLKP(ptr));
      return ptr;
    }
  }

  /* Request more space */
  void* newptr = mm_malloc(size);
  if (newptr == NULL) {
    return 0;
  }

  if (size < ptr_size) {
    ptr_size = size;
  }
  memcpy(newptr, ptr, ptr_size);
  mm_free(ptr);
  return newptr;
}



/*
 * mm_check
 *
 * Loop through every block in the heap and call checkBlock to
 * on each block
 
void mm_check (void) {
  void* bp = heap_listp;

  // Check prologue size 
  if ((GET_SIZE(HDRP(bp)) != MIN_BLOCKSIZE)) {
    printf("Prologue header - wrong size");
  }
  // Check prologue allocation 
  if (!GET_ALLOC(HDRP(bp))) {
    printf("Prologue header - allocation bit incorrect");
  }
  
  bp = start;
  // Check every block starting from the start of the free list and going backwards
  while (bp != NULL) {
    mm_checkBlock(bp);
    bp = PREV_BLKP(bp);
  }
}


 * mm_checkBlock
 * @param: (*void bp): bp is the address of the block to check
 *
 * Checks a block in memory for correctness. It tests if the header
 * and footer exist, then checks if the sizes match, if the allocation bits
 * match, and whether it's out of bounds
 
void mm_checkBlock(void* bp) {
  // Test if header and footer exist 
  if (!HDRP(bp) || !FTRP(bp)) {
    return;
  }
  // Check header and footer information for matching information 
  if (GET_SIZE(HDRP(bp)) != GET_SIZE(FTRP(bp))){
    printf("Header size does not match the footer size \n");
  }

  if (GET_ALLOC(HDRP(bp)) != GET_ALLOC(FTRP(bp))) {
    printf("Header does not match the footer alloc \n");
  }

  // Check the space of the block with the boundaries of the heap 
  void* next = NEXT_BLKP(bp);
  void* prev = PREV_BLKP(bp);
  if (next < mem_heap_lo() || prev < mem_heap_lo()
      || prev > mem_heap_hi() || next > mem_heap_hi()){
    printf("Heap out of bounds \n");
  }
}
*/
