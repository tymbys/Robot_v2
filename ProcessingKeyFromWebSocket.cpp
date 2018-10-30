#include "ProcessingKeyFromWebSocket.h"
#include "MyServerKey.h"

ProcessingKeyFromWebSocket::ProcessingKeyFromWebSocket() {   
}

ProcessingKeyFromWebSocket::~ProcessingKeyFromWebSocket() {
}


void ProcessingKeyFromWebSocket::InitServer() {

    server = thread([this]() {
        //logger = std::make_shared<PrintfLogger>(Logger::Level::DEBUG);
        logger = std::make_shared<PrintfLogger>(verbose ? Logger::Level::DEBUG : Logger::Level::ACCESS);

        ws_server = make_shared<MyServerKey>(logger);
        //ws_server = new MyServer(logger);

        //ws_server->setClientBufferSize(100 * 1024 * 1020);
        ws_server->setLameConnectionTimeoutSeconds(60);

        //shared_ptr<MyHandler> handler = std::make_shared<MyHandler>(ws_server);
        handler = std::make_shared<MyHandlerKey>(ws_server.get());
        //handler->chooseProtocol();
        ws_server->addWebSocketHandler("/ws_key", handler);

        ws_server->serve(".", this->port);
    });

}

void ProcessingKeyFromWebSocket::SetVerbose(bool v) {
    this->verbose = v;
}

void ProcessingKeyFromWebSocket::SetPort(int port) {
    this->port = port;
}