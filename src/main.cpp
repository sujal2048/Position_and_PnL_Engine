// main.cpp
#include "websocket_server.hpp"
#include "position_engine.hpp"
#include <iostream>
#include <boost/asio/signal_set.hpp>
#include <memory>

int main(int argc, char* argv[]) {
    try {
        auto const port = static_cast<unsigned short>(
            argc >= 2 ? std::stoi(argv[1]) : 8080);

        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { ioc.stop(); });

        auto engine = std::make_shared<PositionEngine>();
        tcp::endpoint endpoint(tcp::v4(), port);
        WebSocketServer server(ioc, endpoint, engine);
        server.run();

        std::cout << "Listening on port " << port << " ...\n";
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
