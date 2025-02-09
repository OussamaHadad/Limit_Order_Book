#pragma once

#include <cstdint> // For fixed-width integer types

// Represents the side of an order (Bid or Ask)
enum class OrderSide {
    Bid = 0, // Buy order
    Ask = 1  // Sell order
};

// Represents the type of an order
enum class OrderType {
    LimitOrder,  // Buy/Sell at a specific price or better
    MarketOrder, // Buy/Sell immediately at the best available price
    StopOrder    // Buy/Sell when the price reaches a specified stop price
};

// Represents the time-in-force for an order
enum class TimeInForce {
    GTC, // Good Till Cancel: Order remains active until explicitly canceled
    DAY, // Day: Order expires at the end of the trading day
    IOC, // Immediate or Cancel: Order must be filled immediately or canceled
    FOK  // Fill or Kill: Order must be filled entirely or canceled
};

// Represents the category of an order (Limit or Stop)
enum class OrderCategory {
    Limit, // Order is a limit order
    Stop   // Order is a stop order
};