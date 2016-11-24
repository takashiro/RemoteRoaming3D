#pragma once

#include "global.h"

#include <functional>

RD_NAMESPACE_BEGIN

class Thread
{
public:
	Thread(const std::function<void()> &func);
	~Thread();

	Thread(Thread &&thread);

	void wait();

private:
	RD_DECLARE_PRIVATE;
};

RD_NAMESPACE_BEGIN
