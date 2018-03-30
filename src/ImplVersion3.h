#include "sftp_impl.h"
#include "attributes_v3.h"
#include "types.h"

namespace sftp
{
    /* Class:           ImplVersion3
     * Description:     SFTP version 3.
     */
    class ImplVersion3 : public sftp_impl
    {
        int getVersion() {return SFTP_VERSION_THREE;}
        uint32 getPacketLimit() {return 32000;}
        file_attributes * createAttributeInstance() {return new (std::nothrow) attributes_v3();}
    };
};