/* Author: Zella Running
 * Description: Maintains a free list for each block size. Stores pointers in the forst bytes of free blocks, and each free block points to the next free block of same size.
 * Class: CS 451 - HW2 Memory Allocator w/Buddy System
 * Date: 2026 Febuary 11
 */

 #ifndef FREELIST_H
#define FREELIST_H

#include <stdio.h>

typedef void *FreeList;

extern FreeList freelistcreate(size_t size, int l, int u);
extern void     freelistdelete(FreeList f, int l, int u);

extern void *freelistalloc(FreeList f, void *base, int e, int l);
extern void  freelistfree(FreeList f, void *base, void *mem, int e, int l);

extern int freelistsize(FreeList f, void *base, void *mem, int l, int u);
extern void freelistprint(FreeList f, int l, int u);

#endif
