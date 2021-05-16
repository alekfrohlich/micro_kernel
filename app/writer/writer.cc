#include <utility/ostream.h>
#include <utility/elf.h>
#include <architecture.h>
#include <system.h>
#include <time.h>
#include <memory.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Writer beginned" << endl;
    
    // unsigned int task;
    unsigned int port = 1;
    Shared_Segment * sseg = new Shared_Segment(port, 1024, MMU::Flags::UDATA);
    cout << sseg << endl;
    
    CPU::Log_Addr sseg_log_addr = Task::active()->address_space()->attach(reinterpret_cast<Segment *>(sseg));
    
    cout << "sseg_log_addr=" << sseg_log_addr << endl;
    
    long unsigned int * a = sseg_log_addr;    
    
    CPU::Phy_Addr phy_addr = sseg->phy_address();
    cout << "phy_addr_writer=" << phy_addr << endl;
    
    *a = 12345;
    cout << "writer_a::: " << a << endl;
    cout << "writer_a: " << *a << endl; 
    
    
    
    
    Alarm::delay(2000000);
    cout << "Writer is finishing" << endl;
    Task::active()->address_space()->detach(reinterpret_cast<Segment *>(sseg));
    delete sseg;
    
    
    return 0;
}
