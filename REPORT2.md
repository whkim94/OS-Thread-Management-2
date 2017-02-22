# Project 3

### bitmap API
In our bitmap constructer, we have the following variables:

- size: record the total bits in the bitmmap
- size_avail: record how many bits are still available to be set
- memory: the array of bits

In addition, we created a check_bit() to check if a specific bit is set.

1. In create(): we set the size and size_avail to the parameter nbits, then we malloc() for bitmap->memory.
2. In empty()/full(): we use the size_avail to check for available bits, making it O(1).
3. In any()/none()/all(): we go thourgh the array of bits and immidiately return if an opposite condition is met, making it O(n).
4. In set()/clr()/set_one()/cle_one(): we first check for error cases, then use the given macros to set the bits. We also modify the size_avail accordingly.
5. In find_region(): we set a variable bits_avail = 0. We then loop through the array to see if there's a clear bit, and increment the bits_avail if that's the case, or reset it to 0 if we find a set bit. Once bit_avail is equal to the required space, we set the corresponding bits, set the position of the starting bits and return a success.

### palloc API
1. create(): we used mmap() to link the bitmap to the actual allocated address
2. destroy(): we use munmap() to free the memories used
3. get_pages(): 
4. free_pages():
5. protect_pages(): 
