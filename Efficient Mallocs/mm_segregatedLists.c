/*
 * mm-modified.c - The memory-efficient malloc package.
 * modified versiion of mm-naive.c 
 * Uses segregated explicit lists of size 2^n.
 * In this approach, a block is allocated by searching for the appropriate sized fee block and allocating that space for use. 
 * A block is payload plus header, footer and in the case of free blocks,prev and next block pointer respectively.  Blocks are 
 * never coalesced and reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
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


//** Helper Macros **

/* taken from page 830 in the textbook */

/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */
#define DSIZE       8       /* Doubleword size (bytes) */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))  

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

/* Given block ptr bp, compute address of artimetical next and previous blocks */
#define RIGHT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define LEFT_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/*Given block ptr bp, compute the logical next and the previous of the free blocks */
#define PREV_BLKP(bp)  (*(char **)(bp))
#define NEXT_BLKP(bp)  (*(char **)(bp + DSIZE))

#define LIST_PREV(bp) (*(char **) (bp))
#define LIST_NEXT(bp) (*(char **)(NEXT_BLKP(bp)))

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SET(to, from) (*(unsigned int *)(to) = (unsigned int)(from))

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define LIST_COUNT 20

/*Global variable that points to the front of the heap*/
static char* start;

/* Declare function prototypes so they can be called */
void *find_freeblock(size_t asize); 
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *coalesce(void *ptr);
void removeBlock(void *ptr);
void insertBlock(void *ptr, size_t size);
static void printblock(void *bp);
static void checkblock(void *bp) ;

void* seglist[LIST_COUNT];

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  int i;
  if((start = mem_sbrk(24*WSIZE))==(void *)-1)
    return -1;
  PUT(start,0); /*Alignment Pad*/

  for (i = 0; i < LIST_COUNT; i++)
  {
    seglist[i] = NULL; /*base pointer for freeblock size <= 2^i*/
  }

  PUT(start+21*WSIZE,PACK(DSIZE,1)); /*prologue header*/
  PUT(start+22*WSIZE,PACK(DSIZE,1)); /*prologue footer*/
  PUT(start+23*WSIZE,PACK(0,1));  /*epilogue header*/

  /*extend empty heap with a free block of CHUNKSIZE bytes*/
  if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
    return -1;

  return 0;
}

static void *extend_heap(size_t words)
{
  char *bp;
  size_t size;
  size  = (words % 2)?(words+1)*WSIZE: words*WSIZE;
  if ((long)(bp = mem_sbrk(size))==-1)
    return NULL;
  PUT(HDRP(bp),PACK(size,0));
  PUT(FTRP(bp),PACK(size,0));
  PUT(HDRP(RIGHT_BLKP(bp)),PACK(0,1));
  return coalesce(bp); 
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
  char *bp;
  size_t newsize = ALIGN(size + SIZE_T_SIZE);
  size_t extendsize; 

  if (size == 0)
    return NULL;
  if (size <= DSIZE)
    newsize = 2*DSIZE;
  else
    newsize = DSIZE*((size+(DSIZE)+(DSIZE-1))/DSIZE);

  int count = 0;
  extendsize = newsize;
  while (count < LIST_COUNT) {
    if((count == LIST_COUNT - 1) || ((size <= 1) && (seglist[count] != NULL))) {
      bp = seglist[count];

      while ((bp != NULL) && ((newsize > GET_SIZE(HDRP(bp))))) {
	bp = LIST_PREV(bp);
      }
      if (bp != NULL){
	break;
      }
    }
    size = size / 2;
    count++;
  }

  if (bp == NULL) {
    extendsize = MAX(newsize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize)) == NULL){
      return NULL;
    }
  }
  
  if (bp!=NULL){
    place(bp,newsize);      
    return bp;
   }

  return;
}

static void place(void *bp, size_t asize)
{
  size_t csize = GET_SIZE(HDRP(bp));
  size_t remblockSize;
  remblockSize = csize-asize;
  if (remblockSize >= 2*DSIZE){
    PUT(HDRP(bp),PACK(asize,1));
    PUT(FTRP(bp),PACK(asize,1));
    //remove the connection with other free blocks
    removeBlock(bp);
    bp = RIGHT_BLKP(bp);
    //add the leftover to the front of the correct list
    PUT(HDRP(bp),PACK(remblockSize,0));
    PUT(FTRP(bp),PACK(remblockSize,0));
    insertBlock(bp,remblockSize);
  }
    else{
    PUT(HDRP(bp),PACK(csize,1));
    PUT(FTRP(bp),PACK(csize,1));
    removeBlock(bp);
  }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  size_t size = GET_SIZE(HDRP(ptr));
  PUT(HDRP(ptr),PACK(size,0));
  PUT(FTRP(ptr),PACK(size,0));
  coalesce(ptr); 
}

static void *coalesce(void *ptr)
{
  size_t left_alloc;
  size_t right_alloc;
  size_t size = GET_SIZE(HDRP(ptr));

  //none of the blocks are free
  if (left_alloc && right_alloc) {               
    insertBlock(ptr, size);
    return (ptr);
  }
  
  //Only left block is free
  else if (!left_alloc && right_alloc) {
    //merge physically
    removeBlock(LEFT_BLKP(ptr));
    size = size + GET_SIZE(HDRP(LEFT_BLKP(ptr)));
    PUT(FTRP(ptr), PACK(size, 0));
    PUT(HDRP(LEFT_BLKP(ptr)), PACK(size, 0));
    ptr = LEFT_BLKP(ptr);
  }

  //Only right block is free
  else if (left_alloc && !right_alloc) {
    removeBlock(RIGHT_BLKP(ptr));
    size = size + GET_SIZE(HDRP(LEFT_BLKP(ptr)));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size,0));
  }

  //Both blocks are free
  else if (!left_alloc && !right_alloc) {
    removeBlock(LEFT_BLKP(ptr));
    removeBlock(RIGHT_BLKP(ptr));
    size = size + GET_SIZE(HDRP(LEFT_BLKP(ptr))) + GET_SIZE(FTRP(RIGHT_BLKP(ptr)));
    PUT(HDRP(LEFT_BLKP(ptr)), PACK(size, 0));
    PUT(FTRP(RIGHT_BLKP(ptr)), PACK(size, 0)); 

    //Start pointer at the next left
    ptr = LEFT_BLKP(ptr);
  }
  //Insert ptr into free list
  insertBlock(ptr,size);
  return ptr;
}

void insertBlock(void *ptr, size_t size)
{
  int count = 0;
  void *tmp = ptr;
  void *pos = NULL;

  while ((count < LIST_COUNT - 1) && (size > 1)) {
    size = size / 2;
    count++;
  }

  tmp = seglist[count];
  while((tmp != NULL) && (size > GET_SIZE(HDRP(tmp)))) {
    pos = tmp;
    tmp = LIST_PREV(tmp);
  }

  if (tmp != NULL) {
    if (pos != NULL){
      SET(PREV_BLKP(ptr), tmp);
      SET(NEXT_BLKP(tmp), ptr);
      SET(NEXT_BLKP(ptr), pos);
      SET(PREV_BLKP(pos), ptr);
    } else {
      SET(PREV_BLKP(ptr), tmp);
      SET(NEXT_BLKP(tmp), ptr);
      SET(NEXT_BLKP(ptr), NULL);
      seglist[count] = ptr;
    }
  }
  else {
    if (pos != NULL) {
      SET(PREV_BLKP(ptr), NULL);
      SET(NEXT_BLKP(ptr), pos);
      SET(PREV_BLKP(pos), ptr);
    } else {
      SET(PREV_BLKP(ptr), NULL);
      SET(NEXT_BLKP(ptr), NULL);
    }
  }
}

void removeBlock(void *bp)
{
  int count = 0;
  size_t size = GET_SIZE(HDRP(bp));

  while((count < LIST_COUNT - 1) && (size > 1)){
    size = size / 2;
    count++;
  }

  if (LIST_PREV(bp) != NULL){
    if (LIST_NEXT(bp) != NULL) {
      SET(PREV_BLKP(LIST_PREV(bp)), LIST_NEXT(bp));
      SET(PREV_BLKP(LIST_NEXT(bp)), LIST_PREV(bp));
    } else {
      SET(NEXT_BLKP(LIST_PREV(bp)), NULL);
      seglist[count] = LIST_PREV(bp);
    }
  } else {
    if(LIST_NEXT(bp) != NULL) {
      SET(PREV_BLKP(LIST_NEXT(bp)), NULL);
    } else {
      seglist[count] = NULL;
    }
  }
}
    
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}


//The following is borrowed from https://www.clear.rice.edu/comp221/assignments/malloc/mm.c and must be deleted before the 
//final commit

//***Heap Checker Implementation**************

 /*
 * Requires:
 *   "bp" is the address of a block.
 *
 * Effects:
 *   Perform a minimal check on the block "bp".
 */
static void checkblock(void *bp) 
{
  if ((size_t )bp % DSIZE)
    printf("Error: %p is not doubleword aligned\n", bp);
  if (GET(HDRP(bp)) != GET(FTRP(bp)))
    printf("Error: header does not match footer\n");
}
 
/* 
 * Requires:
 *   None.
 *
 * Effects:
 *   Perform a minimal check of the heap for consistency. 
 */
void
checkheap(bool verbose) 
{
  void *bp;

  if (verbose)
    printf("Heap (%p):\n", start);
  
  if (GET_SIZE(HDRP(start)) != DSIZE ||!GET_ALLOC(HDRP(start)))
    printf("Bad prologue header\n");
  checkblock(start);
  
  for (bp = start; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    if (verbose)
      printblock(bp);
    checkblock(bp);
  }
  
  if (verbose)
    printblock(bp);
  if (GET_SIZE(HDRP(bp)) != 0 || !GET_ALLOC(HDRP(bp)))
    printf("Bad epilogue header\n");
}
 
/*
 * Requires:
 *   "bp" is the address of a block.
 *
 * Effects:
 *   Print the block "bp".
 */
static void printblock(void *bp) 
{
  bool halloc, falloc;
  size_t hsize, fsize;
  
  checkheap(false);
  hsize = GET_SIZE(HDRP(bp));
  halloc = GET_ALLOC(HDRP(bp));  
  fsize = GET_SIZE(FTRP(bp));
  falloc = GET_ALLOC(FTRP(bp));  
  
  if (hsize == 0) {
    printf("%p: end of heap\n", bp);
    return;
  }
  
  printf("%p: header: [%zu:%c] footer: [%zu:%c]\n", bp, 
	 hsize, (halloc ? 'a' : 'f'), 
	 fsize, (falloc ? 'a' : 'f'));
}













