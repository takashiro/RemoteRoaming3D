#pragma once

#include "global.h"

#include <string>
#include <map>
#include <iostream>

RD_NAMESPACE_BEGIN

class ControlPanel{
public:
	typedef void (ControlPanel::*Callback)();
	
	ControlPanel(std::istream &in, std::ostream &out);
	int exec();

protected:
	static std::map<std::string, Callback> mCallbacks;

	void server();
	void client();
	void help();

private:
	std::istream &cin;
	std::ostream &cout;
};

RD_NAMESPACE_END
