// position_engine.hpp
#ifndef POSITION_ENGINE_HPP
#define POSITION_ENGINE_HPP

#include <string>
#include <unordered_map>

struct Position {
    double netQty = 0.0;
    double avgPrice = 0.0;
    double realizedPnL = 0.0;
};

class PositionEngine {
public:
    void fill(const std::string& client_id, const std::string& type,
              double quantity, double price);
    void price(double price);
    std::string print(const std::string& client_id) const; // returns JSON

private:
    std::unordered_map<std::string, Position> positions_;
    double currentPrice_ = 0.0;

    void updatePositionOnBuy(Position& pos, double quantity, double price);
    void updatePositionOnSell(Position& pos, double quantity, double price);
};

#endif
