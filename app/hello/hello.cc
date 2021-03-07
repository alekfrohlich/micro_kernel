#include <utility/ostream.h>
#include<machine/scratchpad.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Hello world!" << endl;
    int * scratchy_integer = new (SCRATCHPAD) int;
    return 0;
}
