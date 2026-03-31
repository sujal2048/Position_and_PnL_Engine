// test_position_engine.cpp
#include "position_engine.hpp"
#include <gtest/gtest.h>

TEST(PositionEngine, BuyAndSell) {
    PositionEngine engine;

    // Buy 100 @ 10
    engine.fill("client1", "buy", 100, 10.0);
    // Check position after buy
    auto res = engine.print("client1");
    EXPECT_TRUE(res.find("\"netQty\":100") != std::string::npos);
    EXPECT_TRUE(res.find("\"avgPrice\":10") != std::string::npos);
    EXPECT_TRUE(res.find("\"realizedPnL\":0") != std::string::npos);

    // Sell 50 @ 12
    engine.fill("client1", "sell", 50, 12.0);
    res = engine.print("client1");
    EXPECT_TRUE(res.find("\"netQty\":50") != std::string::npos);
    EXPECT_TRUE(res.find("\"avgPrice\":10") != std::string::npos);
    EXPECT_TRUE(res.find("\"realizedPnL\":100") != std::string::npos); // 50*(12-10)=100

    // Price update for unrealized
    engine.price(15.0);
    res = engine.print("client1");
    EXPECT_TRUE(res.find("\"unrealizedPnL\":250") != std::string::npos); // 50*(15-10)=250
}

TEST(PositionEngine, ShortAndCover) {
    PositionEngine engine;

    // Short 100 @ 20
    engine.fill("client2", "sell", 100, 20.0);
    auto res = engine.print("client2");
    EXPECT_TRUE(res.find("\"netQty\":-100") != std::string::npos);
    EXPECT_TRUE(res.find("\"avgPrice\":20") != std::string::npos);

    // Buy to cover 30 @ 18
    engine.fill("client2", "buy", 30, 18.0);
    res = engine.print("client2");
    EXPECT_TRUE(res.find("\"netQty\":-70") != std::string::npos);
    EXPECT_TRUE(res.find("\"realizedPnL\":60") != std::string::npos); // 30*(20-18)=60

    engine.price(19.0);
    res = engine.print("client2");
    EXPECT_TRUE(res.find("\"unrealizedPnL\":70") != std::string::npos); // 70*(20-19)=70
}

TEST(PositionEngine, FlipFromLongToShort) {
    PositionEngine engine;

    // Buy 50 @ 10
    engine.fill("client3", "buy", 50, 10.0);
    // Sell 80 @ 12
    engine.fill("client3", "sell", 80, 12.0);
    auto res = engine.print("client3");
    // 50 sold at 12: realized 50*(12-10)=100, remaining short 30 @ avg price 12
    EXPECT_TRUE(res.find("\"netQty\":-30") != std::string::npos);
    EXPECT_TRUE(res.find("\"avgPrice\":12") != std::string::npos);
    EXPECT_TRUE(res.find("\"realizedPnL\":100") != std::string::npos);

    engine.price(13.0);
    res = engine.print("client3");
    EXPECT_TRUE(res.find("\"unrealizedPnL\":-30") != std::string::npos); // short position loses if price rises
    // Actually unrealized for short: -30*(13-12)= -30
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
