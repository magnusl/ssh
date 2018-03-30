#include "ConfigReader.h"
using namespace std;
#include <fstream>
#include <locale>
#include <cctype>
#include <iostream>
#include <sstream>

/*
 * Constructor.
 */
ConfigReader::ConfigReader(bool case_sensitive, char comment)
{
    this->commentChar = comment;
    sensitive = case_sensitive;
}

/* 
 * Reads a config file
 */

bool ConfigReader::readFile(const char * filename)
{
    ifstream file(filename);
    // open the file.
    if(!file.is_open()) return false;
    string line;
    // read the first line.
    while(!file.eof()) {
        getline(file, line);
        if(!line.empty())
        {
            size_t pos = line.find_first_not_of(" ");
            if(line.at(pos) == commentChar) {
                continue;
            }
            else
            {
                // find the first char that isn't a space
                size_t firstChar = line.find_first_not_of(" ");
                if(firstChar == string::npos)
                    continue;

                string str = line.substr(firstChar);
                // find the divider.
                size_t divider = str.find_first_of("=");
                if(divider == string::npos) {
                    return false;
                }
                string var = str.substr(0, divider);
                if(divider == line.length()) {
                    return false;
                }

                string value = str.substr(divider + 1);
                trim(var);
                trim(value);
                // now save the variables.
                
                if(this->sensitive){
                    var = makeLowercase(var);
                    value = makeLowercase(var);
                }   
                // bind the variablename with the value.
                this->m_variables[var] = value;
            }
        }
    }
    file.close();
    return true;
}

/*
 * Converts a string to lowercase.
 */
string ConfigReader::makeLowercase(string & str)
{
    string lowercase;
    for(string::iterator it = str.begin();
        it != str.end();
        it++)
    {
        if(std::isupper(*it)) {
            lowercase += std::tolower(*it);
        } else {
            lowercase += *it;
        }
    }
    return lowercase;
}

/* trims a string
 */
void ConfigReader::trim(string & str)
{
    size_t first = str.find_first_not_of(" ");
    size_t last = str.find_last_not_of(" ");
    if(last == str.length()) {
        str = str.substr(first, string::npos);
    } else {
        str = str.substr(first, last+1);
    }
}

/* Checks if a variable is registered or noe
 */ 
bool ConfigReader::isRegistered(string & name)
{
    return m_variables[name].length() != 0;
}

/* Reads a Integer
 */
bool ConfigReader::readInteger(const char * var, int & val)
{
    std::string name = var;
    if(!isRegistered(name)) return false;
    stringstream converter;
    converter << m_variables[name];
    if(!(converter >> val)) return false;
    return true;
}

/* Reads a float
 */
bool ConfigReader::readFloat(const char * var, float & val)
{
    std::string name = var;
    if(!isRegistered(name)) return false;
    stringstream converter;
    converter << m_variables[name];
    if(!(converter >> val)) return false;
    return true;
}

/* Reads a string
 */
bool ConfigReader::readString(const char * var, string & str)
{
    std::string name = var;
    if(!isRegistered(name)) {
        return false;
    }
    str = m_variables[name];
    return true;
}

/* Splits the string using a specific token.
 */
bool ConfigReader::splitString(string & name, vector<string> & vec, char token, bool trim_string)
{
    if(!isRegistered(name)) return false;
    string temp = m_variables[name];
    string split;
    size_t splitPos = temp.find_first_of(token);
    while(splitPos != string::npos)
    {
        // as long as we can split.
        split = temp.substr(0, splitPos);
        // trim the splitted part
        if(split.length() > 0)
        {
            if(trim_string) trim(split);
            // add the splitted part to the vector
            vec.push_back(split);
            // extract the remaining data.
            if((splitPos + 1) < (temp.length()-1)) {
                temp = temp.substr(splitPos+1);
            }
            else {
                temp = temp.substr(string::npos);
            }
        } else {
            return false;
        }
        
        // try and split
        splitPos = temp.find_first_of(token);
    }
    // add the rest of the string
    if(trim_string) trim(temp);
    vec.push_back(temp);
    return true;
}






