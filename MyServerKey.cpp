#include <vector>
#include "CurrentState.h"
#include "MyServerKey.h"

//std::vector<std::string> split(const std::string& s, char delimiter) {
//    std::vector<std::string> tokens;
//    std::string token;
//    std::istringstream tokenStream(s);
//    while (std::getline(tokenStream, token, delimiter)) {
//        tokens.push_back(token);
//    }
//    return tokens;
//};

MyHandlerKey::MyHandlerKey(MyServerKey* server) : _server(server) {
}

void MyHandlerKey::onConnect(WebSocket* connection) {

    cout << "onConnect getRequestUri: " << connection->getRequestUri() << endl;
    //cout << "onConnect getRequestUri: " << connection->getHeader() << endl;

    //_connections.insert(connection);
    _connections.insert(connection);
    //connection->send(_currentSetValue);
    //sleep(1);
    cout << "Connected: " << connection->getRequestUri()
            << " : " << formatAddress(connection->getRemoteAddress())
            << endl;
    cout << "Credentials: " << *(connection->credentials()) << endl;
}

void MyHandlerKey::onData(WebSocket* connection, const char* data) {
    //        if (0 == strcmp("die", data)) {
    //            _server->terminate();
    //            return;
    //        }
    //        if (0 == strcmp("close", data)) {
    //            cout << "Closing.." << endl;
    //            connection->close();
    //            cout << "Closed." << endl;
    //            return;
    //        }
    //
    //        int value = atoi(data) + 1;
    //        if (value > _currentValue) {
    //            setValue(value);
    //            for (auto c : _connections) {
    //                c->send(_currentSetValue.c_str());
    //            }
    //        }

    string s(data);
    replace(s, "[", "");
    replace(s, "]", "");
    replace(s, " ", "");


    vector<std::string> vect = split(s, ',');
    vector<int> vect_int;

    for (auto const i : vect) {
        int v = std::stoi(i);
        vect_int.push_back(v);
        cout << "onData: " << v << endl;
    }

    int* keys = &vect_int[0];

    if (keys[1]) {
        CurrentState::get().LeftServoBeForward();
        CurrentState::get().RightServoBeForward();
    } else if (keys[4] || keys[3]) {
        if (keys[3]) {
            CurrentState::get().LeftServoBeBack();
            CurrentState::get().RightServoBeForward();
        } else if (keys[4]) {
            CurrentState::get().RightServoBeBack();
            CurrentState::get().LeftServoBeForward();
        }
    } else if (keys[2]) {
        CurrentState::get().LeftServoBeBack();
        CurrentState::get().RightServoBeBack();
    } else {
        CurrentState::get().LeftServoBeStop();
        CurrentState::get().RightServoBeStop();

    }
}

void MyHandlerKey::onDisconnect(WebSocket* connection) {
    _connections.erase(connection);
    cout << "Disconnected: " << connection->getRequestUri()
            << " : " << formatAddress(connection->getRemoteAddress())
            << endl;


}