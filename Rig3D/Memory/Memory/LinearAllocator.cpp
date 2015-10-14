#include "LinearAllocator.h"
#include "AllocatorUtility.h"
#include <assert.h>
#include <cstdlib>
#include <cstring>

using namespace cliqCity::memory;

LinearAllocator::LinearAllocator(size_t size) : mIsOwner(true)
{
	mStart		= (uint8_t*)malloc(size);
	mEnd		= (mStart + size);
	mCurrent	= mStart;
}

LinearAllocator::LinearAllocator(void* start, void* end) : mStart((uint8_t*)start), mEnd((uint8_t*)end), mIsOwner(false)
{
	mCurrent = mStart;
}

LinearAllocator::LinearAllocator()
{
	mStart		= nullptr;
	mEnd		= nullptr;
	mCurrent	= nullptr;
}


LinearAllocator::~LinearAllocator()
{
	mStart		= nullptr;
	mEnd		= nullptr;
	mCurrent	= nullptr;
}

void* LinearAllocator::Allocate(size_t size, size_t alignment, size_t offset)
{
	assert((alignment & (alignment - 1)) == 0);	// Check for power of 2

	// offset pointer first, align it, and offset it back
	mCurrent = (uint8_t*)AlignedPointer((void*)(mCurrent + offset), alignment) - offset;

	void* userPtr = mCurrent;
	mCurrent += size + alignment + offset;

	if (mCurrent >= mEnd)
	{
		// out of memory
		return nullptr;
	}

	return userPtr;
}

void LinearAllocator::Reset()
{
	std::memset(mStart, 0, (mEnd - mStart) * sizeof(uint8_t));
	mCurrent = mStart;
}

void LinearAllocator::Free()
{
	if (mIsOwner && mStart != nullptr) {
		std::free(mStart);
	}
}