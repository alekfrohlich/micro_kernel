#include <time.h>
#include <utility/ostream.h>

using namespace EPOS;
OStream cout;

int main() {
    while (true) {
        cout << "Iterate" << endl;
        Alarm::delay(1000000);
    }
}