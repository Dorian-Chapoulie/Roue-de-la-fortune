#include "tcp_server.h"
#include "Config.h"
#include "protocolhandler.h"
#include <iostream>

TCPServer::TCPServer(ProtocolHandler* protocolHandler)
{
    m_ip = Config::getInstance()->baseIp;
    m_port = Config::getInstance()->basePort;
    this->protocolHandler = protocolHandler;
    init();
}

TCPServer::TCPServer(std::string ip, unsigned int port, ProtocolHandler* protocolHandler)
{
    m_ip = ip;
    m_port = port;
    this->protocolHandler = protocolHandler;
    init();
}

TCPServer::~TCPServer()
{
    delete m_threadReceiver;
    disconnect();
}

void TCPServer::sendMessage(std::string msg, SOCKET& client)
{
    //std::cout << "Send: " << msg << std::endl;
    if (send(client, msg.c_str(), msg.length(), 0) < 0) {

        auto it = std::find(m_socketClients.begin(), m_socketClients.end(), client);
        if (it != m_socketClients.end()) {
            m_socketClients.erase(it);
            std::cout << m_port << ": client disconnected: " << client <<  std::endl;
        }
        else {
            std::cout << "Erreur lors de la supression du client" << std::endl;
        }
        //TODO erreur
    }
}

void TCPServer::disconnect()
{
    doListen = false;
    m_threadReceiver->join();
    closesocket(m_socketfd);
#ifdef WIN32
    WSACleanup();
#endif
}

std::string TCPServer::getIp() const
{
    return m_ip;
}

unsigned int TCPServer::getPort() const
{
    return m_port;
}

std::vector<SOCKET> TCPServer::getClients()
{
    return m_socketClients;
}

void TCPServer::fn_threadReceiver(SOCKET* client)
{
    char buffer[255];
    int bytesReceived = 0;
    while (doListen) {
        bytesReceived = recv(*client, buffer, 255 - 1, 0);
        if (bytesReceived > 0) {
            //std::cout << "Client: " << std::string(buffer, bytesReceived) << std::endl;
            protocolHandler->callEventFromProtocol(std::string(buffer, bytesReceived), client);
            bytesReceived = 0;
        }
    }
}

void TCPServer::fn_threadAcceptNewClient()
{
    SOCKADDR clientSockAddr; // list ?
    int size = sizeof(clientSockAddr);    
    std::cout << "Server " << m_ip << ":" << m_port << " listening" << std::endl;
    while (waitForPlayers) {     

        #ifdef _WIN32
            m_socketClients.push_back(accept(m_socketfd, (SOCKADDR*)&clientSockAddr, &size));
        #else
            m_socketClients.push_back(accept(m_socketfd, (SOCKADDR*)&clientSockAddr, (socklen_t*)&size));
        #endif

        if (m_socketClients.size() == MAX_CLIENT)
            waitForPlayers = false;
        
        if (m_socketClients.back() < 0) {
            printf("server acccept failed...\n");
            exit(0);
        }
        else {               
            std::cout << "Server: " << m_port << ", new client: " << m_socketClients.back() << std::endl;
            m_threadReceiver = new std::thread(&TCPServer::fn_threadReceiver, this, &m_socketClients.back());
            m_threadReceiver->detach();
        }
    }
}

void TCPServer::init()
{
#ifdef WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (err < 0) {
        puts("WSAStartup failed !");
        exit(EXIT_FAILURE);
        //TODO error
    }
#endif

    m_socketClients.reserve(MAX_CLIENT);
    m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socketfd == INVALID_SOCKET) {
        perror("socket()");
        exit(errno);
        //TODO errrur
    }

   
    SOCKADDR_IN sin;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(m_port);
    sin.sin_family = AF_INET;

    if ((bind(m_socketfd, (SOCKADDR*)&sin, sizeof(sin))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }

    int list = listen(m_socketfd, MAX_CLIENT);
    if (list != 0) {
        printf("Listen failed...\n");
        exit(0);
    }

    m_threadAccept = new std::thread(&TCPServer::fn_threadAcceptNewClient, this);
    m_threadAccept->detach();
}
