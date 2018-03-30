/* File:            sftp_send_file.cpp
 * Description:     Handles the sending of a file.
 * Author:          Magnus Leksell
 *
 * Copyright © 2007 Magnus Leksell, all rights reserved. 
 *****************************************************************************/

#include "sftp_send_file.h"
#include <iostream>

using namespace std;

namespace sftp
{
    /* Function:        sftp_send_file::sftp_send_file
     * Description:     Constructor, performs the required initalization.
     */
    sftp_send_file::sftp_send_file(sftp_connection * con, 
                        sftp_notify * notify,
                        bool binary) : 
                        sftp_file_transfer(con, notify)
    {   
    }

    /*
     *
     */
    sftp_send_file::~sftp_send_file()
    {
    }

    /* Function:        sftp_send_file::OnWriteSuccess
     * Description:     Called when the previous write request succeded.
     */
    void sftp_send_file::OnWriteSuccess(uint32 num)
    {
        m_transfered += num;
        time_t current_time;
        time(&current_time);
        if(m_notify) m_notify->OnTransferStatus(this, difftime(current_time, m_start), m_transfered);
        if(m_transfered < m_size) {
            // now send some more.
            sendData();
        }
        else {  
            m_con->close(&m_handle, this);
        }
    }   

    /* Function:        sftp_send_file::BeginTransfer
     * Description:     Begins the file transfer.
     */
    bool sftp_send_file::BeginTransfer(const wchar_t * localfile, const wchar_t * remotefile)
    {
        sftp_file_attributes attributes;
        if(!attributes.set(localfile)) {
            wcerr << L"Could not acquire information about the file '" << m_localfile <<L"'" << endl;
            return false;
        }
        // SFTP_TRANSFER_BUFFER_SIZE should never be > 2^32, so the cast should'nt be any problem.
        m_bufflen = (uint32) min(attributes.m_size, (uint64)SFTP_TRANSFER_BUFFER_SIZE);
        if(m_bufflen > 0) {
            m_buff = new (std::nothrow) unsigned char[m_bufflen];
            if(!m_buff) {
                wcerr << L"Could not allocate memory for the transfer buffer" << endl;
                return false;
            }
        }

        // Open the local file
        m_input.open(localfile, ios_base::in | ios_base::binary);
        if(!m_input) {
            wcerr << L"Could not open '" << m_localfile <<L"' for reading" << std::endl;
            return false;
        }
        // create the remote file.
        if(!m_con->create_file(remotefile,&m_handle, this, attributes)) {
            return false;
        }
        /* now wait until the remote file is opened, will be notified about this */
        return true;
    }

    /* Function:        sftp_send_file::OnOpenFileSuccess
     * Description:     Called when the remote file is opened.
     */
    void sftp_send_file::OnOpenFileSuccess()
    {
        // the file was opened.
    }


    /* Function:        sftp_send_file::OnWriteFailure
     * Description:     Called when the previous write request failed.
     */
    void sftp_send_file::OnWriteFailure(uint32 reason)
    {
        if(m_notify) m_notify->OnTransferFailure(this, reason);
        cleanup();
    }

    /* Function:        sftp_send_file::pause
     * Description:     The user paused the file transfer
     */
    bool sftp_send_file::pause()
    {
        return true;
    }

    /* Function:        sftp_send_file::resume
     * Description:     The user resumed the file transfer
     */
    bool sftp_send_file::resume()
    {
        return true;
    }

    /* Function:        sftp_send_file::stop
     * Description:     The user stopped the file transfer.
     */
    bool sftp_send_file::stop()
    {
        return true;
    }   

    /* Function:        sftp_send_file::stop
     * Description:     Sends the actual data.
     */
    void sendData()
    {
    }




};