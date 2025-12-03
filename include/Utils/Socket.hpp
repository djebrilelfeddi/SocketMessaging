/**
 * @file Socket.hpp
 * @brief RAII wrapper for socket file descriptors
 */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <unistd.h>

/**
 * @class Socket
 * @brief RAII management of file descriptors with move semantics
 * 
 * Automatic closure in destructor. Non-copyable, move-only.
 */
class Socket {
public:
    /**
     * @brief Default constructor (invalid socket)
     */
    Socket() : fd_(-1) {}
    
    /**
     * @brief Constructor with existing fd
     * @param fd File descriptor to manage
     */
    explicit Socket(int fd) : fd_(fd) {}
    
    /**
     * @brief Destructor (closes automatically)
     */
    ~Socket() {
        close();
    }
    
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    
    /**
     * @brief Move constructor
     * @param other Source socket
     */
    Socket(Socket&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;
    }
    
    /**
     * @brief Move assignment
     * @param other Source socket
     * @return Reference to this
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
     * @brief Gets the file descriptor
     * @return fd (-1 if invalid)
     */
    [[nodiscard]] int get() const { return fd_; }
    
    /**
     * @brief Checks socket validity
     * @return true if fd >= 0
     */
    [[nodiscard]] bool isValid() const { return fd_ >= 0; }
    
    /**
     * @brief Closes the socket
     */
    void close() {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }
    
    /**
     * @brief Releases fd without closing (ownership transfer)
     * @return Original fd
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