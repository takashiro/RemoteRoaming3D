#ifndef _CONTROLPANEL_H_
#define _CONTROLPANEL_H_

#include <string>
#include <map>
#include <iostream>

class ControlPanel{
public:
	typedef void (ControlPanel::*Callback)();
	
	ControlPanel(std::istream &in, std::ostream &out);
	int exec();

protected:
	static std::map<std::string, Callback> _callbacks;

	void _server();
	void _client();
	void _help();

private:
	std::istream &cin;
	std::ostream &cout;
};

#endif
