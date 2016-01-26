#pragma once
class IComponent {};
class CompA : public IComponent {};
class CompB : public IComponent {};

template<class ComponentType>
class ComponentPool
{
	static PoolAllocator allocator;

	void operator=(const ComponentPool&) = delete;
	ComponentPool(const ComponentPool&) = delete;
	ComponentPool() = delete;
	~ComponentPool() = delete;

public:
	static ComponentType* Create()
	{
		return reinterpret_cast<ComponentType*>(allocator.Allocate());
	}
};

class Factory
{


public:
	Factory(byte* memory, short size)
	{
	}

	~Factory();

};


