#pragma once

#include <string>
#include <log4cpp/Category.hh>


// PriorityLevel { 
//   EMERG = 0, FATAL = 0, ALERT = 100, CRIT = 200, 
//   ERROR = 300, WARN = 400, NOTICE = 500, INFO = 600, 
//   DEBUG = 700, NOTSET = 800 
// }



#define LOG(x) log4cpp::Category::getRoot() << log4cpp::Priority::x 

// This class magic is from gLog, look there
class LogMessageVoidify {
public:
    LogMessageVoidify() {}

    void operator&(const log4cpp::CategoryStream &) {}
};


void log4cpp_AddOutput(std::string name, std::string fileName);
void log4cpp_AddOstreamIfThereIsNoOutputs();
