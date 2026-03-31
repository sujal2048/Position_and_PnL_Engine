// position_engine.cpp
#include "position_engine.hpp"
#include <sstream>
#include <cmath>

void PositionEngine::fill(const std::string& client_id, const std::string& type,
                          double quantity, double price) {
    auto& pos = positions_[client_id]; // creates default if not present
    if (type == "buy") {
        updatePositionOnBuy(pos, quantity, price);
    } else if (type == "sell") {
        updatePositionOnSell(pos, quantity, price);
    }
}

void PositionEngine::price(double price) {
    currentPrice_ = price;
}

std::string PositionEngine::print(const std::string& client_id) const {
    auto it = positions_.find(client_id);
    if (it == positions_.end()) {
        return R"({"error": "client not found"})";
    }
    const auto& pos = it->second;

    double unrealized = pos.netQty * (currentPrice_ - pos.avgPrice);

    std::ostringstream oss;
    oss << R"({"netQty":)" << pos.netQty
        << R"(,"avgPrice":)" << pos.avgPrice
        << R"(,"realizedPnL":)" << pos.realizedPnL
        << R"(,"unrealizedPnL":)" << unrealized << "}";
    return oss.str();
}

void PositionEngine::updatePositionOnBuy(Position& pos, double q, double p) {
    if (pos.netQty >= 0) {
        // Already long or flat
        double newQty = pos.netQty + q;
        double newAvg = (pos.netQty * pos.avgPrice + q * p) / newQty;
        pos.netQty = newQty;
        pos.avgPrice = newAvg;
    } else {
        // Currently short – buying to cover
        double shortQty = -pos.netQty;
        if (q <= shortQty) {
            pos.realizedPnL += q * (pos.avgPrice - p);  // profit if p < avgPrice
            pos.netQty += q;                           // less short
        } else {
            // Close entire short and go long
            pos.realizedPnL += shortQty * (pos.avgPrice - p);
            pos.netQty = q - shortQty;                 // now positive
            pos.avgPrice = p;                          // new long entry
        }
    }
}

void PositionEngine::updatePositionOnSell(Position& pos, double q, double p) {
    if (pos.netQty <= 0) {
        // Already short or flat
        double shortQty = -pos.netQty;
        double newShortQty = shortQty + q;
        double newAvg = (shortQty * pos.avgPrice + q * p) / newShortQty;
        pos.netQty = -newShortQty;
        pos.avgPrice = newAvg;
    } else {
        // Currently long – selling to reduce
        if (q <= pos.netQty) {
            pos.realizedPnL += q * (p - pos.avgPrice);
            pos.netQty -= q;
        } else {
            // Close entire long and go short
            pos.realizedPnL += pos.netQty * (p - pos.avgPrice);
            pos.netQty = -(q - pos.netQty);
            pos.avgPrice = p;
        }
    }
}
