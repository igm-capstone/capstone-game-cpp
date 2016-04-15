#pragma once
#include <Memory/Memory/Memory.h>
#include <Memory/Memory/AllocatorUtility.h>

template<class ObjectType>
class Factory
{
	using value_type      = ObjectType;
	using reference       = value_type&;
	using pointer	      = value_type*;
	using const_reference = const value_type&;

public:
	class iterator {
	public:
		typedef typename ptrdiff_t difference_type;
		typedef typename Factory::value_type value_type;
		typedef typename Factory::reference  reference;
		typedef typename Factory::pointer    pointer;
		typedef std::input_iterator_tag iterator_category;

	private:
		pointer t;

	public:
		iterator() = default;
		iterator(pointer p) : t(p) {}
		iterator(const iterator& other) : t(other.t) {}
		~iterator() {}

		iterator& operator=(const iterator&) 
		{ 
			//std::swap();
			return *this;
		}
		bool operator==(const iterator& other) const
		{
			return t == other.t; 
		}

		bool operator!=(const iterator& other) const {
			return  t != other.t;
		}

		iterator& operator++() 
		{
			Iterate();
			return *this;
		}
		
		iterator operator++(int)
		{
			iterator it = *this;
			Iterate();
			return it; 
		}

		reference operator*() const 
		{ 
			return *this->t;
		}
		
		pointer operator->() const 
		{
			return *this;
		}

	private:
		void Iterate()
		{
			const int PADDING_VALUE_SO   = 0xBABACACA;
			const int PADDING_VALUE_COMP = 0xB0B0CACA;

			iterator end = EndIterator();
			
			// never iterate past end
			if (t == end.t) return;

			int b;
			do
			{
				b = *reinterpret_cast<int*>(++t);
			} while (t != end.t && b != PADDING_VALUE_SO && b != PADDING_VALUE_COMP);
		}
	};

	//class Container
	//{
	//public:
	//	//Container() noexcept = default;
	//	//Container(const Container& other) noexcept = default;
	//	//~Container() {}

	//	//Container& operator=(const Container& other) { return *this; }
	//	bool operator==(const Container& other) const { return true; }
	//	bool operator!=(const Container& other) const { return false; }

	//	iterator begin() { return Factory::BeginIterator(); }
	//	iterator end() { return Factory::EndIterator(); }
	//};

private:
	static PoolAllocator sAllocator;
	static pointer sBuffer;
	static size_t sMaxCount;
	static size_t sCount;

	//void operator=(const Factory&) = delete;
	//Factory(const Factory&) = delete;
	//Factory() = delete;
	//~Factory() = delete;

public:
	static pointer Create()
	{
		auto ptr = reinterpret_cast<pointer>(sAllocator.Allocate());
		sCount += 1;
		auto ret = new (ptr) value_type();

		// If BaseSceneObj, set the class type
		if (*reinterpret_cast<int*>(ret) == 0xBABACACA) {
			reinterpret_cast<BaseSceneObject*>(ret)->mClassID = typeid(*ret).hash_code();
			reinterpret_cast<BaseSceneObject*>(ret)->mClassName = typeid(*ret).name();
		}

		return ret;
	}

	static void Destroy(pointer object)
	{
		object->~ObjectType();
		sAllocator.Free(object);
		sCount -= 1;
	}

	//static Container GetContainer()
	//{
	//	return Container();
	//}

	static iterator BeginIterator()
	{
		const int PADDING_VALUE_SO   = 0xBABACACA;
		const int PADDING_VALUE_COMP = 0xB0B0CACA;

		pointer start = reinterpret_cast<pointer>(AlignedPointer(sBuffer, alignof(value_type)));
		pointer end = reinterpret_cast<pointer>(AlignedPointer(sBuffer, alignof(value_type))) + sMaxCount;

		while (start != end && *reinterpret_cast<int*>(start) != PADDING_VALUE_SO && *reinterpret_cast<int*>(start) != PADDING_VALUE_COMP)
		{
			++start;
		}

		return iterator(start);
	}

	static iterator EndIterator()
	{
		return iterator(reinterpret_cast<pointer>(AlignedPointer(sBuffer, alignof(value_type))) + sMaxCount);
	}

	static size_t GetCount()
	{
		return sCount;
	}

	iterator begin() { return Factory::BeginIterator(); }
	iterator end() { return Factory::EndIterator(); }
};

