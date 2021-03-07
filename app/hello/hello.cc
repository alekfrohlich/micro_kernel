#include <utility/ostream.h>
#include<machine/scratchpad.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Hello world!" << endl;
    int * scratchy_integer = new (SCRATCHPAD) int;
    *scratchy_integer = 5;
    cout << "addr=" << scratchy_integer << ", val=" << *scratchy_integer << endl;
    return 0;
}
