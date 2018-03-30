#ifndef _CONFIGREADER_H_
#define _CONFIGREADER_H_

#include <string>
#include <map>
#include <sstream>
#include <vector>

/*
 * Config reader class.
 */
class ConfigReader
{
public:
    ConfigReader(bool case_sensitive = false, char comment = '#');
    // reads a file
    bool readFile(const char * filename);
    // checks if a variable is registered or not
    bool isRegistered(std::string & name);
    // reads a integer
    bool readInteger(const char * , int & val);
    // reads a float
    bool readFloat(const char * , float & val);
    // reads a string
    bool readString(const char * , std::string &);
    // splits a string on a token.
    bool splitString(std::string &, std::vector<std::string> &,char token = ',' ,bool trim_string = true);
protected:
    // trims a string.
    void trim(std::string & str);
    std::string makeLowercase(std::string & str);
    char commentChar;
    bool sensitive;
    std::map<std::string, std::string> m_variables;
};

#endif