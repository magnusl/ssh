#ifndef _SFTP_OBJECT_H_
#define _SFTP_OBJECT_H_

#include "ChannelObject.h"
#include <list>
#include <string>
#include "sftp_operation.h"
#include <map>
#include "Event.h"
#include "sequence_number.h"
#include "SynchronizedList.h"
#include "ArrayStream.h"

#define MODE_BINARY         0x01

#define SFTP_INITIAL_WINDOW     100000
#define SFTP_PACKET_LIMIT       120000

namespace sftp 
{
    class ssh_channel;

    /* Class:           sftp_object
     * Description:     Implements the SFTP protocol in the SSH "kernel".
     */
    class sftp_object : public ssh::ChannelObject
    {
    public:

        enum States {StateNotInitalized = 1, StateInitalized, StateWaitInit, StateWait, StateFinished};
        // constructor
        sftp_object(sftp_notify * notify, ssh::Event & ev);
        // destructor.
        virtual ~sftp_object();

        // writes data from a stream
        int write(unsigned char *, uint32, uint32 &);
        // reads data from a buffer
        int parse(unsigned char * src, uint32 count);
        // returns the status of the channel object.
        ChannelObject::ObjectStatus status();
        // increases the remote window size.
        bool increase_remote_window(uint32);        // increases the remote window.
        // initalizes the rempte data.
        bool initalize_remote(uint32 remote_channel, uint32 max_packet, uint32 remote_window);
        uint32 getInitialWindowSize() {return SFTP_INITIAL_WINDOW;}
        uint32 getPacketLimit() {return SFTP_PACKET_LIMIT;}
        // initiates a shutdown.
        void shutdown();
        /*
         * The SFTP requests, used by the client.
         */
        /* Sends a file */
        bool SendFiles(const wchar_t * local[], const wchar_t * remote[], uint32 num, uint32 mode = MODE_BINARY);
        /* Reads a file */
        bool sftp_object::ReadFiles(const wchar_t * local[], const wchar_t * remote[], uint32 num);
        /* Performs a directory listing */
        bool dir(const wchar_t *, sftp::sftp_directory *);
        bool remove(const std::list<std::wstring> & files);


        friend class ssh_channel;
    protected:

        void onEOF();
        void onClosed();

        bool initalize();
        int parsePacket(ssh::ArrayStream &);

        std::map<uint32, sftp_operation *> m_ops;       // the current operations
        ssh::SynchronizedList<sftp_operation *> m_queue;
        
        /* The SFTP implementation used */
        sftp_impl * impl;

        /* the extensions */
        std::list<std::pair<std::string, std::string> > m_extensions;
        
        sftp_notify * m_notify;
        uint32 m_state;
        uint32 m_version;

        ssh::Event & m_startEvent;
        ssh::Event m_quitEvent;
        ssh::sequence_number<uint32> m_requestId;

        uint32 packetSize, bytesRead;
        unsigned char * dst_buff;
    };

};

#endif