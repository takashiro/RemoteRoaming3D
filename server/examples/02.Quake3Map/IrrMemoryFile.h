#ifndef _IRRMEMORYFILE_H_
#define _IRRMEMORYFILE_H_

#include <IWriteFile.h>
#include <WinSock.h>
#include <string>

class IrrMemoryFile: public irr::io::IWriteFile{
public:
	IrrMemoryFile(const irr::io::path &file_name);
	~IrrMemoryFile();
	virtual const irr::io::path &getFileName() const;
	virtual long getPos() const;
	virtual bool seek(long finalPos, bool relativeMovement = false);
	virtual irr::s32 write(const void *buffer, irr::u32 sizeToWrite);
	const std::string &getContent() const;

protected:
	irr::io::path _file_name;
	long _pos;
	std::string _content;
};

#endif
