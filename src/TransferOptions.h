#ifndef _TRANSFEROPTIONS_H_
#define _TRANSFEROPTIONS_H_

namespace sftp
{
    /* Class:           TransferOptions
     * Description:     Used to notify the client about the possible options regarding 
     *                  a operation.
     */
    class TransferOptions
    {
    public: 
        void Cancel()       // Cancels the transfer operation
        {
        }

        void Skip()         // skips the current element in the transfer
        {
        }

        void ReTry()        // try the current operation again
        {
        }

        void Resume()       // indicates thats it's possible to resume a file transfer
        {
        }

        void Overwrite()    // indicates thats it's possible to overwrite it.
        {
        }
    };
};

#endif