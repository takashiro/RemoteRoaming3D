#include "IrrMemoryFile.h"

#include <memory.h>

namespace irr {

namespace io {

MemoryFile::MemoryFile(const path &fileName, long bufferSize)
	:mFileName(fileName), mPos(0), mBufferSize(bufferSize)
{
	mContent = new char[bufferSize];
}

MemoryFile::~MemoryFile()
{
	delete[] mContent;
}

bool MemoryFile::seek(long finalPos, bool relativeMovement)
{
	if (relativeMovement) {
		mPos += finalPos;
	} else {
		mPos = finalPos;
	}

	if (mPos >= 0 && mPos < mBufferSize) {
		return true;
	}

	if (mPos < 0) {
		mPos = 0;
	} else {
		mPos = mBufferSize - 1;
	}

	return false;
}

s32 MemoryFile::write(const void *buffer, u32 sizeToWrite)
{
	if (mPos + (long)sizeToWrite >= mBufferSize) {
		sizeToWrite = mBufferSize - mPos - 1;
		if (sizeToWrite <= 0)
			return 0;
	}

	memcpy(mContent + mPos, buffer, sizeToWrite);
	mPos += sizeToWrite;
	mContent[mPos] = 0;

	return sizeToWrite;
}

} //io

} //irr
