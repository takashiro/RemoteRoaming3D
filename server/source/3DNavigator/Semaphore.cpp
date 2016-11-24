#include "Semaphore.h"

#include <Windows.h>

RD_NAMESPACE_BEGIN

struct Semaphore::Private
{
	HANDLE handle;
};

Semaphore::Semaphore(uint initial, uint maximum)
	: d(new Private)
{
	d->handle = CreateSemaphore(nullptr, initial, maximum, nullptr);
}

Semaphore::~Semaphore()
{
	CloseHandle(d->handle);
	delete d;
}

void Semaphore::acquire()
{
	WaitForSingleObject(d->handle, INFINITE);
}

void Semaphore::release(uint num)
{
	ReleaseSemaphore(d->handle, num, nullptr);
}

RD_NAMESPACE_END
