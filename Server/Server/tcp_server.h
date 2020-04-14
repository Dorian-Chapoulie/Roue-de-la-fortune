#ifndef CLIENT_H
#define CLIENT_H
#include <string>
#include <thread>
#include <mutex>

#ifdef _WIN32
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

class TCPServer
{
public:
    TCPServer();
    TCPServer(std::string ip, unsigned int port);
    ~TCPServer();

    void    sendMessage(std::string msg, SOCKET& client);
    void    disconnect();    
    std::string getIp() const;
    unsigned int getPort() const;

//private:    
    const unsigned int MAX_CLIENT = 40;

    unsigned int m_port;
    unsigned int clientsNumber = 0;
    unsigned int m_socketFd;

    std::string m_ip;    

    SOCKET*     m_socketClients = new SOCKET[MAX_CLIENT];
    SOCKET      m_socketfd;    

    std::thread* m_threadReceiver = nullptr;
    std::thread* m_threadAccept = nullptr;
    std::mutex mutex;

    bool doListen = true;  
    bool waitForPlayers = true;

    void    init();
    void    fn_threadReceiver(SOCKET* client);    
    void    fn_threadAcceptNewClient();
};

#endif // CLIENT_H
