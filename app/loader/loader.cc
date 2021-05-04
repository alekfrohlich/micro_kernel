#include <utility/ostream.h>
#include <utility/elf.h>
#include <architecture.h>
#include <system.h>
#include <time.h>

using namespace EPOS;

OStream cout;

extern "C" char _end;

int main()
{
    cout << "Loading beginned" << endl;
    unsigned end = ((unsigned)&_end < Application::APP_DATA) ? Application::APP_DATA : (unsigned)MMU::align_page(&_end);
    int * extras = static_cast<int*>(MMU::align_page(end) + Application::HEAP_SIZE);
    cout << "end=" << end << endl;
    cout << "Extras is located at addr=" << extras << endl;
    for (int app_size = *extras; app_size; extras += app_size/4, app_size = *reinterpret_cast<int*>(extras)) {
        ELF * app_elf = reinterpret_cast<ELF *>(++extras);
        if (!app_elf->valid()) {
            cout << "Skipping corrupted App" << endl;
            continue;
        }

        cout << "Loading App: addr=" << app_elf << ", size=" << app_size << endl;
        // db<Setup>(TRC) << "Setup_SifiveE::load_app()" << endl;
        // if(app_elf->load_segment(0) < 0) {
        //     db<Setup>(ERR) << "Application code segment was corrupted during INIT!" << endl;
        //     Machine::panic();
        // }
        // for(int j = 1; j < app_elf->segments(); j++)
        //     if(app_elf->load_segment(j) < 0) {
        //         db<Setup>(ERR) << "Application data segment was corrupted during INIT!" << endl;
        //         Machine::panic();
        //     }
    }

    cout << "Loading finished" << endl;
    return 0;
}
