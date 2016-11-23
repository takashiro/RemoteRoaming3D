#pragma once

#include <IWriteFile.h>

namespace irr {

namespace io {

class MemoryFile : public irr::io::IWriteFile
{
public:
	MemoryFile(const irr::io::path &file_name, long buffer_size);
	~MemoryFile();
	bool seek(long finalPos, bool relativeMovement = false);
	irr::s32 write(const void *buffer, irr::u32 sizeToWrite);

	inline const irr::io::path &getFileName() const { return mFileName; };
	inline long getPos() const { return mPos; };
	inline const char *getContent() const { return mContent; };
	inline void clear() { mPos = 0; };

protected:
	irr::io::path mFileName;
	long mPos;
	long mBufferSize;
	char *mContent;
};

}

}
