#include "StackAllocator.h"

using namespace cliqCity::memory;

StackAllocator::StackAllocator(void* start, void* end) : mStart(static_cast<uint8_t*>(start)), mEnd(static_cast<uint8_t*>(end)), mCurrent(mStart)
{
	
}


StackAllocator::StackAllocator() : StackAllocator(nullptr, nullptr)
{
}


StackAllocator::~StackAllocator()
{
}
