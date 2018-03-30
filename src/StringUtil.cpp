#include <string>
#include <vector>

using namespace std;

namespace ssh
{
    /* Function:        Tokenize
     * Description:     Splits a string into tokens.
     */
    void Tokenize(const string& str, vector<string>& tokens,const string& delimiters = " ")
    {
           // Skip delimiters at beginning.
         string::size_type lastPos = str.find_first_not_of(delimiters, 0);
          // Find first "non-delimiter".
         string::size_type pos     = str.find_first_of(delimiters, lastPos);

         while (string::npos != pos || string::npos != lastPos)
         {
               // Found a token, add it to the vector.
               tokens.push_back(str.substr(lastPos, pos - lastPos));
              // Skip delimiters.  Note the "not_of"
              lastPos = str.find_first_not_of(delimiters, pos);
              // Find next "non-delimiter"
              pos = str.find_first_of(delimiters, lastPos);
        }
    }

    /* Function:        ssh_transport::decide
     * Description:     decides what algorithm to use.
     */
    bool decide(const std::string & client_algorithms, const std::string & server_algorithms, std::string & result)
    {
        // split the strings
        std::vector<std::string> client_vector, server_vector;
        Tokenize(client_algorithms, client_vector, ",");
        Tokenize(server_algorithms, server_vector, ",");

        size_t c = client_vector.size();
        size_t s = server_vector.size();
        for(size_t currentC = 0; currentC < c; ++currentC) {
            // for all the clients algorithms
            string & m_c = client_vector[currentC];
            for(size_t currentS = 0; currentS < s; ++currentS) {
                string & m_s = server_vector[currentS];
                if(m_c.compare(m_s) == 0) {
                    result = m_c;
                    return true;
                }
            }
        }
        return false;
    }
};