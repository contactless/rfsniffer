#include "logging.h"

#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/BasicLayout.hh>

#include <iostream>

void log4cpp_AddOstream(bool addIfThereIsNoOutputs) {
    log4cpp::Category& root = log4cpp::Category::getRoot();

    if (addIfThereIsNoOutputs && root.getAllAppenders().size() > 0)
        return;

    log4cpp::Appender *appender = new log4cpp::OstreamAppender("default", &std::cout);

    appender->setLayout(new log4cpp::BasicLayout());

    root.setPriority(log4cpp::Priority::NOTSET);
    root.addAppender(appender);
}

void log4cpp_AddOstreamIfThereIsNoOutputs() {
    log4cpp_AddOstream(true);
}

void log4cpp_AddOutput(std::string name, std::string fileName) {
    log4cpp::Appender *appender = new log4cpp::FileAppender(name, fileName);

    appender->setLayout(new log4cpp::BasicLayout());

    log4cpp::Category& root = log4cpp::Category::getRoot();
    root.setPriority(log4cpp::Priority::INFO);
    root.addAppender(appender);
}

