#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <atomic>
#include <iostream>

template<class T, std::size_t size>
concept SizeConcept = (sizeof(T) == size);

template<std::size_t Size>
struct Mem
{
	std::uint8_t buff[Size];	
	
	template <SizeConcept<Size> T>
	operator T&()
	{
		return *(reinterpret_cast<T*>(buff)); 
	}
};

template<std::size_t Size, std::size_t Count, bool isDebug = false>
class PoolAllocator
{
	union Buffer;
public:
	using pointer = void*;
public:
	PoolAllocator():start(CreateMem()), current(start), size_(Count)
	{}

	~PoolAllocator()
	{
		delete[] this->start; 
		if constexpr(isDebug)
			if(Count != size_)
			{
				std::cout << "Memory leak!!!!!!\n";
			}
	}

	pointer allocate() noexcept
	{
		Buffer* tmp = this->current.load(std::memory_order_relaxed);
		Buffer* block;
		do{
			if(tmp == nullptr)
				return nullptr;
			block = tmp->ptr;
		}
		while(!this->current.compare_exchange_weak(tmp, block,
				       	std::memory_order_relaxed, std::memory_order_relaxed));
		this->size_.fetch_sub(1, std::memory_order_relaxed);
		return tmp;
	}

	void deallocate(pointer mem) noexcept
	{
		if(!mem)
			return;

		Buffer* buffer = reinterpret_cast<Buffer*>(mem);
		Buffer* tmp = this->current.load(std::memory_order_relaxed);
		this->size_.fetch_add(1, std::memory_order_relaxed);
		do	
		{
			buffer->ptr = tmp;
		}
		while(!this->current.compare_exchange_weak(tmp, buffer, 
					std::memory_order_relaxed, std::memory_order_relaxed));		
	}
	
	constexpr std::size_t size() const noexcept
	{
		return size_.load(std::memory_order_relaxed);
	}

private:
	union Buffer
	{
		Buffer* ptr;
		Mem<Size> object;
	};
	

	static Buffer* CreateMem()
	{
		Buffer* start = new Buffer[Count];
		Buffer* current = start;
		
		for(Buffer* end = (start + Count - 1); current != end; current++)
		{
			current->ptr = current + 1;	
		}
		current->ptr = nullptr;
		return start;	
	}

	Buffer* start;
	std::atomic<Buffer*> current;
	std::atomic<std::size_t> size_;
};


