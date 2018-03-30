#include "types.h"

uint64 endian_swap64(uint64 x)
{
    return (x>>56) | 
            ((x<<40) & 0x00FF000000000000) |
            ((x<<24) & 0x0000FF0000000000) |
            ((x<<8)  & 0x000000FF00000000) |
            ((x>>8)  & 0x00000000FF000000) |
            ((x>>24) & 0x0000000000FF0000) |
            ((x>>40) & 0x000000000000FF00) |
            (x<<56);
}

uint32 endian_swap32(uint32 x)
{
    return  x = (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}

uint16 endian_swap16(uint16 x)
{
    return (x>>8) | (x<<8);
}