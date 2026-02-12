/* Author: Zella Running
 * Description: Memory allocation vis mmap, w/math utilities for the buddy system, and bit manipulation utilities for the free list.
 * Class: CS 451 - HW2 Memory Allocator w/Buddy System
 * Date: 2026 February 11
 */
#include "utils.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>

/*  (1) round size up to multiple of page size
    (2) call mmap to allocate memory, with appropriate flags for anonymous private mapping
    (3) return pointer to allocated memory, or (void *)-1 on failure
*/
extern void *mmalloc(size_t size){
    void *p = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (p == MAP_FAILED){
        return (void*)-1;
    }
    return p;
}

/*  (1) call munmap to free memory, with appropriate size
    (2) return nothing
*/
extern void mmfree(void *p, size_t size){
    munmap(p, size);
}

/*  (1) compute n/d, rounding up to next integer if there's a remainder
    (2) return result
*/
extern size_t divup(size_t n, size_t d){
    return (n + d - 1) / d;
}

/*  (1) compute number of bytes needed to store
    (2) return result
*/  
extern size_t bits2bytes(size_t bits){
    return divup(bits, bitsperbyte);
}

/*  (1) compute 2^e
    (2) return result
*/
extern size_t e2size(int e){
    return 1UL << e;
}

/*  (1) start with e = 0
    (2) while 2^e < size, increment e
    (3) return e
*/
extern int size2e(size_t size){
    int e = 0;
    size_t s = s;
    while(2^e < size){
        s <<= 1;
        e++;
    }
    return e;
}

/*  (1) make pointer to byte
    (2) find bit number
    (3) set bit to 1 using OR opp
*/
extern void bitset(void *p, int bit){
    unsigned char *byte = (unsigned char *)p;
    *byte |= (1 << bit);
}

/*  (1) make pointer to byte
    (2) find bit number
    (3) set bit to 0 using AND with inverse mask
*/
extern void bitclr(void *p, int bit){
    unsigned char *byte = (unsigned char *)p;
    *byte &= ~(1 << bit);
}

/*  (1) make pointer to byte
    (2) find bit number
    (3) toggle bit using XOR
*/
extern void bitinv(void *p, int bit){
    unsigned char *byte = (unsigned char *)p;
    *byte ^= (1 << bit);
}

/*  (1) make pointer to byte
    (2) find bit number
    (3) test bit using AND and return 0 or 1
*/
extern int  bittst(void *p, int bit){
    unsigned char *byte = (unsigned char *)p;
    return *byte & (1 << bit);
}
