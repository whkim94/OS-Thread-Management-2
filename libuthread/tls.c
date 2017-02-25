#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

//#define _UTHREAD_PRIVATE
#include "palloc.h"
#include "uthread.h"
#include "tls.h"

struct node {
	int startpos;
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
	mytls = malloc(sizeof(struct tls));
	mytls->addr = palloc_get_pages(npages);
	if (mytls->addr == NULL)	return -1;
	mytls->blocks = malloc(npages * 4096);
	if (mytls->blocks == NULL)	return -1;
	mytls->blockSize = npages*4096;
	return 0;
}

int tls_destroy(void)
{
	printf(" ");
	free(mytls->blocks);
	free(mytls);
	if(mytls == NULL) return 0;
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
	struct node* newNode = malloc(sizeof(newNode));
	int end=0;
	int count = 0;
	int index = -1;
	while (end != mytls->blockSize) {
		if (mytls->blocks[end] == 0) {
			count++;
			if (count == size) {
				index = end-count+1;
				int j;
				for (j=end;j>=index;j--) mytls->blocks[j] = 1;
				break;
			}
		}
		else 	count = 0;
		end++;
	}

	if (index == -1) {
		free(newNode);
		return NULL;
	}
	newNode->ndata = mytls->addr + index;
	newNode->startpos = index;
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
}

int tls_free(void *ptr)
{
	int pageSize = 0;
	struct node* iter = mytls->start;
	while (iter != NULL) {
		if (iter->ndata == ptr) {
			pageSize = iter->pageNum;
			for (int i = iter->startpos; i<pageSize; i++) {
				mytls->blocks[i] = 0;
			}
			break;
		}
		iter = iter->next;
	}
	if (pageSize == 0) return -1;
	return 0;
}