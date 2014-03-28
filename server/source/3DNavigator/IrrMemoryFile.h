#ifndef _IRRMEMORYFILE_H_
#define _IRRMEMORYFILE_H_

#include <IWriteFile.h>

class IrrMemoryFile: public irr::io::IWriteFile{
public:
	IrrMemoryFile(const irr::io::path &file_name, long buffer_size);
	~IrrMemoryFile();
	bool seek(long finalPos, bool relativeMovement = false);
	irr::s32 write(const void *buffer, irr::u32 sizeToWrite);
	
	inline const irr::io::path &getFileName() const{return _file_name;};
	inline long getPos() const{return _pos;};
	inline const char *getContent() const{return _content;};
	inline void clear(){_pos = 0;};

protected:
	irr::io::path _file_name;
	long _pos;
	long _buffer_size;
	char *_content;
};

#endif
