#include "PoolAllocator.h"

using namespace	cliqCity::memory;

PoolAllocator::PoolAllocator(void* start, void* end, size_t elementSize) : mNext(nullptr)
{
	union
	{
		void* asVoid;
		char* asChar;
		Node* asNode;
	};

	// Get Pointer to first memory location
	asVoid	= start;
	mNext	= asNode;

	const size_t elementCount = (static_cast<char*>(end) - asChar) / elementSize;

	// Move to next memory location
	asChar += elementSize;

	Node* iterator = mNext;
	for (int i = 1; i < elementCount; i++)
	{
		iterator->mNext	= asNode;
		iterator		= iterator->mNext;
		asChar			+= elementSize;
	}
}

void* PoolAllocator::Allocate()
{
	Node* head	= mNext;
	mNext		= head->mNext;
	return head;
}

void PoolAllocator::Free(void* pointer)
{
	Node* head	= static_cast<Node*>(pointer);
	head->mNext = mNext;
	mNext = head;
}