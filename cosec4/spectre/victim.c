#include <stdlib.h>
#include <stdint.h>
#include "spectre.h"

unsigned int array_size = 16;
uint8_t unused1[64]; // Used to push array_size to different cacheline than array
uint8_t array[16] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };

char *secret = "COSEC{Stealing_data_with_Spectre!}";

uint8_t victim_function(size_t x) {
    if (x < array_size) {
		lfence();
        return array[x];
    }
}

size_t get_offset() {
    return (size_t)(secret - (char*)array);
}
