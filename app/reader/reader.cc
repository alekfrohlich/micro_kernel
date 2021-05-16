#include <utility/ostream.h>
#include <utility/elf.h>
#include <architecture.h>
#include <system.h>
#include <time.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Reader beginned" << endl;
    
    unsigned int port = 1;
    Shared_Segment * sseg = new Shared_Segment(port, 1024, MMU::Flags::UDATA);
    cout << sseg << endl;
    
    CPU::Log_Addr sseg_log_addr = Task::active()->address_space()->attach(reinterpret_cast<Segment *>(sseg));
    long unsigned int * a = sseg_log_addr;
    
    CPU::Phy_Addr phy_addr = sseg->phy_address();
    cout << "phy_addr_reader=" << phy_addr << endl;
    
    Alarm::delay(1000000);
    cout << "reader_a::: " << a << endl;
    cout << "reader_a: " << *a << endl; 
    
    cout << "Reader is finishing" << endl;
    Task::active()->address_space()->detach(reinterpret_cast<Segment *>(sseg));
    delete sseg;
    
    
    return 0;
}
