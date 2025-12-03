/**
 * @file NetworkStream.hpp
 * @brief Abstraction réseau avec encodage/décodage automatique
 */

#ifndef NETWORK_STREAM_HPP
#define NETWORK_STREAM_HPP

#include <string>
#include <optional>

namespace Network {

/**
 * @class NetworkStream
 * @brief Encapsule l'envoi/réception avec protocole [4-byte length][data]
 * 
 * Gère automatiquement l'encodage (préfixe de longueur) et le décodage.
 * Non-copyable.
 */
class NetworkStream {
public:
    /**
     * @brief Constructeur
     * @param socket File descriptor du socket connecté
     */
    explicit NetworkStream(int socket);
    
    NetworkStream(const NetworkStream&) = delete;
    NetworkStream& operator=(const NetworkStream&) = delete;
    
    /**
     * @brief Envoie un message (encodage automatique)
     * @param message Données à envoyer
     * @return true si envoi réussi
     */
    [[nodiscard]] bool send(const std::string& message);
    
    /**
     * @brief Reçoit un message (décodage automatique)
     * @return Message reçu ou std::nullopt si échec
     */
    [[nodiscard]] std::optional<std::string> receive();
    
    /**
     * @brief Vérifie si la connexion est active
     * @return true si connecté
     */
    [[nodiscard]] bool isConnected() const;
    
    /**
     * @brief Obtient le file descriptor
     * @return Socket fd
     */
    [[nodiscard]] int getSocket() const { return socketFd; }
    
private:
    std::string encodeMessage(const std::string& message);
    std::string decodeMessage(bool& success);
    
    int socketFd;
    bool connected;
};

} // namespace Network

#endif