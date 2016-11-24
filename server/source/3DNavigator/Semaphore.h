#pragma once

#include "global.h"

RD_NAMESPACE_BEGIN

class Semaphore
{
public:
	Semaphore(uint initial = 1, uint maximum = 1);
	~Semaphore();

	Semaphore(const Semaphore &semaphore) = delete;

	void acquire();
	void release(uint num = 1);

private:
	RD_DECLARE_PRIVATE;
};

RD_NAMESPACE_END
