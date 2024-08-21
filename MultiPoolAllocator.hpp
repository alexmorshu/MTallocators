#include <initializer_list>
#include <algorithm>
#include <iostream>
#include "PoolAllocator.hpp"
template<class, std::size_t>
class MultiPoolAllocator; 

template<class T>
class PointerForMain
{
	T** ptr_;
	std::size_t size;
public:
	PointerForMain(T** container, std::size_t size):
		ptr_(container), size(size)
	{
	}
	PointerForMain(std::nullptr_t): ptr_(nullptr), size(0)
	{}

	PointerForMain(const PointerForMain<T>& obj) = default;
	PointerForMain<T>& operator=(const PointerForMain<T>& obj) = default;
		

	T& operator*() noexcept
	{
		return **ptr_; 
	}
	
	const T& operator*() const noexcept
	{
		return **ptr_;
	}

	PointerForMain<T> operator++(int) noexcept
	{
		PointerForMain<T> current(*this);
		this->ptr_+=1;
		return current;	
	}

	PointerForMain<T>& operator++() noexcept
	{
		this->ptr_+=1;
		return *this;
	}
	
	PointerForMain<T> operator+(std::size_t i) const noexcept
	{
		PointerForMain<T> tmp(*this);
		tmp.ptr_+=i;	
		return tmp;
	}

	PointerForMain<T> operator-(std::size_t i) const noexcept
	{
		PointerForMain<T> tmp(*this);
		tmp.ptr_-=i;
		return tmp;
	}

	T& operator[](std::ptrdiff_t i) noexcept
	{
		return **(this->ptr_ + i);
	}
	
	bool operator == (std::nullptr_t) const noexcept 
	{
		return this->ptr_ == nullptr;
	}
	
	bool operator == (const PointerForMain<T>& obj) const noexcept
	{
		return (this->ptr_ == obj.ptr_);
	}	

	template<class Q>
	bool operator != (Q obj) const noexcept
	{
		return !(*this == obj);
	}

	operator bool() const noexcept
	{
		return (*this != nullptr);
	}
	
	PointerForMain<T>& operator += (std::size_t i) noexcept
	{
		this->ptr_ += i;
		return *this;
	}

	PointerForMain<T>& operator -= (std::size_t i) noexcept
	{
		this->ptr_ -= i;
		return *this;
	}

	template<class Q, std::size_t C>
	friend class MultiPoolAllocator;	

};

template<class T, std::size_t Count>
class MultiPoolAllocator
{
	PoolAllocator<sizeof(T), Count, true> allocator_;
public:
	using pointer = PointerForMain<T>;

	pointer allocate(std::size_t size)
	{

		T** container = new T*[size];
		std::size_t i = 0;
		bool fail = false;
		for(; i < size; i++)
		{
			void* ptr = allocator_.allocate(); 
			if(ptr == nullptr)
			{
				fail = true;
				break;
			}
			*(container+i) = reinterpret_cast<T*>(ptr);	
		}

		if(fail)
		{
			for(std::size_t q = 0; q < i; q++)
			{
				allocator_.deallocate(*(container + q));
			}
			delete[] container;
			return nullptr;
		}
		return {container, size};
	}

	void deallocate(pointer ptr, std::size_t size) noexcept
	{
		T** container = ptr.ptr_;
		for(std::size_t i = 0; i < size; i++)
		{
			allocator_.deallocate(*(container + i));
		}
		delete[] container;
	}
};

