#include <stdint.h>

/**
 * Read timestamp counter
 * RET: (uint64_t) CPU timestamp value
 */
uint64_t rdtsc();

/**
 * Access a linked list of memory addresses
 * IN: (volatile void *) start of the linked list
 */
void maccess_list(volatile void *);
