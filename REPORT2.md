# Project 3

### bitmap API

##### main functions
1. In our bitmap constructer, we have the following variables:
  - size: record the total amount of bits in the bitmmap
  - size_avail: record how many bits are still available to be set
  - memory: the array of bits
2. create(): we set size and size_avail to the parameter [nbits], then we malloc() for memory.
3. destroy(): we free the array of bits first, then we free the entire bitmap struct.
4. empty()/full(): we use the size_avail to check for available bits, saving time by making it O(1).
5. any()/none()/all(): we go thourgh the array of bits and immidiately return if an invalid condition is met, making it O(n). Otherwise all bits meet the valid condition and return success
6. set()/clr()/set_one()/cle_one(): we first check for error cases, then use the given macros to set the bits. We also modify the size_avail accordingly.
7. In find_region(): we set a variable bits_avail = 0. We then loop through the array to see if there's a clear bit, and increment the bits_avail if that's the case, or reset it to 0 if we find a set bit. Once bit_avail is equal to the required space, we set the corresponding bits, set the position of the starting bits and return a success.

##### custom functions
1. check_bit(int nr, volatile unsigned char *map): We check if the @nr bit of @map is set to 1. Return 1 if it's set and 0 if not.
2. getMemory(bitmap_t bitmap): We return the array of bits of @bitmap.

### palloc API
1. create(): we used mmap() to link the bitmap to the actual allocated address with protection PAGE_WR_ACCESS
2. destroy(): we use munmap() to free the memories used
3. get_pages(): after checking for error cases, we used bitmap_find_region() to get the index of the page, then we return (the starting address ppool + index * PAGE_SIZE)
4. free_pages(): we first find the difference between the address of the ptr passed in, and the address of the start of ppool.addr. Using the difference, we divide it by PAGE_SIZE to determine which page the ptr corresponds to and use bitmap_clr() to clear the corresponding bits.
5. protect_pages(): we used mprotect() to set the memory protection as PAGE_NO_AcCESS

### tls API
1. In our tls structure, we have the following variables
  - blocks: the map of what bytes are available
	- blockSize: the amount of pages allocated for tls
	- addr: the starting address of the actual data for tls
2. create():
  1. we malloc() for the tls structure
  2. we set the addr topointer returned by palloc_get_pages(npages)
  3. we malloc() for blocks
  4. we set blockSize to @npages passed in by user
3. destroy(): we free() all variables inside of the tls structure.
4. open():
5. close():
6. alloc():
  1. we check inside blocks to see if there's enough space to allocate @size Bytes
  2. if there is, then we return the address of (addr+index of free space found)
7. free():
  1. we check if the address is in the list of allocated blocks
    - if not, return -1
    - if yes, clear the record in blocks and return 0
