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
    
    unsigned int port = 1;
    Shared_Segment * sseg = new Shared_Segment(port, 1024, MMU::Flags::UDATA);
    cout << sseg << endl;
    
    CPU::Phy_Addr phy_addr = sseg->phy_address();
    cout << "phy_addr_writer=" << phy_addr << endl;
    
    CPU::Log_Addr sseg_log_addr = Task::active()->address_space()->attach(reinterpret_cast<Segment *>(sseg));
    
    cout << "sseg_log_addr_writer=" << sseg_log_addr << endl;
    
    long unsigned int * a = sseg_log_addr;    
    *a = 12345;
    
    Semaphore * writer_sem_addr = reinterpret_cast<Semaphore *>(a + sizeof(long unsigned int));
    cout << "writer_sem_addr=" << writer_sem_addr << endl;
    Semaphore * writer_sem = new (writer_sem_addr) Semaphore(0);
    
    ASM("SEM_WRITER1:");
    
    Semaphore * reader_sem_addr = reinterpret_cast<Semaphore *>(writer_sem_addr + sizeof(Semaphore));
    cout << "reader_sem_addr=" << reader_sem_addr << endl;
    Semaphore * reader_sem = reader_sem_addr;
    
    writer_sem->v();
    writer_sem->p();
    ASM("SEM_READER1:");
    writer_sem->p();
    
    cout << "writer_a::: " << a << endl;
    cout << "writer_a: " << *a << endl;
    
    reader_sem->p();
    cout << "Writer is finishing" << endl;
    Task::active()->address_space()->detach(reinterpret_cast<Segment *>(sseg));
    delete sseg;
    
    
    return 0;
}
