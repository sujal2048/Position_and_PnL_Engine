// websocket_server.hpp
#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <memory>

class PositionEngine;

class WebSocketServer {
public:
    WebSocketServer(boost::asio::io_context& ioc,
                    const boost::asio::ip::tcp::endpoint& endpoint,
                    std::shared_ptr<PositionEngine> engine);
    void run();

private:
    boost::asio::io_context& ioc_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<PositionEngine> engine_;
    void doAccept();
};

#endif
