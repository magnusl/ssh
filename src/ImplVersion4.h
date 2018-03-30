#include "sftp_impl.h"
#include "attributes_v4.h"
#include "types.h"

namespace sftp
{
    /* Class:           ImplVersion3
     * Description:     SFTP version 3.
     */
    class ImplVersion4 : public sftp_impl
    {
        int getVersion() {return SFTP_VERSION_FOUR;}
        uint32 getPacketLimit() {return 32000;}
        file_attributes * createAttributeInstance() {return new (std::nothrow) attributes_v4();}
    };
};