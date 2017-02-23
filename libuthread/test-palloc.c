#include <stdlib.h>
#include <stdio.h>

#include "uthread.h"
#include "bitmap.h"
#include "queue.h"
#include "palloc.h"

int main() {
	printf("Hello world!\n");
	palloc_configure(10);

	int* myPages = palloc_get_pages(5);

	palloc_free_pages(myPages,4);

	palloc_destroy();
	printf("Goodbye you little piece of shit world!\n");
	return 0;
}