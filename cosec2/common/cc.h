#include <stddef.h>
#include <stdint.h>

/**
 * Synchronize covert channel sender and receiver
 * RET: (uint64_t) timestamp counter after synchronization
 */
uint64_t cc_sync();

/**
 * Encode a string as binary array
 * IN: - (char *) string to encode
 *     - (size_t) length of the string
 * RET: (uint8_t *) array of length 8*(len+1) containing the big endian binary representation of the string
 */
uint8_t *str2bin(char *str, size_t len);

/**
 * Decode a binary array to a string
 * IN: - (uint8_t *) binary array
 *     - (size_t) length of the array (must be an integer multiple of 8)
 * RET: (char *) string representation of the binary array with length len/8
 */
char *bin2str(uint8_t *bin, size_t len);
