#include "tcp_server.h"
#include "Config.h"
#include "protocolhandler.h"
#include <algorithm>

#ifdef _WIN32
	#define MSG_NOSIGNAL 0
#endif

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
    disconnect();
    //shutdown(m_socketFd, 0);
    //shutdown(m_socketFd, 1);

	//wait for running thread to terminate
    while (threadsRunning > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    //std::cout << "thread delete ok" << std::endl;
    delete m_threadAccept;
    delete m_threadReceiver;    
}

void TCPServer::sendMessage(std::string msg, SOCKET& client)
{
  
    if (send(client, msg.c_str(), msg.length(), MSG_NOSIGNAL) < 0) {

        auto it = std::find(m_socketClients.begin(), m_socketClients.end(), client);
    	//if the client is disconnected
        if (it != m_socketClients.end()) {
            m_socketClients.erase(it);
            protocolHandler->callEventFromProtocol("D-" + std::to_string(client), &client);
        }
    }
}

void TCPServer::disconnect()
{
    doListen = false;    
    closesocket(m_socketfd);
#ifdef WIN32
    WSACleanup();
#endif
}

void TCPServer::disconnectClient(int socketFd)
{
    closesocket(socketFd);
}

bool TCPServer::isListening() const
{
    return doListen;
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

//function that will be running in a separated thread
//1 thread / client
void TCPServer::fn_threadReceiver(SOCKET* client)
{
    char buffer[255];
    int bytesReceived = 0;
    while (doListen) {
        bytesReceived = recv(*client, buffer, 255 - 1, 0);
        if (bytesReceived > 0 && doListen) {
            ////std::cout << "Client: " << std::string(buffer, bytesReceived) << std::endl;
            mutex.lock();
            protocolHandler->callEventFromProtocol(std::string(buffer, bytesReceived), client);
            mutex.unlock();
            bytesReceived = 0;
        }
        else {
            break;
        }
    }
    //std::cout << "Thread Receiver done" << std::endl;
    threadsRunning--;
}

//thread for accepting client
void TCPServer::fn_threadAcceptNewClient()
{
    SOCKADDR clientSockAddr;
    int size = sizeof(clientSockAddr);
    while (waitForPlayers && doListen) {     
        SOCKET tempSocket = 0;
        #ifdef _WIN32
            tempSocket = accept(m_socketfd, (SOCKADDR*)&clientSockAddr, &size);
        #else
            tempSocket = accept(m_socketfd, (SOCKADDR*)&clientSockAddr, (socklen_t*)&size);
        #endif

        if (m_socketClients.size() == MAX_CLIENT - 1)
            waitForPlayers = false;

        if (tempSocket < 0) {
            //printf("server acccept failed...\n");
            exit(0);
        }
        else if (static_cast<unsigned int>(tempSocket) != -1) {
            //if the client is valid, we store the socket and create a new thread that will listen on this socket
            m_socketClients.push_back(tempSocket);
            m_threadReceiver = new std::thread(&TCPServer::fn_threadReceiver, this, &m_socketClients.back());
            m_threadReceiver->detach();     
            threadsRunning++;
        }
    }
    //std::cout << "Thread Accept closed" << std::endl;
    threadsRunning--;
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
        //printf("socket bind failed...\n");
        return;
    }

    //int yes = '1';
    //setsockopt(m_socketfd,IPPROTO_TCP,TCP_NODELAY,(char*)&yes,sizeof(int));

    int list = listen(m_socketfd, MAX_CLIENT);
    if (list != 0) {
        //printf("Listen failed...\n");
        //exit(0);
    }
    
    m_threadAccept = new std::thread(&TCPServer::fn_threadAcceptNewClient, this);
    m_threadAccept->detach();
    threadsRunning++;
}
