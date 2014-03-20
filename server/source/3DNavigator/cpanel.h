#ifndef _CPANEL_H_
#define _CPANEL_H_

#include <string>
#include <map>

typedef void (*CPanelCallback)();
extern std::map<std::string, CPanelCallback> CPanelCommand;

#endif
