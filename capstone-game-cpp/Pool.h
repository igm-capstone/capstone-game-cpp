#pragma once

template<class ObjectType>
class Pool
{
	using value_type      = ObjectType;
	using reference       = value_type&;
	using pointer	      = value_type*;
	using const_reference = const value_type&;

public:
	class iterator {
	public:
		typedef typename ptrdiff_t difference_type;
		typedef typename Pool::value_type value_type;
		typedef typename Pool::reference  reference;
		typedef typename Pool::pointer    pointer;
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
			const int PADDING_VALUE = 0xBABACACA;

			iterator end = EndIterator();
			
			// never iterate past end
			if (t == end.t) return;

			int b;
			do
			{
				b = *reinterpret_cast<int*>(++t);
			} while (t != end.t && b != PADDING_VALUE);
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

	//	iterator begin() { return Pool::BeginIterator(); }
	//	iterator end() { return Pool::EndIterator(); }
	//};

private:
	static PoolAllocator sAllocator;
	static pointer sBuffer;
	static size_t sCount;

	//void operator=(const Pool&) = delete;
	//Pool(const Pool&) = delete;
	//Pool() = delete;
	//~Pool() = delete;

public:
	static pointer Create()
	{
		auto ptr = reinterpret_cast<pointer>(sAllocator.Allocate());
		return new (ptr) value_type();
	}

	static void Destroy(pointer object)
	{
		sAllocator.Free(object);
	}

	//static Container GetContainer()
	//{
	//	return Container();
	//}

	static iterator BeginIterator()
	{
		return iterator(sBuffer);
	}

	static iterator EndIterator()
	{
		return iterator(sBuffer + sCount);
	}

	iterator begin() { return Pool::BeginIterator(); }
	iterator end() { return Pool::EndIterator(); }
};

