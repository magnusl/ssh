#ifndef _PROTOCOL_VERSION_H_
#define _PROTOCOL_VERSION_H_

namespace ssh
{
    bool supported_version(const std::string &);
    bool protocol_version_string(const std::string &);
    bool parse_protocol_version(const std::string & str, endpoint_info * info);
};
#endif