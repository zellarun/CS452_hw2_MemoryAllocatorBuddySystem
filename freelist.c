/* Author: Zella Running
 * Description: Maintains a free list for each block size. Stores pointers in the forst bytes of free blocks, and each free block points to the next free block of same size.
 * Class: CS 451 - HW2 Memory Allocator w/Buddy System
 * Date: 2026 February 11
 */

#include "freelist.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

/*  (1) make count
    (2) allocate array of void* of size count, initialized to NULL
    (3) return pointer to array
*/
extern FreeList freelistcreate(size_t size, int l, int u){
    (void)size;
    int count = u - l + 1;
    void **lists = mmalloc(count * sizeof(void *));
    if((long)lists == 1)
        return NULL;

    //initialize all lists to empty
    for (int i = 0; i < count; i++){
        lists[i] = NULL;
    }

    return lists;
}

/*  (1) free array of void*
    (2) return nothing
*/
extern void freelistdelete(FreeList f, int l, int u){
    int count = u - l +1;
    mmfree(f, count * sizeof(void *));
}

/*  (1) check free list for level e for available block
    (2) if found, remove from list and return pointer to block
    (3) if not found, return NULL
*/
extern void *freelistalloc(FreeList f, void *base, int e, int l){
    (void)base;
    void **lists = f;
    int index = e - l;
    
    void *block = lists[index];
    if (block == NULL)
        return NULL;

    void *next = *(void **)block;
    lists[index] = next;

    return block;
}

/*  (1) add block to free list for level e
    (2) return nothing
*/
extern void freelistfree(FreeList f, void *base, void *mem, int e, int l){
    (void)base;
    void **lists = f;
    int index = e - l;

    //store pointer to current head in this block
    *(void **)mem = lists[index];

    //make this block the new head
    lists[index] = mem;
}

/*  (1) check if block is in free list for level e
    (2) return 1 if found, 0 if not found

*/
extern int freelistsize(FreeList f, void *base, void *mem, int l, int u){
    (void)f;
    (void)base;
    (void)mem;
    (void)l;
    (void)u;

    return 0;
}

/*  (1) print free list for each level from l to u, showing addresses of free blocks
    (2) return nothing
*/
extern void freelistprint(FreeList f, int l, int u){
    void **lists = f;

    for (int e = l; e < u; e++){
         int index = e - l;
         printf("Free list[2%d} (size %4lu): ", e, e2size(e));
         
        void *block = lists[index];
        if (block == NULL){
            printf("empty\n");
        } else {
            while (block != NULL){
                printf("%p -> ", block);
                block = *(void **)block;
            }
            printf("NULL\n");
        }
    }
  
}
