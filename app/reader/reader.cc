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
    
    Alarm::delay(100000);
    unsigned int port = 1;
    Shared_Segment * sseg = new Shared_Segment(port, 1024, MMU::Flags::UDATA);
    cout << sseg << endl;
    
    CPU::Phy_Addr phy_addr = sseg->phy_address();
    cout << "phy_addr_reader=" << phy_addr << endl;
    
    CPU::Log_Addr sseg_log_addr = Task::active()->address_space()->attach(reinterpret_cast<Segment *>(sseg));
    
    cout << "sseg_log_addr_reader=" << sseg_log_addr << endl;
    
    long unsigned int * a = sseg_log_addr;    
        
    Semaphore * writer_sem_addr = reinterpret_cast<Semaphore *>(a + sizeof(long unsigned int));
    cout << "writer_sem_addr=" << writer_sem_addr << endl;
    Semaphore * writer_sem = writer_sem_addr;
    cout << "writer_sem=" << writer_sem << endl;
    
    ASM("SEM_WRITER2:");

    // writer_sem->v();
    // writer_sem->v();
    // writer_sem->v();
    // writer_sem->p();
    // writer_sem->p();

    
    Semaphore * reader_sem_addr = reinterpret_cast<Semaphore *>(writer_sem_addr + sizeof(Semaphore));
    cout << "reader_sem_addr=" << reader_sem_addr << endl;
    Semaphore * reader_sem = new (reader_sem_addr) Semaphore(0);
    
    ASM("SEM_READER2:");
    
    writer_sem->v();
    writer_sem->v();
    writer_sem->v();
    writer_sem->p();
    writer_sem->p();
    
    
    cout << "reader_a::: " << a << endl;
    cout << "reader_a: " << *a << endl;
    
    cout << "reader is finishing" << endl;
    reader_sem->v();
    Task::active()->address_space()->detach(reinterpret_cast<Segment *>(sseg));
    delete sseg;
    
    
    return 0;
}
