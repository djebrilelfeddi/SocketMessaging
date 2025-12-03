/**
 * @file Socket.hpp
 * @brief Wrapper RAII pour file descriptors de sockets
 */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <unistd.h>

/**
 * @class Socket
 * @brief Gestion RAII de file descriptors avec move semantics
 * 
 * Fermeture automatique dans le destructeur. Non-copyable, move-only.
 */
class Socket {
public:
    /**
     * @brief Constructeur par défaut (socket invalide)
     */
    Socket() : fd_(-1) {}
    
    /**
     * @brief Constructeur avec fd existant
     * @param fd File descriptor à gérer
     */
    explicit Socket(int fd) : fd_(fd) {}
    
    /**
     * @brief Destructeur (ferme automatiquement)
     */
    ~Socket() {
        close();
    }
    
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    
    /**
     * @brief Move constructor
     * @param other Socket source
     */
    Socket(Socket&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }
    
    /**
     * @brief Move assignment
     * @param other Socket source
     * @return Référence à this
     */
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            close();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }
    
    /**
     * @brief Obtient le file descriptor
     * @return fd (-1 si invalide)
     */
    [[nodiscard]] int get() const { return fd_; }
    
    /**
     * @brief Vérifie la validité du socket
     * @return true si fd >= 0
     */
    [[nodiscard]] bool isValid() const { return fd_ >= 0; }
    
    /**
     * @brief Ferme le socket
     */
    void close() {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }
    
    /**
     * @brief Libère le fd sans fermer (transfert ownership)
     * @return fd original
     */
    int release() {
        int temp = fd_;
        fd_ = -1;
        return temp;
    }
    
private:
    int fd_;
};

#endif