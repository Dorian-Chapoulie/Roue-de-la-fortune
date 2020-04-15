#include "network/tcpclient.h"
#include "protocol/protocolhandler.h"
#include "event/eventmanager.h"

TCPClient::TCPClient(std::string ip, unsigned int port)
{  
    m_ip = ip;
    m_port = port;
    init();
}

TCPClient::~TCPClient()
{    
    delete m_threadReceiver;
    disconnect();
}
#include <iostream>
void TCPClient::sendMessage(std::string msg)
{
    if(!isConnected)
        init();

    std::cout << "send: " << msg << std::endl;
    if(send(m_socket, msg.c_str(), msg.length(), 0) < 0){
        EventManager::getInstance()->triggerEvent(EventManager::EVENT::CONNEXION_FAILURE,
                                                  "Une erreur de communication avec le serveur est survenue. Veuillez réessayer plus tard.");
        isConnected = false;
        listen = false;
    }
    //return true / false;
}

void TCPClient::disconnect()
{    
    listen = false;
    isConnected = false;

    closesocket(m_socket);
    #ifdef WIN32
        WSACleanup();
    #endif
}

bool TCPClient::connectToServer(std::string ip, unsigned int port)
{
    if(isConnected)
        disconnect();

    m_ip = ip;
    m_port = port;
    return init();
}

#include <iostream>
void TCPClient::fn_threadReceiver()
{
    ProtocolHandler protocolHandler;
    char buffer[255];
    int bytesReceived = 0;
    while(listen) {
        bytesReceived = recv(m_socket, buffer, 255 - 1, 0);
        if(bytesReceived > 0) {            
            protocolHandler.callEventFromProtocol(std::string(buffer, bytesReceived));
            std::cout << "received: " << std::string(buffer, bytesReceived) << std::endl;
            bytesReceived = 0;
            memset(buffer, 0, 255);
        }
    }
}

bool TCPClient::init()
{
    #ifdef WIN32
        WSADATA wsa;
        int err = WSAStartup(MAKEWORD(2, 2), &wsa);
        if(err < 0) {
            puts("WSAStartup failed !");
            exit(EXIT_FAILURE);
            //TODO error
            return false;
        }
    #endif

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(m_socket == INVALID_SOCKET){
        perror("socket()");
        exit(errno);
        //TODO errrur
        return false;
    }


    struct hostent *hostinfo = nullptr;
    SOCKADDR_IN sin;

    hostinfo = gethostbyname(m_ip.c_str());
    if (hostinfo == NULL){
        fprintf (stderr, "Unknown host %s.\n", m_ip.c_str());
        exit(EXIT_FAILURE);
        //TODO erreur
        return false;
    }

    sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
    sin.sin_port = htons(m_port);
    sin.sin_family = AF_INET;        

    if(connect(m_socket,(SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        EventManager::getInstance()->triggerEvent(EventManager::EVENT::CONNEXION_FAILURE, "Connection au serveur impossible.");
        return false;
    }else {        
        isConnected = true;
        listen = true;
        m_threadReceiver = new std::thread(&TCPClient::fn_threadReceiver, this);
        m_threadReceiver->detach();
    }
    return true;
}
