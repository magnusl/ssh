#ifndef _HEX_H_
#define _HEX_H_

#include "types.h"
#include <string>

/* Function:        ssh_bin2hex
 * Description:     Converts a binary string to a hexadecimal one.
 */
static const char hex_table[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'}; 

void ssh_bin2hex(byte * buffer, uint32 length, std::string & out, bool separator)
{
    for(uint32 i = 0;i<length;i++)
    {
        if(separator && i > 0) out.insert(out.end(),':');
        byte low = buffer[i] & 0x0f, high = ((buffer[i] & 0xf0) >> 4);
        out.insert(out.end(), hex_table[high]);
        out.insert(out.end(), hex_table[low]);
    }
}


#endif