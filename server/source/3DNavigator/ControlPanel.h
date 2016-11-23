#pragma once

#include "global.h"

#include <string>
#include <map>
#include <iostream>

RD_NAMESPACE_BEGIN

class Server;

class ControlPanel
{
public:
	typedef void (ControlPanel::*Callback)();

	ControlPanel(std::istream &in, std::ostream &out);
	~ControlPanel();
	int exec();

protected:
	static std::map<std::string, Callback> mCallbacks;

	void server();
	void client();
	void help();

private:
	std::istream &cin;
	std::ostream &cout;

	Server *mServer;
};

RD_NAMESPACE_END
