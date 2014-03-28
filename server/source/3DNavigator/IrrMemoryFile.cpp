#include "IrrMemoryFile.h"

#include <memory.h>

using namespace irr;
using namespace io;

IrrMemoryFile::IrrMemoryFile(const path &file_name, long buffer_size)
	:_file_name(file_name), _pos(0), _buffer_size(buffer_size)
{
	_content = new char[buffer_size];
}

IrrMemoryFile::~IrrMemoryFile(){
	delete[] _content;
}

bool IrrMemoryFile::seek(long finalPos, bool relativeMovement)
{
	if(relativeMovement)
	{
		_pos += finalPos;
	}else{
		_pos = finalPos;
	}

	if(_pos >= 0 && _pos < _buffer_size)
	{
		return true;
	}

	if(_pos < 0)
	{
		_pos = 0;
	}
	else
	{
		_pos = _buffer_size - 1;
	}

	return false;
}

s32 IrrMemoryFile::write(const void *buffer, u32 sizeToWrite)
{
	if(_pos + (long) sizeToWrite >= _buffer_size)
	{
		sizeToWrite = _buffer_size - _pos - 1;
		if(sizeToWrite <= 0)
			return 0;
	}

	memcpy(_content + _pos, buffer, sizeToWrite);
	_pos += sizeToWrite;
	_content[_pos] = 0;

	return sizeToWrite;
}
