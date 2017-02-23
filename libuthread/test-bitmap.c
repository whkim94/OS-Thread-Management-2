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
	int test_any = bitmap_any(mybitmap,10,20);
	printf("Is there set bits at pos:10 and 20 bits after? Ans: %d\n",test_any);
	int test_none = bitmap_none(mybitmap,10,20);
	printf("Is there non-set bits at pos:10 and 20 bits after? Ans: %d\n",test_any);
}

int main() {
	
	printf("Created a new bitmap.\n");
	bitmap_t bitmapA = bitmap_create(64);
	test_status(bitmapA);
	
	printf("Set bits at pos 6 and 16 bits after.\n");
	//bitmap_set_one(bitmapA,3);
	bitmap_set(bitmapA,6,16);
	test_status(bitmapA);

	printf("Clearing the previously-set bits.\n");
	//bitmap_clr_one(bitmapA,3);
	bitmap_clr(bitmapA,6,16);
	test_status(bitmapA);

	printf("Filling all bits in the bitmap.\n");
	//bitmap_set_one(bitmapA,3);
	bitmap_set(bitmapA,0,64);
	test_status(bitmapA);

	bitmap_destroy(bitmapA);
	return 0;
}


