#include "StompProtocol.h"
#include <string>
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
    try {
        ConnectionHandler* connectionHandler = nullptr;
        StompProtocol StompProtocol(connectionHandler);
        StompProtocol.initiate();
    } catch (const exception& exception) {
        cout << "Error: " << exception.what() << endl;
        return 1;
    }

    return 0;
}