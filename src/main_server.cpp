#include "Server/Server.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Constants.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <thread>

Server* globalServer = nullptr;

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nSignal d'arrêt reçu...\n";
        if (globalServer) {
            globalServer->stop();
        }
        std::exit(0);
    }
}

int main(int argc, char* argv[]) {
    int port = Constants::DEFAULT_PORT;
    int maxConnections = 100;
    bool verbose = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                port = std::atoi(argv[++i]);
                if (port <= 0 || port > 65535) {
                    std::cerr << "Port invalide. Utilisation du port par défaut: " << Constants::DEFAULT_PORT << "\n";
                    port = Constants::DEFAULT_PORT;
                }
            } else {
                std::cerr << "Error: -p/--port requires an argument\n";
                return 1;
            }
        } else if (arg == "-c" || arg == "--connections") {
            if (i + 1 < argc) {
                maxConnections = std::atoi(argv[++i]);
                if (maxConnections <= 0) {
                    maxConnections = 100;
                }
            } else {
                std::cerr << "Error: -c/--connections requires an argument\n";
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [OPTIONS]\n";
            std::cout << "Options:\n";
            std::cout << "  -p, --port <port>           Server port (default: " << Constants::DEFAULT_PORT << ")\n";
            std::cout << "  -c, --connections <num>     Max connections (default: 100)\n";
            std::cout << "  -v, --verbose               Enable verbose logging (show DEBUG messages)\n";
            std::cout << "  -h, --help                  Show this help message\n";
            return 0;
        }
    }
    
    Logger::getInstance().setLogFile(Constants::DEFAULT_SERVER_LOG);
    Logger::getInstance().setVerbose(verbose);
    
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    Server server;
    globalServer = &server;
    
    if (server.start(port, maxConnections) != 0) {
        LOG_ERROR("Échec du démarrage du serveur");
        return 1;
    }
    
    while (server.getStatus() == SERVER_STATUS::RUNNING) {
        std::this_thread::sleep_for(std::chrono::seconds(Constants::MAIN_LOOP_SLEEP_S));
    }
    
    return 0;
}