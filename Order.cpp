#include <iostream>
#include <assert.h>

#include "Order.h"
#include "Trade.h"


Order::Order(int _idNumber, Side _side, int _shares, int _limitPrice, OrderType _type, TIF _tif):
    idNumber(_idNumber), side(_side), shares(_shares), limitPrice(_limitPrice), 
    type(_type), tif(_tif), submissionTime(std::time(nullptr)),
    parentTrade(nullptr), previousOrder(nullptr), nextOrder(nullptr)
    {}

void Order::showOrder() const{
    std::cout << "Following are the information of Order" << idNumber << std::endl;
    std::cout << "  Side: " << (Order::getOrderSide() == Side::Buy ? "Buy" : "Sell") << std::endl;
    std::cout << "  Number of shares: " << shares << std::endl;
    if (type == OrderType::LimitOrder)
        std::cout << "  Limit price: " << limitPrice << std::endl;
    std::cout << "  Order Type: " << (Order::getOrderType() == OrderType::LimitOrder ? "Limit Order" : 
        Order::getOrderType() == OrderType::MarketOrder ? "Market Order" : "Stop Order") << std::endl;
    //std::cout << "  Time in force: " << Order::getTIF() << std::endl;
}

int Order::getOrderId() const{
    return idNumber;
}

Side Order::getOrderSide() const{
    return side;
}

int Order::getShares() const{
    return shares;
}

int Order::getLimitPrice() const{
    return limitPrice;
}

OrderType Order::getOrderType() const{
    return type;
}

TIF Order::getTIF() const{
    return tif;
}

Trade* Order::getParentTrade() const{
    return parentTrade;
}

Order* Order::getNextOrder() const{
    return nextOrder;
}

void Order::amendOrder(int newShares, int newLimitPrice){
    // We assume that the initial submission time is conserved when updating an order
    shares = newShares;
    limitPrice = newLimitPrice;
    parentTrade = nullptr;
    previousOrder = nullptr;
    nextOrder = nullptr;
}

void Order::cancelOrder(){
    // One important difference between cancelling and executing an order is that any order can be cancelled, but only the head order can be executed

    parentTrade->numberOfTrades -= 1;

    if (previousOrder != nullptr)
        previousOrder->nextOrder = nextOrder;
    else    // then we are cancelling the head order, hence parentTrade->headOrder should be updated
        parentTrade->headOrder = nextOrder;

    if (nextOrder != nullptr)
        nextOrder->previousOrder = previousOrder;
    else    // then we are cancelling the tail order, hence parentTrade->tailOrder should be updated
        parentTrade->tailOrder = previousOrder;
    
    // Finally, we remove the traded shares from the total shares of the trade and the order
    parentTrade->totalShares -= shares;
    shares = 0;
}

void Order::executeOrder(int tradedShares){ // This implementatiion considers only Limit Orders
    assert(tradedShares <= shares && "Error: The number of traded shares exceeds the number of shares in the order");
    
    // When executing an order there are two possibilities: Full execution OR Partial execution
    if (tradedShares == shares){ // Full execution -> Decrement number of trades 1 Eliminate this Order from its Trade
        parentTrade->numberOfTrades -= 1;

        Order* crnt_head = parentTrade->headOrder;
        
        if (crnt_head->nextOrder == nullptr)  // If there is only one order in the trade, then it is both head and tail, hence both should be updated
            parentTrade->headOrder = parentTrade->tailOrder = nullptr;
        else
            parentTrade->headOrder = crnt_head->nextOrder;
        
        previousOrder = nextOrder = nullptr;
    }

    // Finally, we remove the traded shares from the total shares of the trade and the order
    shares -= tradedShares;
    parentTrade->totalShares -= tradedShares;
}

