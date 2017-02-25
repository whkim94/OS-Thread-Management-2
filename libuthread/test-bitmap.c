
#include <stdlib.h>
#include <stdio.h>

#include "uthread.h"
#include "bitmap.h"
#include "queue.h"

void test_status(bitmap_t mybitmap) {

	int test_empty = bitmap_empty(mybitmap);
	printf("Is the bitmap empty? Ans: %d\n",test_empty);
	int test_full = bitmap_full(mybitmap);
	printf("Is the bitmap full? Ans: %d\n",test_full);
	int test_any = bitmap_any(mybitmap,4,2);
	printf("Is there set bits at pos: 4 and 6 bits after? Ans: %d\n",test_any);
	int test_none = bitmap_none(mybitmap,5,2);
	printf("Is there non-set bits at pos: 4 and 6 bits after?Ans: %d\n",test_none);
	int test_all = bitmap_all(mybitmap,1,4);
	printf("Are all bits from 1 to 4 set to 1? Ans: %d\n",test_all);
	}

int main() {
	//typedef struct bitmap* bitmap_t;
	printf("Created a new bitmap.\n");
	bitmap_t bitmapA = bitmap_create(8);
	test_status(bitmapA);
	
	printf("\nSet bits at pos 3 and 6 bits after.\n");
	//bitmap_set_one(bitmapA,3);
	bitmap_set(bitmapA,3,2);
	//bitmap_set_one(bitmapA, 4);
	test_status(bitmapA);

	printf("\nClearing the previously-set bits.\n");
	//bitmap_clr_one(bitmapA,3);
	bitmap_clr(bitmapA,3,2);
	//bitmap_clr_one(bitmapA,4);
	test_status(bitmapA);

	printf("\nFilling all bits in the bitmap.\n");
	//bitmap_set_one(bitmapA,3);
	bitmap_set(bitmapA,1,4);
	test_status(bitmapA);
	
	
	size_t *ptr;
	bitmap_find_region(bitmapA, 3, ptr);
	printf("print value of ptr = %d\n", *ptr);

	bitmap_destroy(bitmapA);
	return 0;
}