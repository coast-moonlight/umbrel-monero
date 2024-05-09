#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cc.h"
#include "pnp.h"
#include "pnp_cc.h"

// handler for CTRL+C
static volatile int terminate = 0;
void handleSIGINT(int dummy) {
    terminate = 1;
}

/**
 * Send a message bit
 *
 * A '1' bit is transmitted by repetedly accessing eviction set.
 * A '0' bit is transmitted by doing nothing.
 *
 * IN: (uint8_t) the message bit
 */
void snd_bit(void **addr, uint8_t bit) {
    // synchronize with the receiver
    // if the bit is 1
    // wait INTERVAL cycles for the receiver to prime the cache set
    // the prime the cache set with your eviction set for INTERVAL cycles
    // then wait another INTERVAL cycles for the receiver to probe the cache set
    // if the bit is 0
    // do nothing for 3*INTERVAL cycles

	uint64_t start_time = cc_sync();

	if (bit) {
		while (rdtsc() - start_time < INTERVAL) {}

		while (rdtsc() - start_time < 2 * INTERVAL) {
			maccess_list(addr);
		}

		while (rdtsc() - start_time < 3 * INTERVAL) {}
	} else {
		while (rdtsc() - start_time < 3 * INTERVAL) {}
	}
}

// Main function
int main() {
    // register handler for CTRL+C
    signal(SIGINT, handleSIGINT);

    // construct an eviction set for cache set CACHE_SET in the L1D
    // to do so, you can allocate 4KB pages using the following function call:
	int page_count = 32;
    void **page = mmap(NULL, page_count * 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
	for (int i = 0; i < 64 * page_count; i++) {
		if (i + 1 < 64 * page_count) {
			page[i] = &page[64 * (i + 1)];
		} else {
			page[i] = NULL;
		}
	}

    printf("Please enter your message:\n");
    while(!terminate) {
        // wait for user input
        printf("< ");
        char input_buf[128];
        fgets(input_buf, sizeof(input_buf), stdin);
        size_t input_buf_len = strlen(input_buf) - 1;
        if (strncmp(input_buf, "exit", 4) == 0) {
            terminate = 1;
        }

        // define a 8 bit start sequence
		uint8_t start_seq = 0xab;

        // send the start sequence bit by bit
		for (int i = 0; i < 8; i++) {
			uint8_t bit = ((start_seq >> (7 - i)) & 1);
			snd_bit(page, bit);
		}

		for (int i = 0; input_buf[i]; i++) {
			for (int j = 0; j < 8; j++) {
				// convert the input string to bits
				uint8_t bit = ((input_buf[i] >> (7 - j)) & 1);
				// then send the message bit by bit
				snd_bit(page, bit);
			}
		}

        // free the message bit array
		// > We didn't allocate any memory, so no free :)
    }

    // free all memory regions allocated by calling malloc()
    // unmap all memory regions allocated by calling mmap()

	munmap(page, page_count * 64);
    return 0;
}
