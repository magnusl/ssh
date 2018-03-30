#include "Buffer.h"
#include "common.h"
#include "types.h"

namespace ssh
{
    /* Function:        Buffer::Buffer
     * Description:     The buffers constructor
     */
    Buffer::Buffer(uint32 _size)
    {
        size = _size;
        data = new (std::nothrow) unsigned char[size];
        read_pos = write_pos = 0;
    }

    /* Function:        Buffer::~Buffer
     * Description:     Destructor, performs the required cleanup.
     */
    Buffer::~Buffer()
    {
        SAFE_DELETE_ARRAY(data)
    }

    /* Function:        Buffer::writeBytes
     * Description:     Writes data to the buffer.
     */
    bool Buffer::writeBytes(const unsigned char * src, uint32 num)
    {
        uint32 possible = size - write_pos;
        if(possible < num)
            return false;

        memcpy(data + write_pos , src, num);
        write_pos += num;
        return true;
    }

    /* Function:        Buffer::readBytes
     * Description:     Reads data from the buffer.
     */
    bool Buffer::readBytes(unsigned char * dst, uint32 num)
    {
        uint32 possible = write_pos - read_pos;
        if(possible < num)
            return false;

        memcpy(dst, data + read_pos,num);
        read_pos += num;
        return true;
    }

    /* Function:        Buffer::reset
     * Description:     Resets the read and write positions.
     */
    void Buffer::reset()
    {
        read_pos = write_pos = 0;
    }
};