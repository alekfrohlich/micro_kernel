// Description:
// The Heap reaches main() containing two elements: E1 and E2.
//    E1 begun from &_end to BOOT_STACK, but now has 32904 less bytes due to the allocations
//  of System::init().
//    E2 is still untouched and ranges from MEM_TOP to MEM_TOP-STACK_SIZE.
// E1 is way larger than E2.
//    In this test program, we allocate almost all of E1 so that Worst-Fit has to reach for E2
//  in its second allocation (line 30).
// For comparision purposes, we print how First Fit would have handled each allocation.

#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

// for Single System Heap
static const int REMAINING_SYS_HEAP = 134201064 - 32904 - 16*1024;
// for Application Heap
static const int REMAINING_APP_HEAP = 16777216;

enum {
    TEST_SINGLE_HEAP,
    TEST_MULTI_HEAP,
};

static const unsigned int MODE = TEST_MULTI_HEAP;
static const unsigned int REMAINING = (MODE==TEST_MULTI_HEAP)? REMAINING_APP_HEAP : REMAINING_SYS_HEAP;

// TODO: test only multi-heap and do it right

char * alloc(int bytes) {
    return new char[bytes];
}

int main()
{
    // terminal output is important after the following line
    cout << "Testing Worst-Fit..." << endl;

    // leave less than a stack worth of memory on E1
    alloc(REMAINING - 8*1024);
    // this should go to E2
    alloc(16); // will alloc 16+4 due to size

    return 0;
}
