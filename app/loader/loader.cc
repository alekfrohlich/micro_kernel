#include <utility/ostream.h>
#include <architecture.h>
#include <system.h>
#include <time.h>
#include <elf.h>

using namespace EPOS;

OStream cout;

extern "C" char * _end;

int main()
{
    cout << "Loader" << endl;
    cout << "_end=" << &_end << endl;
    void * extras = static_cast<void*>(MMU::align_page(&_end) + Application::STACK_SIZE + Application::HEAP_SIZE);
    cout << "Extras is located at addr=" << extras << endl;
    // ELF * app_elf = reinterpret_cast<ELF *>(&bi[si->bm.application_offset]);
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
    return 0;
}
