// We show the Worst Fit behavior by allocating 3 blocks (one large, two small), and then
// freeing the outer blocks (b1, b3). This will fragment the Heap into two blocks: b1, b3.
// Then, we alloc something in b3, which is the biggest block (but last in the underlying Grouping_List).
// 
// Traits<Application>::HEAP_SIZE = 16KB
// For comparision purposes, we print how First Fit would have handled each allocation.

#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

char * alloc(int bytes) {
    return new char[bytes];
}

int main()
{
    // Terminal output is important after the following line
    cout << "Testing Worst-Fit..." << endl;
    CPU::int_disable();

    char * b1 = alloc(15*1024);
    char * b2 = alloc(128);
    char * b3 = alloc(128);
    
    free(b3);
    free(b1);

    char * a1 = alloc(16);

    return 0;
}
