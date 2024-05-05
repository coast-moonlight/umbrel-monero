#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cc.h"
#include "fnr.h"
#include "fnr_cc.h"

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
uint8_t rcv_bit(void *addr) {
    int misses = 0;
    int hits = 0;

    // sync with the sender and take the returned timestamp as start time
    // for INTERVAL clock cycles since start
    // measure the access time to addr
    // if the access time is above THRESHOLD
    // then increment misses
    // otherwise increment hits

	uint64_t start_time = cc_sync();

	while (rdtsc() - start_time < INTERVAL) {
		uint64_t curr_time = rdtsc();
		maccess(addr);
		uint64_t access_time = rdtsc() - curr_time;
		if (access_time > THRESHOLD) {
			misses++;
		} else {
			hits++;
		}
	}

    return misses >= hits; // decode as '1' bit, if more or equal misses than hits
}

int main() {
    // register handler for CTRL+C
    signal(SIGINT, handleSIGINT);

    // open file and map it into the address space as shared memory in read only mode
    int fd = open(FILE_NAME, O_RDONLY);
    void *addr = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, 0);

    // buffer for secret message
    uint8_t rcv_buf[2048] = { 0 };

	uint8_t seq = 0;
	printf("Listening...\n");
    while (!terminate) {
        // after each bit check whether the received bit sequence matches the start sequence
		while (!terminate && seq != 0xab) {
			seq <<= 1;
			// receive bits and store them in a variable
			seq |= rcv_bit(addr);
		}

        // if so, start receiving the message bits
		int i;
		for (i = 0; !terminate && i < sizeof(rcv_buf) - 1; i++) {
			uint8_t byte = 0;
			for (int j = 0; j < 8; j++) {
				byte <<= 1;
				byte |= rcv_bit(addr);
			}

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

    // unmap shared memory and close file
    munmap(addr, 4096);
    close(fd);
    return 0;
}
