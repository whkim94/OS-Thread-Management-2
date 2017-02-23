#include <assert.h>
#include <signal.h>
#include <sys/mman.h>
#include <stdlib.h>
//#define _UTHREAD_PRIVATE
#include "bitmap.h"
#include "palloc.h"
#include "preempt.h"

struct palloc {
	size_t npages;	/* Number of physical pages */
	bitmap_t pagemap;
	void *addr;
};

/* Pool of memory pages */
static struct palloc ppool;

void palloc_configure(size_t npages)
{
	
/*
 * palloc_configure - Configure the memory page allocator
 * @npages: Number of pages
 *
 * Setup the number of available pages in the system
 */
	ppool.npages = npages;
	ppool.pagemap = bitmap_create(npages);
	int status = palloc_create();
}

int palloc_create(void)
{
	/* TODO: PART 2 - Phase 2 */
	/*
 * palloc_create - Create the memory page allocator
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
 
	unsigned char* temp = getMemory(ppool.pagemap);  
 	void *ptr = mmap(temp, ppool.npages*PAGE_SIZE, PAGE_RW_ACCESS, MAP_ANONYMOUS, -1, 0);
 	ppool.addr = ptr;
 	if(ptr == NULL)
 		return -1;
 	//void *mmap(addr, size_t length, int prot, int flags, int fd  -1, off_t offset 0 );
	//void *mmap(addr, length, prot, flags, -1, 0 );
	return 0;
}

int palloc_destroy(void)
{
	/* TODO: PART 2 - Phase 2 */
	
/*
 * palloc_destroy - Destory the memory page allocator
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
	unsigned char* temp = getMemory(ppool.pagemap);  
	return munmap(temp, ppool.npages*PAGE_SIZE);
}

void *palloc_get_pages(size_t count)
{
	
/*
 * palloc_get_pages - Get a set of memory pages
 * @count: Number of pages required
 *
 * Return: Address of a memory area containing @count free pages, or NULL in
 * case of failure
 */

	if(bitmap_full(ppool.pagemap))
		return NULL;
	if(bitmap_empty(ppool.pagemap))
		return ppool.addr;
	size_t *index;
	if(bitmap_find_region(ppool.pagemap, count, index)) {
		return ppool.addr + ((*index)*PAGE_SIZE);
	}

	return NULL;

}

void palloc_free_pages(void *ptr, size_t count)
{
	/* TODO: PART 2 - Phase 2 */
	/*
 * palloc_free_pages - Free a set of memory pages
 * @ptr: Address of the memory area containing the pages
 * @count: Number of consecutive pages to free
 */

	// int* temp1 = (ppool.addr);
	// int* temp2 = &(ptr);
	int diff = &(ppool.addr) - &(ptr);
	int index = diff/PAGE_SIZE;
	bitmap_clr(ppool.pagemap, (size_t)index, count);
}

int palloc_protect_pages(void *ptr, size_t count, enum page_protection prot)
{
	/* TODO: PART 2 - Phase 3 */
	/*
 * palloc_protect_pages - Protect a set of memory pages
 * @ptr: Address of the memory area to protect
 * @count: Number of consecutive pages to protect
 * @prot: Protection to apply to the set of pages
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
	if (!mprotect(ptr, count, prot))
		return 0; //that's it
	return -1;
}

