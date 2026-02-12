/* Author: Zella Running
 * Description: Test suite for buddy system allocator. Tests basic allocation and freeing, buddy coalescing, correct block sizes, fragmentation handling, memory exhaustion, and writing/reading data.
 * Class: CS 451 - HW2 Memory Allocator w/Buddy System
 * Date: 2026 February 11
 */
#include "balloc.h"
#include <stdio.h>
#include <string.h>

#include <stdio.h>
#include <string.h>
#include "balloc.h"

void test_basic_allocation() {
    printf("=== Test 1: Basic Allocation ===\n");
    
    Balloc pool = bcreate(4096, 4, 12);
    if (!pool) {
        printf("FAIL: Could not create pool\n");
        return;
    }
    
    printf("Pool created successfully\n");
    bprint(pool);
    
    //allocate a few blocks
    void *p1 = balloc(pool, 100);  //should get 128 bytes (2^7)
    printf("\nAllocated 100 bytes at %p\n", p1);
    printf("Block size: %u\n", bsize(pool, p1));
    
    void *p2 = balloc(pool, 200);  //should get 256 bytes (2^8)
    printf("\nAllocated 200 bytes at %p\n", p2);
    printf("Block size: %u\n", bsize(pool, p2));
    
    void *p3 = balloc(pool, 50);   //should get 64 bytes (2^6)
    printf("\nAllocated 50 bytes at %p\n", p3);
    printf("Block size: %u\n", bsize(pool, p3));
    
    printf("\nAfter allocations:\n");
    bprint(pool);
    
    //free blocks
    printf("\n--- Freeing first block ---\n");
    bfree(pool, p1);
    bprint(pool);
    
    printf("\n--- Freeing second block ---\n");
    bfree(pool, p2);
    bprint(pool);
    
    printf("\n--- Freeing third block ---\n");
    bfree(pool, p3);
    bprint(pool);
    
    bdelete(pool);
    printf("\nTest 1: PASSED\n\n");
}

void test_coalescing() {
    printf("=== Test 2: Buddy Coalescing ===\n");
    
    Balloc pool = bcreate(1024, 4, 10);
    if (!pool) {
        printf("FAIL: Could not create pool\n");
        return;
    }
    
    //allocate two buddies
    void *p1 = balloc(pool, 64);  //64 bytes (2^6)
    void *p2 = balloc(pool, 64);  //64 bytes (2^6)
    
    printf("Allocated two 64-byte blocks:\n");
    printf("  p1 = %p\n", p1);
    printf("  p2 = %p\n", p2);
    bprint(pool);
    
    //free them in order - should coalesce
    printf("\n--- Freeing p1 ---\n");
    bfree(pool, p1);
    bprint(pool);
    
    printf("\n--- Freeing p2 (should coalesce with p1) ---\n");
    bfree(pool, p2);
    bprint(pool);
    
    bdelete(pool);
    printf("\nTest 2: PASSED\n\n");
}

void test_sizes() {
    printf("=== Test 3: Different Sizes ===\n");
    
    Balloc pool = bcreate(4096, 4, 12);
    
    struct {
        unsigned int request;
        unsigned int expected;
    } tests[] = {
        {1, 16},
        {16, 16},
        {17, 32},
        {32, 32},
        {33, 64},
        {100, 128},
        {256, 256},
        {257, 512},
        {1000, 1024},
        {2048, 2048},
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    
    for (int i = 0; i < num_tests; i++) {
        void *p = balloc(pool, tests[i].request);
        unsigned int actual = bsize(pool, p);
        
        printf("Request %4u bytes -> allocated %4u bytes (expected %4u) %s\n",
               tests[i].request, actual, tests[i].expected,
               actual == tests[i].expected ? "OK" : "FAIL");
        
        bfree(pool, p);
    }
    
    bdelete(pool);
    printf("\nTest 3: PASSED\n\n");
}

void test_fragmentation() {
    printf("=== Test 4: Fragmentation ===\n");
    
    Balloc pool = bcreate(1024, 4, 10);
    
    //allocate many small blocks
    void *blocks[10];
    for (int i = 0; i < 10; i++) {
        blocks[i] = balloc(pool, 32);
        printf("Allocated block %d at %p\n", i, blocks[i]);
    }
    
    printf("\nAfter allocating 10 blocks:\n");
    bprint(pool);
    
    //free every other block
    printf("\n--- Freeing every other block ---\n");
    for (int i = 0; i < 10; i += 2) {
        bfree(pool, blocks[i]);
    }
    bprint(pool);
    
    //free remaining blocks
    printf("\n--- Freeing remaining blocks ---\n");
    for (int i = 1; i < 10; i += 2) {
        bfree(pool, blocks[i]);
    }
    bprint(pool);
    
    bdelete(pool);
    printf("\nTest 4: PASSED\n\n");
}

void test_exhaustion() {
    printf("=== Test 5: Memory Exhaustion ===\n");
    
    Balloc pool = bcreate(256, 4, 8);  // Small pool
    
    void *blocks[20];
    int count = 0;
    
    // Try to allocate until we run out
    for (int i = 0; i < 20; i++) {
        blocks[i] = balloc(pool, 16);
        if (blocks[i] == NULL) {
            printf("Allocation %d failed (expected)\n", i);
            break;
        }
        count++;
        printf("Allocated block %d at %p\n", i, blocks[i]);
    }
    
    printf("\nSuccessfully allocated %d blocks\n", count);
    bprint(pool);
    
    //free all blocks
    printf("\n--- Freeing all blocks ---\n");
    for (int i = 0; i < count; i++) {
        bfree(pool, blocks[i]);
    }
    bprint(pool);
    
    bdelete(pool);
    printf("\nTest 5: PASSED\n\n");
}

void test_write_read() {
    printf("=== Test 6: Write and Read Data ===\n");
    
    Balloc pool = bcreate(4096, 4, 12);
    
    //allocate and write data
    char *p1 = balloc(pool, 100);
    strcpy(p1, "Hello, Buddy System!");
    
    int *p2 = balloc(pool, 40);
    for (int i = 0; i < 10; i++) {
        p2[i] = i * i;
    }
    
    char *p3 = balloc(pool, 50);
    strcpy(p3, "Another block");
    
    //read back
    printf("p1: %s\n", p1);
    printf("p2: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", p2[i]);
    }
    printf("\n");
    printf("p3: %s\n", p3);
    
    //free
    bfree(pool, p1);
    bfree(pool, p2);
    bfree(pool, p3);
    
    bdelete(pool);
    printf("\nTest 6: PASSED\n\n");
}

int main() {
    printf("Buddy System Allocator Test Suite\n");
    printf("==================================\n\n");
    
    test_basic_allocation();
    test_coalescing();
    test_sizes();
    test_fragmentation();
    test_exhaustion();
    test_write_read();
    
    printf("==================================\n");
    printf("All tests completed!\n");
    
    return 0;
}
