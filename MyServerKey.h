#ifndef MYSERVERKEY_H
#define MYSERVERKEY_H


#include "seasocks/PrintfLogger.h"
#include "seasocks/Server.h"
#include "seasocks/StringUtil.h"
#include "seasocks/WebSocket.h"
#include "seasocks/util/Json.h"


#include <iostream>
#include <cstdio>
#include <chrono>
#include <cstdlib>
#include <getopt.h>
#include <memory>
#include <cstring>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <mutex>


using namespace std::chrono;
using namespace seasocks;
using namespace std;

class MyServerKey : public Server {
public:
    using Server::Server;
private:

    virtual void checkThread() const override {

    };
};

class MyHandlerKey : public WebSocket::Handler {
public:

    explicit MyHandlerKey(MyServerKey* server);

    virtual void onConnect(WebSocket* connection);
    virtual void onData(WebSocket* connection, const char* data);
    virtual void onDisconnect(WebSocket* connection);

private:
    set<WebSocket*> _connections;
    Server* _server;
    int _currentValue;
    string _currentSetValue;
    std::mutex _mutex;

    void setValue(int value);
};

#endif /* MYSERVERKEY_H */

