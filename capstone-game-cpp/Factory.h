#pragma once

template<class T>
class FactoryPool
{
	static PoolAllocator allocator;

	void operator=(const FactoryPool&) = delete;
	FactoryPool(const FactoryPool&) = delete;
	FactoryPool() = delete;
	~FactoryPool() = delete;

public:
	static T* Create()
	{
		auto ptr = reinterpret_cast<T*>(allocator.Allocate());
		return new (ptr) T();
	}
};

/*
template<class SceneObjectType>
class SceneObjectPool
{
	static PoolAllocator allocator;

	void operator=(const SceneObjectPool&) = delete;
	SceneObjectPool(const SceneObjectPool&) = delete;
	SceneObjectPool() = delete;
	~SceneObjectPool() = delete;

public:
	static SceneObjectType* Create()
	{
		auto ptr =  reinterpret_cast<SceneObjectType*>(allocator.Allocate());
		return new (ptr) SceneObjectType();
	}
};*/

class Factory
{


public:
	Factory(byte* memory, short size)
	{
	}

	~Factory();

};


