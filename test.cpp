#include "PoolAllocator.hpp"
#include <vector>
#include <thread>

PoolAllocator<4, 100000, true> a;

void T1()
{
        std::size_t max = 100;
        std::size_t q = 0;
        while(q < max){
                std::vector< void* > vector(99000);
                for(int i = 0; i < 99000; i++)
                {
                        vector[i] = a.allocate();
                }
                for(int i = 0; i < 99000; i++)
                {
                        a.deallocate(vector[i]);
                }
                q++;
        }

}
void T2()
{
        std::size_t max = 9900;
        std::size_t q = 0;
        while(q < max)
        {
                std::vector< void* > vector(1000);
                for(int i = 0; i < 1000; i++)
                {
                        vector[i] = a.allocate();
                }
                for(int i = 0; i < 1000; i++)
                {
                        a.deallocate(vector[i]);
                }
                q++;
        }
}

int main()
{
	std::thread t1(T1);
	std::thread t2(T2);

	t1.join();
	t2.join();
	std::cout << a.size() << std::endl;
}

