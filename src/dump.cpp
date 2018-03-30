#include <iostream>
#include "types.h"

using namespace std;

void dump(char * name, unsigned char * data, uint32 len)
{
    cout << endl << name <<":" << endl;
    for(uint32 i=0;i<len;++i)
    {
        if(i > 0 && ((i % 16) == 0))
            std::cout << endl;
        unsigned char low = data[i] & 0x0f, high = (data[i] >> 4);
        char h = (high < 10) ? ('0' + high) : ('a' + (high - 10));;
        char l = (low < 10) ? ('0' + low) : ('a' + (low - 10));

        std::cout <<h << l;
        if(((i % 2) != 0) && ((i % 16) != 0)) std::cout <<" ";
    }
}