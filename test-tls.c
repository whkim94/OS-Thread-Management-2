/*
 * Thread TLS test
 *
 * A thread allocates some data in its TLS and checks that after deallocation of
 * a block, asking for two smaller blocks of half the size will use the space
 * occupied by the former block. It also checks that after freeing the two
 * adjacent small blocks, they are merged into a bigger block.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>
#include <tls.h>

#define PAGES_PER_THREAD 100

void check_ptr(void *p1, void *p2)
{
	if (p1 != p2) {
		printf("Error: expected %p, got %p\n", p1, p2);
		exit(1);
	}
}

void thread1(void* arg)
{
	int i;
	void *array[3], *old;

	tls_create(PAGES_PER_THREAD);

	array[0] = tls_alloc(64);
	array[1] = tls_alloc(64);

	old = array[0];
	tls_free(array[0]);

	array[0] = tls_alloc(32);
	array[2] = tls_alloc(32);

	

	check_ptr(array[0], old);
	check_ptr(array[2], old + 32);

	tls_free(array[0]);
	tls_free(array[2]);

	array[0] = tls_alloc(64);
	check_ptr(array[0], old);

	tls_destroy();

	printf("All good!\n");
}

#define TOTAL_UTHREAD_PAGES 1000

int main(void)
{
	uthread_mem_config(TOTAL_UTHREAD_PAGES);
	uthread_start(thread1, NULL);
	return 0;
}
