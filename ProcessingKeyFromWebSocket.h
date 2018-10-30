#ifndef PROCESSINGKEYFROMWEBSOCKET_H
#define PROCESSINGKEYFROMWEBSOCKET_H

#include <cstdio>
#include <cstdlib>

#include "WebServer.h"
#include "MyServerKey.h"

class ProcessingKeyFromWebSocket {
public:
    ProcessingKeyFromWebSocket();
    ProcessingKeyFromWebSocket(const ProcessingKeyFromWebSocket& orig);
    virtual ~ProcessingKeyFromWebSocket();    
    
    static void threadServer();
    void InitServer();
    
    void SetVerbose(bool v);
    void SetPort(int port);
    
private:
    
    thread server;
    
    shared_ptr<PrintfLogger> logger;
    shared_ptr<MyServerKey> ws_server;
    shared_ptr<MyHandlerKey> handler;
    
    bool verbose;
    int port;

};

#endif 

