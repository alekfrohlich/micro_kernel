#include <utility/ostream.h>
#include <utility/elf.h>
#include <architecture.h>
#include <system.h>
#include <time.h>

using namespace EPOS;

OStream cout;

extern "C" char _end;
typedef int (Main)();

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
        
        Segment * cs;
        Segment * ds;
        
        // CODE
        if(app_elf->segment_size(0) <= 0) {
            cout << "Application code segment was corrupted during INIT!" << endl;
            return -1;
        }else{
            cout << "Creating code segment:  segment_size= " << hex << app_elf->segment_size(0) << endl;
            cs = new Segment(app_elf->segment_size(0), MMU::Flags::ALL);
        }
        
        CPU::Log_Addr cs_log_addr = Task::active()->address_space()->attach(cs);
        cout << "Loading code segment:  cs_log_addr= " << hex << cs_log_addr << endl;
        app_elf->load_segment(0, cs_log_addr);
        Task::active()->address_space()->detach(cs, cs_log_addr);
        
        // DATA
        int data_size = 0; 
        for(int j = 1; j < app_elf->segments(); j++){
            data_size += app_elf->segment_size(j);
        }
        data_size = MMU::align_page(data_size);
        data_size += MMU::align_page(Application::HEAP_SIZE);
        
        ds = new Segment(data_size, MMU::Flags::UDATA);
        
        CPU::Log_Addr ds_log_addr = Task::active()->address_space()->attach(ds);
        cout << "Loading data segment:  ds_log_addr= " << ds_log_addr << endl;
        CPU::Log_Addr cur_ds_log_addr =  ds_log_addr;
        for(int j = 1; j < app_elf->segments(); j++){
            if(app_elf->segment_size(j) > 0){
                app_elf->load_segment(j, cur_ds_log_addr);
                cur_ds_log_addr += app_elf->segment_size(j);
            }
        }
        Task::active()->address_space()->detach(cs, cs_log_addr);
        
        ASM("test123:");
        Task * app = new Task(cs, ds, reinterpret_cast<Main *>(app_elf->entry()));
        volatile CPU::Phy_Addr phy_cs1 = app->code_segment()->phy_address();
        volatile CPU::Phy_Addr phy_cs2 = app->address_space()->physical(Application::APP_CODE);
        volatile CPU::Phy_Addr phy_ds1 = app->data_segment()->phy_address();
        volatile CPU::Phy_Addr phy_ds2 = app->address_space()->physical(Application::APP_DATA);
        
        cout << "Seg says that cs' physical address is: " << *const_cast<CPU::Log_Addr*>(&phy_cs1) << endl;
        cout << "AS says that  cs physical address is: " << *const_cast<CPU::Log_Addr*>(&phy_cs2) << endl;
        cout << "Seg says that ds' physical address is: " << *const_cast<CPU::Log_Addr*>(&phy_ds1) << endl;
        cout << "AS says that ds'physical address is: " << *const_cast<CPU::Log_Addr*>(&phy_ds2) << endl;
    }
    cout << "Loading finished" << endl;
    
    // P5 -> delete all threads
    // for(int i = 0; i < apps; i++){
    //     app[i]->main()->join();
    //     delete app[i];    
    // }
    
    return 0;
}
