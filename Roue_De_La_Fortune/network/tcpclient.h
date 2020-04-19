#ifndef CLIENT_H
#define CLIENT_H
#include <string>
#include <thread>

#ifdef WIN32
    #include <winsock2.h>
    #pragma comment(lib, "Ws2_32.lib")
#elif defined (linux)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
    typedef struct in_addr IN_ADDR;
#endif

class TCPClient
{
public:
    TCPClient(std::string ip, unsigned int port);
    ~TCPClient();

    void    sendMessage(std::string msg);
    void    disconnect();
    bool    connectToServer(std::string ip, unsigned int port);
    bool    connectToBaseServer();

private:
    unsigned int m_port;
    unsigned int m_socketFd;
    std::string m_ip;

    SOCKET  m_socket;
    std::thread* m_threadReceiver = nullptr;    

    bool listen = true;
    bool isConnected = false;

    bool    init();
    void    fn_threadReceiver();
};

#endif // CLIENT_H
