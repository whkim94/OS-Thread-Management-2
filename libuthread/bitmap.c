#include <assert.h>
#include <stdlib.h>

#include <stdio.h>
//#define _UTHREAD_PRIVATE
#include "bitmap.h"
#include "preempt.h"

/* Divide @n by @d and round up to the nearest @d unit */
#define DIV_ROUND_UP(n, d)	(((n) + (d) - 1) / (d))  // Used to estimate the number of byted it will take to store n bits.

#define BITS_PER_BYTE		8
#define BITS_TO_BYTES(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE) // NUmber of bytes needed to store nr bits

#define BIT(nr)			(1 << (nr)) //
#define BIT_MASK(nr)		(1 << ((nr) % BITS_PER_BYTE))
#define BIT_WORD(nr)		((nr) / BITS_PER_BYTE) // Gives us the byte where to look at. for 2 byte char array BIT_WORD(1) is byte 0.

static inline void set_bit(int nr, volatile unsigned char *map)
{
	unsigned char mask = BIT_MASK(nr);
	unsigned char *p = ((unsigned char*)map) + BIT_WORD(nr);
	sigset_t level;

	preempt_save(&level);
	*p |= mask;
	preempt_restore(&level);
}

static inline void clr_bit(int nr, volatile unsigned char *map)
{
	unsigned char mask = BIT_MASK(nr);
	unsigned char *p = ((unsigned char*)map) + BIT_WORD(nr);
	sigset_t level;

	preempt_save(&level);
	*p &= ~mask;
	preempt_restore(&level);
}

int check_bit(int nr, volatile unsigned char *map){
	unsigned char mask = BIT_MASK(nr);
	unsigned char *p = ((unsigned char*)map) + BIT_WORD(nr); // What byte constains that bit. 
	unsigned char p2;
	p2 = *p & mask;
	if(p2 == mask)
		return 1; // Bit is set
	return 0; // bit is not set
}

struct bitmap {
	
	int size;
	int size_avail;
	unsigned char* memory;
};

unsigned char * getMemory(bitmap_t bitmap){
	return bitmap->memory;	
}

bitmap_t bitmap_create(size_t nbits)
{
	bitmap_t newBitMap = malloc(sizeof(bitmap_t));
	if (newBitMap==NULL) //failed to malloc
		return NULL;
	newBitMap->size = nbits;
	newBitMap->size_avail = nbits;
	newBitMap->memory = malloc(BITS_TO_BYTES(nbits) * sizeof(unsigned char *));
	int k = 0;
	while (k++ != nbits)
		clr_bit(k, (newBitMap->memory));
	return newBitMap;
}

int bitmap_destroy(bitmap_t bitmap)
{
	free(bitmap->memory);
	if (bitmap->memory != NULL) //didn't destroy
		return -1;

	free(bitmap);
	if (bitmap==NULL) //didn't destroy
		return -1;

	return 0;
}

int bitmap_empty(bitmap_t bitmap)
{
	return (bitmap->size_avail == bitmap->size);
}

int bitmap_full(bitmap_t bitmap)
{
	return (bitmap->size_avail == 0);
}

int bitmap_any(bitmap_t bitmap, size_t pos, size_t nbits)
{
	size_t i = 0;
	for(i = pos + nbits - 1; i >= pos; i--)	{
		if(check_bit(i, bitmap->memory))
			return 1;
	}
	return 0;
}

int bitmap_none(bitmap_t bitmap, size_t pos, size_t nbits)
{
	size_t i = 0;
	for(i = pos + nbits - 1; i >= pos; i--)	{
		if(check_bit(i, bitmap->memory))
			return 0;
	}
	return 1;
}

int bitmap_all(bitmap_t bitmap, size_t pos, size_t nbits)
{
	size_t i = 0;
	for(i = pos + nbits - 1; i >= pos; i--)	{
		if(check_bit(i, bitmap->memory) == 0)
			return 0;
	}
	return 1;
}

int bitmap_set(bitmap_t bitmap, size_t pos, size_t nbits)
{
	if(pos > bitmap->size || pos+nbits > bitmap->size)
		return -1;
	if (bitmap->size_avail < nbits) //not enough bits available
		return -1;
	size_t i = 0;
	for(i = pos + nbits - 1; i >= pos; i--)	{
		bitmap_set_one(bitmap, i);
	}
	return 0;
}

int bitmap_clr(bitmap_t bitmap, size_t pos, size_t nbits)
{ 
	if(pos > bitmap->size || pos+nbits > bitmap->size)
		return -1;
	if (bitmap->size_avail < nbits) //not enough bits available
		return -1;
	size_t i = 0;
	for(i = pos + nbits - 1; i >= pos; i--)	{
		bitmap_clr_one(bitmap, i);
	}
	return 0;
}

int bitmap_set_one(bitmap_t bitmap, size_t pos)
{
	if (bitmap->size_avail < 1) //not enough bit available
		return -1;

	set_bit(pos,bitmap->memory);
	bitmap->size_avail -= 1;
	return 0;
}

int bitmap_clr_one(bitmap_t bitmap, size_t pos)
{
	if(pos > bitmap->size)
		return -1;
	
	clr_bit(pos,bitmap->memory);
	bitmap->size_avail += 1;
	return 0;
}

int bitmap_find_region(bitmap_t bitmap, size_t count, size_t *pos)
{
	int bits_avail = 0;
	size_t i;
	for (i=0;i<bitmap->size;i++) {
		if (!check_bit(i, bitmap->memory)) { //the bit isnt set
			bits_avail++;
			if (bits_avail == count) { //we have a region with enough bits to set
				bitmap_set(bitmap, i, count); //count,&(bitmap->memory[i]));
				*pos = i+1-count;
				return 0;
			}
		}
		else {
			bits_avail = 0; //found a set bit, restart at 0
		}
	}
	return -1; //we didn't find a region with enough bits to set
}

