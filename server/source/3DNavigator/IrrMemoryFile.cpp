#include "IrrMemoryFile.h"

using namespace irr;
using namespace io;

IrrMemoryFile::IrrMemoryFile(const path &file_name)
	:_file_name(file_name), _pos(0)
{
}

IrrMemoryFile::~IrrMemoryFile(){
	
}

const path &IrrMemoryFile::getFileName() const
{
	return _file_name;
}

long IrrMemoryFile::getPos() const
{
	return _pos;
}

bool IrrMemoryFile::seek(long finalPos, bool relativeMovement)
{
	if(relativeMovement){
		_pos += finalPos;
	}else{
		_pos = finalPos;
	}
	return true;
}

s32 IrrMemoryFile::write(const void *buffer, u32 sizeToWrite)
{
	_content.append((const char *) buffer, sizeToWrite);
	_pos += sizeToWrite;
	return sizeToWrite;
}

const std::string &IrrMemoryFile::getContent() const{
	return _content;
}
