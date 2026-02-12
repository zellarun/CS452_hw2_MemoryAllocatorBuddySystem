/* Author: Zella Running
 * Description: Main allocator implementation. Manages the pool structure, implements splitting and coalescing, and coordinates all the modules.
 * Class: CS 451 - HW2 Memory Allocator w/Buddy System
 * Date: 2026 February 11
 */
#include "balloc.h"
#include "freelist.h"
#include "bbm.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

// Pool structure: contains base address and size of mem. pool, min and max block sizes, arrays of free lists and buddy bitmaps for each level
typedef struct {
    void *base;             //base address of mem. pool
    size_t size;            //total size of mem. pool
    int l, u;               //min exponent and max exponent of block sizes
    FreeList *freelists;    //array of free lists, one for each block size
    BBM *buddy_bitmaps;     //array of buddy bitmaps, one for each block size
    BBM *alloc_bitmaps;     //array of allocation bitmaps, one for each block size, to track allocated blocks and their sizes
} Pool;

/*  (1) create pool structure
    (2) allocate main memory pool using mmalloc
    (3) create free lists and buddy bitmaps for each level, and initialize them
    (4) add initial blocks to free lists, starting with largest blocks working down
    (5) return pointer to pool, or NULL on failure
*/
extern Balloc bcreate(unsigned int size, int l, int u){
    Pool *pool = malloc(sizeof(Pool));
    if (!pool)
        return NULL;

    //allocatie main memory pool
    void *base = mmalloc(size);
    if ((long)base == -1){
        free(pool);
        return NULL;
    }

    //initialize pool structure
    pool->base = base;
    pool->size = size;
    pool->l = l;
    pool->u = u;

    //create free lists
    pool->freelists = freelistcreate(size, l, u);
    if (!pool->freelists){
        mmfree(base, size);
        free(pool);
        return NULL;
    }

    //create buddy bitmaps
    int count = u - l + 1;
    pool->buddy_bitmaps = malloc(count * sizeof(BBM));
    if (!pool->buddy_bitmaps){
        freelistdelete(pool->freelists, l, u);
        mmfree(base, size);
        free(pool);
        return NULL;
    }

    pool->alloc_bitmaps = malloc(count * sizeof(BBM));
    if (!pool->alloc_bitmaps){
        free(pool->buddy_bitmaps);
        freelistdelete(pool->freelists, l, u);
        mmfree(base, size);
        free(pool);
        return NULL;
    }

    //initialize each bitmap
    for (int e = l; e < u; e++){
        int index = e - l;
        pool->buddy_bitmaps[index] = bbmcreate(size, e);
        pool->alloc_bitmaps[index] = bbmcreate(size, e);
        if (!pool->buddy_bitmaps[index] || !pool->alloc_bitmaps[index]){
            //clean up previously created bitmaps
            for (int j = 0; j < index; j++){
                if(pool->buddy_bitmaps[j])
                    bbmdelete(pool->buddy_bitmaps[j]);
                if(pool->alloc_bitmaps[j])
                    bbmdelete(pool->alloc_bitmaps[j]);
            }
            free(pool->alloc_bitmaps);
            free(pool->buddy_bitmaps);
            freelistdelete(pool->freelists, l, u);
            mmfree(base, size);
            free(pool);
            return NULL;
        }
    }

    //intalize the pool, start with largest blocks working down
    void *current = base;
    size_t remaining = size;

    for (int e = u; e >= l && remaining > 0; e--){
        size_t blocksize = e2size(e);

        //create as many block of this size as possible
        while (remaining >= blocksize){
            freelistfree(pool->freelists, base, current, e, l);
            current += blocksize;
            remaining -= blocksize;
        }
    }

    return pool;
    
}

/*  (1) free all memory associated w/pool, including bitmaps, free lists, and mem. pool itself
    (2) return nothing
*/
extern void   bdelete(Balloc pool){
    Pool *p = pool;

    //free bitmaps and free lists
    for (int e = p->l; e <= p->u; e++){
            bbmdelete(p->buddy_bitmaps[e - p->l]);
            bbmdelete(p->alloc_bitmaps[e - p->l]);
    }
    free(p->alloc_bitmaps);
    free(p->buddy_bitmaps);

    //free lists
    freelistdelete(p->freelists, p->l, p->u);

    //free main pool
    mmfree(p->base, p->size);

    //free pool structure
    free(p);
}

static void split_block(Pool *pool, void *mem, int e){
    //calculate new size
    int e_new = e - 1;

    //upper buddy is at mem + 2^e_new
    void *buddy = mem + e2size(e_new);

    //add upper buddy to free list for e_new
    freelistfree(pool->freelists, pool->base, buddy, e_new, pool->l);
}

/*  (1) convert size to exponent e 
    (2) find smallest free list w/available block of size 2^e
    (3) if found at level K where k < e: 
        remove block from list[k]
        split k times: k->k-1..->e, adding buddies to free lists as you go and setting buddy bits in bitmaps
        for each split, put one buddy in appropriate free list
    (4) mark block as allocated in bitmap and return pointer to block
    (5) return NULL if no block is available
*/
extern void *balloc(Balloc pool, unsigned int size){
    Pool *p = pool;

    if (size == 0 || size > p->size)
        return NULL;
    
    //convert size to exponent
    int e = size2e(size);

    //clamp to valid range
    if (e < p->l)
        e = p->l;
    if (e > p->u)
        return NULL;    //if request too large, fail
    
    //find a free block, startng at level e2size
    int k;
    void *block = NULL;
    for (k = e; k <= p->u; k++){
        block = freelistalloc(p->freelists, p->base, k, p->l);
        if (block !=NULL)
            break;
    }

    if (block == NULL)
        return NULL;  //no free block found  

    //split blocks down to desired level
    while (k > e){
        k--;
        split_block(p, block, k + 1);
    }

    //mark block as allocated in bitmap
    int index = e - p->l;

    //update buddy bitmap to indicate this block is now allocated
    if(bbmtst(p->buddy_bitmaps[index], p->base, block, e)){
        //bit was 1, set it to 0
        bbmclr(p->buddy_bitmaps[index], p->base, block, e);
    } else {
        //bit was 0, set it to 1
        bbmset(p->buddy_bitmaps[index], p->base, block, e);
    }

    //mark this specific block as allocated in alloc bitmap
    bbmset(p->alloc_bitmaps[index], p->base, block, e);

    return block;
    
}

/*  (1) determine block's size by checking bmap
    (2) mark as free
    (3) attempt to coalesce w/buddy:
        while buddy is also free:
            remove buddy from free list
            clear buddy bit in bitmap
            merge into larger block
            move up to next level and repeat
    (4) add final block to appropriate free list
*/
extern void  bfree(Balloc pool, void *mem){
    Pool *p = pool;

    if (mem == NULL || mem < p->base || mem >= p->base + p->size)
        return; //invalid pointer, ignore

    //determine block size by checking alloc bitmap
    int e = -1;
    for(int level = p->l; level <= p->u; level++){
        int index = level - p->l;
        if (bbmtst(p->alloc_bitmaps[index], p->base, mem, level)){
            e = level;
            break;
        }
    }

    if (e == -1)
        fprintf(stderr, "Error: Attempt to free unallocated block at %p\n", mem);
        return; //block not found in alloc bitmap, ignore

    //clear allocation bit
    int index = e - p->l;
    bbmclr(p->alloc_bitmaps[index], p->base, mem, e);

    //try to coalesce with buddy
    while (e < p->u){
        index = e - p->l;

        if (bbmtst(p->buddy_bitmaps[index], p->base, mem, e)){
            //buddy is allocated, can't coalesce
            bbmclr(p->buddy_bitmaps[index], p->base, mem, e);
            break;
        } else {
            //buddy is free, coalesce
            void *buddy = baddrinv(p->base, mem, e);
            
            //remove buddy from free list
            void **lists = p->freelists;
            void **head = &lists[index];
            void **current = head;

            int found = 0;
            while (*current != NULL){
                if (*current == buddy){
                    //found buddy, remove from list
                    *current = *(void **)(*current);
                    found = 1;
                    break;
                }
                current = (void **)(*current);
            }

            if (!found){
                fprintf(stderr, "Error: Buddy block at %p not found in free list during coalescing\n", buddy);
                bbmset(p->buddy_bitmaps[index], p->base, mem, e); //restore buddy bit since we couldn't coalesce    
                break;
            }

            if (buddy < mem){
                mem = buddy; //lower address becomes new block
            }

            //more to next
            e++;
        }
    }

    //add block to free list for final level
    freelistfree(p->freelists, p->base, mem, e, p->l);
}

/*  (1) start @ 1, check each bmap
    (2) find the level where this block is allocated, and return size of block (2^e)
    (3) return 0 if block is not allocated
*/
extern unsigned int bsize(Balloc pool, void *mem){
    Pool *p = pool;

    if (mem == NULL || mem < p->base || mem >= p->base + p->size)
        return 0; //invalid pointer, return 0
    
    //check each level's alloc bitmap to find block size
    for (int e = p->l; e <= p->u; e++){
        int index = e - p->l;
        if (bbmtst(p->alloc_bitmaps[index], p->base, mem, e)){
            return e2size(e); //block found, return size
        }
    }
    return 0;
}

/*  (1) print pool info: base address, total size, min and max block sizes
    (2) for each level from l to u:
        print level number and block size (2^e)
        print bitmap for that level
        print addresses of free blocks in that level's free list
    (3) return nothing
*/
extern void bprint(Balloc pool){
    Pool *p = pool;

    printf("Buddy Allocator:\n");
    printf("Base address: %p\n", p->base);
    printf("Total size: %lu bytes\n", p->size);
    printf("Min block size: %lu bytes (2^%d)\n", e2size(p->l), p->l);
    printf("Max block size: %lu bytes (2^%d)\n", e2size(p->u), p->u);
    printf("\n");

    printf("Free Lists:\n");
    freelistprint(p->freelists, p->l, p->u);
    printf("\n");

    printf("Buddy Bitmaps:\n");
    for (int e = p->l; e <= p->u; e++){
        int index = e - p->l;
        printf("Level %d (block size %lu): ", e, e2size(e));
        bbmprt(p->buddy_bitmaps[index]);
    }
    printf("\n");

    printf("Allocation Bitmaps:\n");
    for (int e = p->l; e <= p->u; e++){
        int index = e - p->l;
        printf("Level %d (block size %lu): ", e, e2size(e));
        bbmprt(p->alloc_bitmaps[index]);
    }
}
