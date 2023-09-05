#ifndef ILOGTAIL_MY_LOG_H
#define ILOGTAIL_MY_LOG_H 1

#include <vector>
#include <string>

class Content {
public:
    std::string key;
    std::string value;
};

class Log {
public:
    std::vector<Content> contents;
};

#endif // ILOGTAIL_MY_LOG_H