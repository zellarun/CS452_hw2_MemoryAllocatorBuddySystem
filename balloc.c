/* Author: Zella Running
 * Description: Main allocator implementation. Manages the pool structure, implements splitting and coalescing, and coordinates all the modules.
 * Class: CS 451 - HW2 Memory Allocator w/Buddy System
 * Date: 2026 Febuary 11
 */

#ifndef BALLOC_H
#define BALLOC_H

typedef void *Balloc;

typedef struct {
    void *base;             //base address of mem. pool
    size_t size;            //total size of mem. pool
    int l, u;               //min exponent and max exponent of block sizes
    FreeList *freelists;    //array of free lists, one for each block size
    BBM *bitmaps;           //array of buddy bitmaps, one for each block size
} Pool;

extern Balloc bcreate(unsigned int size, int l, int u){
    return NULL;
}
extern void   bdelete(Balloc pool){

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
    return NULL;
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

}

/*  (1) start @ 1, check each bmap
    (2) find the level where this block is allocated, and return size of block (2^e)
    (3) return 0 if block is not allocated
*/
extern unsigned int bsize(Balloc pool, void *mem){
    return 0;
}
extern void bprint(Balloc pool){

}

#endif