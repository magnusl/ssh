/*#include <iostream>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "sftp_client.h"
#include <winsock2.h>

using namespace std;

ssh::Event ev;
sftp::sftp_directory dir;

class sftpclient : public sftp::sftp_client
{
public:

    void authenticate()
    {
        std::wstring pass;
        std::string user;
        cout << "Username: ";
        cin >> user;
        cout << "Password: ";
        wcin >> pass;

        m_ssh->password_authentication(user.c_str(),pass.c_str());  
    }

    void OnInitalized()
    {
        ev.signal();    
        //m_sftp->dir(L"/", new sftp::sftp_directory());
    }

    virtual void OnDirectoryListingSuccess(const sftp::sftp_directory * dir)
    {
        std::cerr << "Directory listing complete" << endl;
        ev.signal();
    }
    virtual void OnDirectoryListingFailure(const sftp::sftp_directory * dir)
    {
        std::cerr << "Directory listing failed" << endl;
        ev.signal();
    }

    void OnRemoveSuccess(const wchar_t * file)
    {
        std::wcerr << L"The file: " << file << L" was deleted" << endl;
        ev.signal();
    }

    void OnRemoveFailure(const wchar_t * file, const wchar_t * error)
    {
        std::wcerr << L"Could not delete the file: " << file << L", " << error << endl;
        ev.signal();
    }

    void OnChannelClose(ssh::ssh_channel * channel)
    {
        std::wcerr << L"The channel was closed, now terminating the SSH connection" << std::endl;
        m_ssh->shutdown();
    }

    void OnConnectionClosed()
    {
        std::cerr << "The connection was closed" << std::endl;
        ev.signal();
    }

    void OnDisconnect(const wchar_t * msg)
    {
        std::cerr << "The connection was closed" << std::endl;
    }
};

void wTokenize(const wstring& str, vector<wstring>& tokens,const wstring& delimiters = L" ")
{
    // Skip delimiters at beginning.
    wstring::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    wstring::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (wstring::npos != pos || wstring::npos != lastPos)
    {
        // Found a token, add it to the vector.
         tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

sftpclient client;

void wait(ssh::Event & ev)
{
    while(!ev.isSignaled()) {
        Sleep(100);
    }
}

 // Parses the command
 
void parse_cmd(const wstring & cmd)
{
    if(cmd.empty()) return;
    vector<wstring> tokens;
    wTokenize(cmd, tokens);
    if(tokens.size() == 0) return;
    if(tokens[0] == L"dir" || tokens[0] == L"ls") {
        if(tokens.size() == 1) {
            cerr << "Not enough arguments for this operation, was expecting a path" << endl;
            return;
        }
        ev.reset();
        client.dir(tokens[1].c_str(), &dir);
        // wait for the event.
        wait(ev);
    } else if(tokens[0] == L"send") {
        cout << "Upload a file" << endl;
    } else if(tokens[0] == L"get") {
        cout << "Download a file" << endl;
    } else if(tokens[0] == L"del") {
        if(tokens.size() == 1) {
            wcerr << L"Not enough arguments for this operation, was expecting a path" << endl;
            return;
        } else {
            wcout << L"Delete the file: " << tokens[1] << std::endl;
            ev.reset();
            client.remove(tokens[1].c_str());
            wait(ev);
        }
    }
}


int main()
{   
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,0), &wsaData);
    
    sftp::sftp_directory dir;
    
    client.Connect("192.168.233.128", "22");
    while(!ev.isSignaled()) {
        Sleep(100);
    }
    std::wstring pwd = L"/";
    std::wstring cmd;
    while(1)
    {
        std::wcout << pwd << L"> ";
        std::getline(wcin, cmd);
        if(cmd == L"quit") {
            ev.reset();
            client.close();
            // wait for the event.
            wait(ev);
        }
        parse_cmd(cmd);
    }

    WSACleanup();
}
*/