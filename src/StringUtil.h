#ifndef _STRINGUTIL_H_
#define _STRINGUTIL_H_

#include <vector>
#include <string>

namespace ssh
{
    void Tokenize(const std::string& str, std::vector<std::string>& tokens,const std::string& delimiters);
    bool decide(const std::string & client_algorithms, const std::string & server_algorithms, std::string & result);
};

#endif