#include "Client/Client.hpp"
#include "Client/ClientUI.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Constants.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::string serverIp = "127.0.0.1";
    int serverPort = Constants::DEFAULT_PORT;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-s" || arg == "--server") && i + 1 < argc) {
            serverIp = argv[++i];
        } else if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
            serverPort = std::atoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [-s ip] [-p port] [-h]\n";
            return 0;
        }
    }
    
    Logger::getInstance().setLogFile(Constants::DEFAULT_CLIENT_LOG);
    
    Client client(serverIp, serverPort);
    ClientUI ui(client, serverIp, serverPort);
    
    return ui.run() ? 0 : 1;
}
