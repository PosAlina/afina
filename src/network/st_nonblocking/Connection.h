#ifndef AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
#define AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H

#include <cstring>
#include <sys/epoll.h>

#include <afina/Storage.h>
#include <afina/logging/Service.h>
#include <afina/execute/Command.h>
#include <protocol/Parser.h>

namespace Afina {
namespace Network {
namespace STnonblock {

class Connection {
public:
    Connection(int s, std::shared_ptr<spdlog::logger> logger, std::shared_ptr<Afina::Storage> storage,
               std::shared_ptr<Afina::Logging::Service> logging)
        : _socket(s),
          _logger(logger),
          pStorage(storage),
          pLogging(logging) {
        std::memset(&_event, 0, sizeof(struct epoll_event));
        _event.data.ptr = this;
    }

    inline bool isAlive() const { return _is_alive; }

    void Start();

protected:
    void OnError();
    void OnClose();
    void DoRead();
    void DoWrite();

private:
    friend class ServerImpl;

    int _socket;
    struct epoll_event _event;

    std::string _results;
    bool _is_alive;

    // logger to use
    std::shared_ptr<spdlog::logger> _logger;

    Protocol::Parser parser;
    std::unique_ptr<Afina::Execute::Command> command_to_execute;
    std::shared_ptr<Afina::Storage> pStorage;
    std::shared_ptr<Afina::Logging::Service> pLogging;
    
    std::size_t arg_remains;
    std::string argument_for_command;
    
};

} // namespace STnonblock
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
