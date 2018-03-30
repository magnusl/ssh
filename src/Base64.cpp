#include <cstdlib>
#include <string>

namespace ssh
{
    using namespace std;
    /*
     *
     */
    unsigned char Base64Inv(char c)
    {
        if(c >= 'A' && c <= 'Z')
            return (c - 'A');
        else if(c >= 'a' && c <= 'z') 
            return (c - 'a') + 26;
        else if(c >= '0' && c <= '9')
            return c - '0' + 52;
        else if(c == '=')
            return 0;
        else 
            return 0;
    }

    /*
     *
     */
    string RemoveIllegalCars(const std::string & in)
    {
        string sret;
        for(string::const_iterator it = in.begin(); it != in.end(); it++)
        {
            char c = *it;
            if(isalpha(c) || isdigit(c) || (c == '='))
                sret.insert(sret.end(), c);
        }
        return sret;
    }

    size_t Base64_decode(const char * src, unsigned char * dst)
    {
        size_t numpads = 0, d = 0;
        const char * pad = strchr(src, '=');
        if(pad != NULL) {
            size_t p = pad - src;
            numpads = strlen(src) - p;
        }
        for(size_t i = 0, num = strlen(src); i < num;i += 4)
        {
            dst[d++] = (Base64Inv(src[i]) << 2) | (Base64Inv(src[i+1]) >> 4);
            dst[d++] = (Base64Inv(src[i+1]) << 4) | (Base64Inv(src[i+2]) >> 2);
            dst[d++] = (Base64Inv(src[i+2]) << 6) | Base64Inv(src[i+3]);
        }
        return d - numpads;
    }
};