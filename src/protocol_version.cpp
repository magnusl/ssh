/* File:            protocol_version
 * Description:     Contains the functions related to the protocol version
 *                  exchange.
 * Author:          Magnus Leksell
 *
 * Copyright 2006-2007 © Magnus Leksell, all rights reserved.
 *****************************************************************************/
#include <string>
#include "endpoint_info.h"

namespace ssh
{

    using namespace std;

    /* Function:        supported_version
     * Description:     Checks if the version supplied is supported by the current implementation
     */
    bool supported_version(const string & version)
    {
        return (version == "2.0" || version == "1.99");
    }

    /* Function:        protocol_version_string
     * Description:     Checks if the string is a protocol version string.
     */
    bool protocol_version_string(const std::string & str)
    {
        return (str.find("SSH-", 0) == 0);
    }

    /* Function:        parse_protocol_version
     * Description:     Parses the protocol version string and extracts the information.
     */
    bool parse_protocol_version(const std::string & str, endpoint_info * info)
    {
        string temp = str;
        size_t pos;

        if(!protocol_version_string(str))
            return false;
        temp = temp.substr(4);  // skip the "SSH-" part
        pos = temp.find_first_of("-");
        if(pos == string::npos)
            return false;
        info->version = temp.substr(0, pos);        // get the version.
        temp = temp.substr(pos);
        
        pos = temp.find_first_of(" ");
        if(pos == string::npos) {
            // no comment part
            info->software = temp;
        } else {
            info->software = temp.substr(0, pos);
            info->comment = temp.substr(pos);
        }
        return true;
    }
};
