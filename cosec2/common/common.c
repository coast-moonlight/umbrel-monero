#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

/**
 * Flush an address from the cache hierarchy
 * IN: (void *) address to be flushed
 */
extern inline __attribute__((always_inline))
void flush(void *addr) {
    __builtin_ia32_clflush(addr);
    _mm_lfence();
}

/**
 * Read timestamp counter
 * RET: (uint64_t) CPU timestamp value
 */
extern inline __attribute__((always_inline))
uint64_t rdtsc()
{
    _mm_lfence();
    return __rdtsc();
}

/**
 * Access a memory address
 * IN: (void *) memory address
 */
extern inline __attribute__((always_inline))
void maccess(void *addr) {
    __asm__ __volatile__ ("movq (%%rdi), %%rdi;" :: "D" (addr));
}

/**
 * Access a linked list of memory addresses
 * IN: (volatile void *) start of the linked list
 */
extern inline __attribute__((always_inline))
void maccess_list(volatile void *addr) {
    __asm__ __volatile__ (
        "maccess_list_loop:"
        "movq (%%rdx), %%rax;"
        "movq (%%rax), %%rcx;"
        "movq (%%rdx), %%rcx;"
        "movq (%%rax), %%rcx;"
        "movq (%%rdx), %%rcx;"
        "movq (%%rax), %%rcx;"
        "movq (%%rdx), %%rcx;"
        "movq (%%rax), %%rcx;"
        "movq %%rax, %%rdx;"
        "test %%rcx, %%rcx;"
        "jnz maccess_list_loop;"
        :
        : "d" (addr)
        : "rax", "rcx"
    );
}


#define SYNC_MASK 0xFFFFFF
#define SYNC_BOUND 0x0100

/**
 * Synchronize covert channel sender and receiver
 * RET: (uint64_t) timestamp counter after synchronization
 */
extern inline __attribute__((always_inline))
uint64_t cc_sync() {
    while((rdtsc() & SYNC_MASK) > SYNC_BOUND) {}
    return rdtsc();
}

/**
 * Encode a string as binary array
 * IN: - (char *) string to encode
 *     - (size_t) length of the string
 * RET: (uint8_t *) array of length 8*(len+1) containing the big endian binary representation of the string
 */
uint8_t *str2bin(char *str, size_t len) {
    uint8_t *bin = malloc(8*(len+1));
    memset(bin, 0, 8*(len+1));
    int i, j;
    for (i = 0; i < len; ++i) {
        for (j = 0; j < 8; ++j) {
            bin[8 * i + j] = (str[i] >> (7-j)) & 1;
        }
    }
    return bin;
}

/**
 * Decode a binary array to a string
 * IN: - (uint8_t *) binary array
 *     - (size_t) length of the array (must be an integer multiple of 8)
 * RET: (char *) string representation of the binary array with length len/8
 */
char *bin2str(uint8_t *bin, size_t len) {
    if ((len % 8) != 0) {
        return NULL;
    }
    char *string_buf = malloc((len + 8) / 8);
    memset(string_buf, 0, (len + 8) / 8);
    int i, j;
    for (i = 0; i < len/8; ++i) {
        for (j = 0; j < 8; ++j) {
            string_buf[i] <<= 1;
            string_buf[i] += bin[8 * i + j];
        }
    }
    return string_buf;
}
