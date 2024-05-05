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
 * Send a message bit
 *
 * A '1' bit is transmitted by repetedly flushing the shared address.
 * A '0' bit is transmitted by doing nothing.
 *
 * IN: - (void *) a memory address shared with the receiver
 *     - (uint8_t) the message bit
 */
void snd_bit(void *addr, uint8_t bit) {
    // synchronize with the receiver
    // and store the returned timestamp counter as start time
    // then send bit:
    // if bit is 1
    // then continuously flush addr for INTERVAL clock cycles since start time
    // else do nothing for INTERVAL clock cycles since start time

	uint64_t start_time = cc_sync();

	if (bit) {
		while (rdtsc() - start_time < INTERVAL) {
			flush(addr);
		}
	} else {
		while (rdtsc() - start_time < INTERVAL) {}
	}
}

// Main function
int main() {
    // register handler for CTRL+C
    signal(SIGINT, handleSIGINT);

    // open file and map it into the address space as shared memory in read only mode
    int fd = open(FILE_NAME, O_RDONLY);
    void *shared_addr = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, 0);

    int i, j;
    printf("Please enter your message:\n");
    while(!terminate) {
        // read user input
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
		for (i = 0; i < 8; i++) {
			uint8_t bit = ((start_seq >> (7 - i)) & 1);
			snd_bit(shared_addr, bit);
		}

		for (i = 0; input_buf[i]; i++) {
			for (j = 0; j < 8; j++) {
				// convert the input string to bits
				uint8_t bit = ((input_buf[i] >> (7 - j)) & 1);
				// then send the message bit by bit
				snd_bit(shared_addr, bit);
			}
		}

        // free the message bit array
		// > We didn't allocate any memory, so no free :)
    }

    // unmap shared memory and close file
    munmap(shared_addr, 4096);
    close(fd);
    return 0;
}
