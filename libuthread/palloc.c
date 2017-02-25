#include <assert.h>
#include <signal.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
//#define _UTHREAD_PRIVATE
#include "bitmap.h"
#include "palloc.h"


struct palloc {
	size_t npages;	/* Number of physical pages */
	bitmap_t pagemap;
	void *addr;
};

/* Pool of memory pages */
static struct palloc ppool;

void palloc_configure(size_t npages)
{
	ppool.npages = npages;
}

int palloc_create(void)
{
 	ppool.pagemap = bitmap_create(ppool.npages);
 	void *ptr = mmap(NULL, ppool.npages*PAGE_SIZE, PROT_NONE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
 	ppool.addr = ptr;
 	if(ptr == NULL)
 		return -1;
	return 0;
}

int palloc_destroy(void)
{
	return munmap(ppool.addr, ppool.npages*PAGE_SIZE);
}

void *palloc_get_pages(size_t count)
{
	if(bitmap_full(ppool.pagemap))	return NULL;
	size_t *index;
	if(bitmap_find_region(ppool.pagemap, count, index)) return ppool.addr + ((*index)*PAGE_SIZE);
	return NULL;
}

void palloc_free_pages(void *ptr, size_t count)
{
	int diff = (ppool.addr) - (ptr);
	int index = diff/PAGE_SIZE + 1;
	bitmap_clr(ppool.pagemap, (size_t)index, count);
}

int palloc_protect_pages(void *ptr, size_t count, enum page_protection prot)
{
	if (!mprotect(ptr, count, prot)) return 0; //that's it
	return -1;
}