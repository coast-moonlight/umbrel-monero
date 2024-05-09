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
 * Receive a message bit
 *
 * A '1' bit is received if more or equal misses than hits occur.
 * A '0' bit is received if less misses than hits occur.
 *
 * IN: (void *) a memory address shared with the receiver
 * RET: (uint8_t) the received bit
 */
uint8_t rcv_bit(void *ev_start)
{
    // sync with sender
	uint64_t start_time = rdtsc();

    // prime cache for INTERVAL cycles
	while (rdtsc() - start_time < INTERVAL) {
		maccess_list(ev_start);
	}

    // wait for sender to access cache sets for INTERVAL cycles
	while (rdtsc() - start_time < 2 * INTERVAL) {}

    // probe cache
	start_time = rdtsc();
	maccess_list(ev_start);
	uint64_t access_time = rdtsc() - start_time;

	while (rdtsc() - start_time < 3 * INTERVAL) {}

    // if access time is above threshold return 1
	if (access_time > THRESHOLD) {
		return 1;
	}

    // else return 0;
    return 0;
}

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

	uint8_t seq = 0;

    // buffer for secret message
    char rcv_buf[1024] = { 0 };
    printf("Listening...\n");
    while (!terminate) {
        // after each bit check whether the received bit sequence matches the start sequence
		while (!terminate && seq != 0xab) {
			seq <<= 1;
			// receive bits and store them in a variable
			seq |= rcv_bit(page);
		}

        // if so, start receiving the message bits
		int i;
		for (i = 0; !terminate && i < sizeof(rcv_buf) - 1; i++) {
			uint8_t byte = 0;
			for (int j = 0; j < 8; j++) {
				byte <<= 1;
				byte |= rcv_bit(page);
			}

			printf("received '%c'\n", byte);
			rcv_buf[i] = byte;
			// until you receive 8 0-bits in a row (end of string)
			if (byte == 0) {
				break;
			}
		}

        // then convert the bits to a string and print it
		printf("%s\n", rcv_buf);
		seq = 0;
    }

    // free all memory regions allocated by calling malloc()
    // unmap all memory regions allocated by calling mmap()

	munmap(page, page_count * 64);
    return 0;
}
