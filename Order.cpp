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
    /* Note: After calling this method, 
        1° Add limit level to limit/stop map
        2° Add the modified order to its limit/stop DLL in the limit/stop map
        3° Add the new number of shares to the limit level, if it's a limit level
    */
    if (newShares <= 0)
        throw std::invalid_argument("Order shares must be positive");

    submissionTime = std::time(nullptr);
    orderShares = newShares;

    if (limitPrice != newLimitPrice) {
        limitPrice = newLimitPrice;
            
        parentLimit->numberOfOrders -= 1;
        parentLimit->totalShares -= orderShares;
        
        parentLimit = nullptr;
        previousOrder = nextOrder = nullptr;
    }

    // TO DO: Update Head & Tail orders
}

void Order::cancelOrder() {
    /* Note: After cancelling an order, 
        1° Delete its limit/stop level if it's empty
        2° Remove order from its map
        3° Finally delete order
    */
    if (!parentLimit) 
        return;

    if (previousOrder)
        previousOrder->setNextOrder(nextOrder);
    else
        parentLimit->setHeadOrder(nextOrder);

    if (nextOrder)
        nextOrder->setPreviousOrder(previousOrder);
    else
        parentLimit->setTailOrder(previousOrder);

    parentLimit->numberOfOrders -= 1;
    parentLimit->totalShares -= orderShares;
    orderShares = 0;
}

void Order::executeOrder(int tradedShares) {
    /* Note: After an order is fully executed:
        1° Delete limit level if no order is left
        2° Delete order from orders map
        3° Delete order
    */
    assert(tradedShares > 0 && tradedShares <= orderShares && "Invalid traded shares");

    if (tradedShares == orderShares) { // Full execution
        //parentLimit->numberOfOrders -= 1; No need to do it here as it will be decremented when the order is removed

        if (parentLimit->getHeadOrder() == this) {
            parentLimit->setHeadOrder(nextOrder);
            if (!nextOrder) 
                parentLimit->setTailOrder(nullptr);
        }
        previousOrder = nextOrder = nullptr;
    }

    orderShares -= tradedShares;
    parentLimit->totalShares -= tradedShares;
}
