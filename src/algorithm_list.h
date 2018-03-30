#ifndef _ALGORITHM_LIST_H_
#define _ALGORITHM_LIST_H_

namespace ssh
{
    /* Struct:          algorithm_list
     * Description:     Contains a list of the selected algorithms.
     */
    struct algorithm_list
    {
        std::string     keyexchange_algorithm,          // the selected keyexchange algorithm   
                        hostkey_algorithm,              // the selected hostkey algorithm
                        cipher_client_algorithm,
                        cipher_server_algorithm,
                        hmac_client_algorithm,
                        hmac_server_algorithm,
                        compression_client_algorithm,
                        compression_server_algorithm;
    };
};
#endif