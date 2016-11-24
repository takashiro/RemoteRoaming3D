#include "Thread.h"

#include <Windows.h>

RD_NAMESPACE_BEGIN

struct Thread::Private
{
	std::function<void()> func;
	HANDLE handle;

	static DWORD WINAPI ThreadFunc(LPVOID lpParam)
	{
		Private *d = static_cast<Private *>(lpParam);
		d->func();
		return 0;
	}
};

Thread::Thread(const std::function<void ()> &func)
	: d(new Private)
{
	d->func = func;
	d->handle = CreateThread(nullptr, 0, Private::ThreadFunc, static_cast<LPVOID>(d), 0, nullptr);
}

Thread::~Thread()
{
	if (d) {
		CloseHandle(d->handle);
		delete d;
	}
}

Thread::Thread(Thread &&thread)
	: d(thread.d)
{
	thread.d = nullptr;
}

void Thread::wait()
{
	WaitForSingleObject(d->handle, INFINITE);
}

RD_NAMESPACE_END
