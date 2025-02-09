#include <iostream>
#include <assert.h>
#include <chrono>

#include "Order.h"
#include "Limit.h"


Order::Order(int _idNumber, OrderSide _orderSide, int _orderShares, int _limitPrice, OrderType _orderType, TimeInForce _tif): 
    idNumber(_idNumber), orderSide(_orderSide), orderShares(_orderShares), limitPrice(_limitPrice),
    orderType(_orderType), tif(_tif), submissionTime(std::time(nullptr)),
    parentLimit(nullptr), previousOrder(nullptr), nextOrder(nullptr)
{}

void Order::displayOrder() const {
    std::cout << "Following are the information of Order " << idNumber << std::endl;
    std::cout << "  Order Side: " << (orderSide == OrderSide::Bid ? "Bid" : "Ask") << std::endl;
    std::cout << "  Number of shares: " << orderShares << std::endl;
    if (orderType == OrderType::LimitOrder)
        std::cout << "  Limit price: " << limitPrice << std::endl;
    std::cout << "  Order Type: " << (orderType == OrderType::LimitOrder ? "Limit Order" 
        : orderType == OrderType::MarketOrder ? "Market Order" : "Stop Order") << std::endl;
}

void Order::amendOrder(int newShares, int newLimitPrice) {
    if (newShares <= 0)
        throw std::invalid_argument("Order shares must be positive");

    submissionTime = std::time(nullptr);

    if (limitPrice != newLimitPrice) {
        limitPrice = newLimitPrice;
        if (parentLimit) {
            parentLimit->numberOfOrders -= 1;
            parentLimit->totalShares -= orderShares;
        }
        parentLimit = nullptr;
        previousOrder = nextOrder = nullptr;
    }

    orderShares = newShares;
}

// Note: Cancel VS Execute: any order can be cancelled, but only the head order can be executed

void Order::cancelOrder() {
    if (!parentLimit) 
        return;

    if (previousOrder)
        previousOrder->nextOrder = nextOrder;
    else
        parentLimit->headOrder = nextOrder;

    if (nextOrder)
        nextOrder->previousOrder = previousOrder;
    else
        parentLimit->tailOrder = previousOrder;

    parentLimit->numberOfOrders -= 1;
    parentLimit->totalShares -= orderShares;
    orderShares = 0;
}

void Order::executeOrder(int tradedShares) {
    assert(tradedShares > 0 && tradedShares <= orderShares && "Invalid traded shares");

    if (tradedShares == orderShares) { // Full execution
        parentLimit->numberOfOrders -= 1;

        if (parentLimit->headOrder == this) {
            parentLimit->headOrder = nextOrder;
            if (!nextOrder) 
                parentLimit->tailOrder = nullptr;
        }
        previousOrder = nextOrder = nullptr;
    }

    orderShares -= tradedShares;
    parentLimit->totalShares -= tradedShares;
}


