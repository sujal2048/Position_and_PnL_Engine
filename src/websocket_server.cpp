// websocket_server.cpp
#include "websocket_server.hpp"
#include "position_engine.hpp"
#include <boost/beast/websocket.hpp>
#include <boost/json.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace json = boost::json;
using tcp = net::ip::tcp;

// Session handles one client connection
class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, std::shared_ptr<PositionEngine> engine)
        : ws_(std::move(socket)), engine_(engine) {}

    void run() {
        ws_.async_accept(
            [self = shared_from_this()](beast::error_code ec) {
                if (!ec) self->readMessage();
            });
    }

private:
    websocket::stream<tcp::socket> ws_;
    std::shared_ptr<PositionEngine> engine_;
    beast::flat_buffer buffer_;

    void readMessage() {
        ws_.async_read(buffer_,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                if (!ec) self->handleMessage();
                else if (ec != websocket::error::closed)
                    std::cerr << "read error: " << ec.message() << "\n";
            });
    }

    void handleMessage() {
        auto data = beast::buffers_to_string(buffer_.data());
        buffer_.consume(buffer_.size());

        // Parse JSON
        json::value jv;
        try {
            jv = json::parse(data);
        } catch (const std::exception& e) {
            sendError("Invalid JSON: " + std::string(e.what()));
            return readMessage();
        }

        auto& obj = jv.as_object();
        auto cmd = obj.find("command");
        if (cmd == obj.end() || !cmd->value().is_string()) {
            sendError("Missing 'command' field");
            return readMessage();
        }

        std::string command = cmd->value().as_string().c_str();

        if (command == "FILL") {
            // client_id, type, quantity, price
            auto client = obj.find("client_id");
            auto type = obj.find("type");
            auto qty = obj.find("quantity");
            auto price = obj.find("price");
            if (client == obj.end() || !client->value().is_string() ||
                type == obj.end() || !type->value().is_string() ||
                qty == obj.end() || !qty->value().is_double() ||
                price == obj.end() || !price->value().is_double()) {
                sendError("Missing/invalid FILL fields");
                return readMessage();
            }
            engine_->fill(client->value().as_string().c_str(),
                          type->value().as_string().c_str(),
                          qty->value().as_double(),
                          price->value().as_double());
            sendOk();
        } else if (command == "PRICE") {
            auto price = obj.find("price");
            if (price == obj.end() || !price->value().is_double()) {
                sendError("Missing/invalid PRICE field");
                return readMessage();
            }
            engine_->price(price->value().as_double());
            sendOk();
        } else if (command == "PRINT") {
            auto client = obj.find("client_id");
            if (client == obj.end() || !client->value().is_string()) {
                sendError("Missing 'client_id' for PRINT");
                return readMessage();
            }
            std::string response = engine_->print(client->value().as_string().c_str());
            ws_.text(true);
            ws_.async_write(net::buffer(response),
                [self = shared_from_this()](beast::error_code ec, std::size_t) {
                    if (ec) std::cerr << "write error: " << ec.message() << "\n";
                    else self->readMessage();
                });
            return; // wait for write completion before reading next
        } else {
            sendError("Unknown command: " + command);
        }

        // Continue reading next message
        readMessage();
    }

    void sendOk() {
        sendResponse(R"({"status":"OK"})");
    }

    void sendError(const std::string& msg) {
        sendResponse(R"({"error":")" + msg + R"("})");
    }

    void sendResponse(const std::string& msg) {
        ws_.text(true);
        ws_.async_write(net::buffer(msg),
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                if (ec) std::cerr << "write error: " << ec.message() << "\n";
            });
    }
};

WebSocketServer::WebSocketServer(boost::asio::io_context& ioc,
                                 const tcp::endpoint& endpoint,
                                 std::shared_ptr<PositionEngine> engine)
    : ioc_(ioc), acceptor_(ioc), engine_(engine) {
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(net::socket_base::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen(net::socket_base::max_listen_connections);
}

void WebSocketServer::run() {
    doAccept();
}

void WebSocketServer::doAccept() {
    acceptor_.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket), engine_)->run();
            }
            doAccept(); // continue accepting
        });
}
