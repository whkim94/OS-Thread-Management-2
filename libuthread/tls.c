#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

//#define _UTHREAD_PRIVATE
#include "palloc.h"
#include "uthread.h"
#include "tls.h"

struct node {
	void* ndata;
	int pageNum;
	struct node* next; 
};

struct tls {
	struct node* start;
	struct node* end;
	int* blocks;
	int blockSize;
	void* addr;
};

struct tls* mytls;

int tls_create(size_t npages)
{
	/*
 * tls_create - Create a new TLS area
 * @npages: Number of required pages
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
	mytls = malloc(sizeof(struct tls));

	mytls->addr = malloc(npages * 4096);
	if (mytls->addr == NULL)
		return -1;
	//uthread_set_tls(mytls->addr);
	
	//printf("size of npages = %d\n", npages);
	mytls->blocks = malloc(npages * sizeof(int));
	//printf("sizeof blocks = %d\n",sizeof(mytls->blocks));
	if (mytls->blocks == NULL)
		return -1;
	//int tempArray[npages] = {0};
	//mytls->blocks = tempArray;
	int blockSize = npages;

	return 0;
}

int tls_destroy(void)
{
	/*
 * tls_destory - Destroy TLS area
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
	free(mytls->blocks);
	free(mytls->addr);
	free(mytls);

	if(mytls == NULL)
		return 0;
	return -1;
}

void tls_open(void)
{
	/*
 * tls_open - Open the TLS area of the calling thread
 */

	//struct tls* currenttls = uthread_get_tls();
	//mprotect(currenttls, , prot);

}

void tls_close(void)
{
	/*
 * tls_close - Close the TLS area of the calling thread
 */
	//struct tls* currenttls = uthread_get_tls();
}

void *tls_alloc(size_t size)
{
	/*
 * tls_alloc - Allocate memory from TLS
 * @size: Size of memory to allocate in bytes
 *
 * Return: Address on the allocated memory area, or NULL in case of failure
 */
	//struct tls* currenttls = uthread_get_tls();

	printf("In tls_alloc\n");
	struct node* newNode = malloc(sizeof(newNode));
	newNode->ndata = palloc_get_pages(size);
	printf("palloc_get_pages() returned this address: %p\n",newNode->ndata);
	if (newNode->ndata == NULL) {
		printf("Error: Not enough space available.\n");
		free(newNode);
		return NULL;
	}
	newNode->pageNum = size;
	newNode->next = NULL;

	if (mytls->start == NULL) { //there's no allocated space yet
		mytls->start = newNode;
		mytls->end = newNode;
	}
	else { //We have allocated space before
		mytls->end->next = newNode;
		mytls->end = newNode;
	}

	return newNode->ndata;
	//old stuff-----------------------
	/*
	size_t count = 0;
	int space_found = 0;
	int index = 0;
	
	for (int i=0;i<mytls->blockSize;i++) {
		//printf("%d",mytls->blocks[i]);
		if (!mytls->blocks[i]) {
			count++;
			if (count == size) {
				space_found = 1;
				index = i;
				for (int j=(int)count;j>=(i-count);j--) {
					mytls->blocks[j] = 1;
				}
				break;
			}
		}
		else {
			count = 0;
		}
	}
	//printf("\n");
	if (space_found == 0) {
		printf("Error: Not enough space available.\n");
		return NULL;
	}
	*/

	//return (mytls->addr + (index * 4096));


}

int tls_free(void *ptr)
{
	/*
 * tls_free - Free memory
 * @ptr: Address of memory area to be freed
 *
 * Return: 0 in case of success, or -1 in case of failure
 */
	printf("In tls_free()\n");
	int pageSize = 0;
	struct node* iter = mytls->start;
	while (iter != NULL) {
		if (iter == ptr) {
			pageSize = iter->pageNum;
			break;
		}
		iter = iter->next;
	}
	if (pageSize == 0) {
		printf("ptr in tls_free() wasn't found.\n");
		return -1;
	}
	palloc_free_pages(ptr,pageSize);
	return 0;
	//printf("Address of ptr in free %p\n", ptr);

}
