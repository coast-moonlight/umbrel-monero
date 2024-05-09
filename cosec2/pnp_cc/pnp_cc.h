// Bytes per cache line
// cat /sys/devices/system/cpu/cpu0/cache/index?/coherency_line_size
#define CL_SIZE 64
// Number of address bits to access bytes inside a cacheline (log(CL_SIZE))
#define CL_SIZE_BITS 6
// Number of cache sets in L1D cache
// cat /sys/devices/system/cpu/cpu0/cache/index0/number_of_sets
#define L1D_SETS 64
// Number of address bits defining the L1D cache set mapping (log(L1D_SETS))
#define L1D_SET_BITS 6
// Associativity of L1D cache
// cat /sys/devices/system/cpu/cpu0/cache/index0/ways_of_associativity
#define L1D_ASSOC 8

// Interval length in clock cycles
#define INTERVAL 0x00008000
// memory access times above THRESHOLD should be interpreted as cache miss
// memory access times below THRESHOLD should be interpreted as cache hit
// Adjust this value if needed
#define THRESHOLD 70

// L1D cache set to use for communication
#define CACHE_SET 63

// Don't change this
#if CACHE_SET >= L1D_SETS
#error "Choose a cache set in the range of 0 - (L1D_SETS-1)"
#endif
