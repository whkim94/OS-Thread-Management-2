# Project 3

### bitmap API
In our bitmap constructer, we have the following variables:

- size: record the total bits in the bitmmap
- size_avail: record how many bits are still available to be set
- memory: the array of bits

In addition, we created a check_bit() to check if a specific bit is set.

In create(): we set the size and size_avail to the parameter nbits, then we malloc() for bitmap->memory.

In empty()/full(): we use the size_avail to check for available bits, making it O(1).

In any()/none()/all(): we go thourgh the array of bits and immidiately return if an opposite condition is met, making it O(n).

In set()/clr()/set_one()/cle_one(): we first check for error cases, then use the given macros to set the bits. We also modify the size_avail accordingly.

In find_region(): we set a variable bits_avail = 0. We then loop through the array to see if there's a clear bit, and increment the bits_avail if that's the case, or reset it to 0 if we find a set bit. Once bit_avail is equal to the required space, we set the corresponding bits, set the position of the starting bits and return a success.
