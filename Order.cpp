#include <iostream>
#include <assert.h>

#include "Order.h"
#include "Limit.h"


Order::Order(int _idNumber, Side _side, int _orderShares, int _limitPrice, OrderType _type, TIF _tif):
    idNumber(_idNumber), side(_side), orderShares(_orderShares), limitPrice(_limitPrice), 
    type(_type), tif(_tif), submissionTime(std::time(nullptr)),
    parentLimit(nullptr), previousOrder(nullptr), nextOrder(nullptr)
    {}


int Order::getOrderId() const{
    return idNumber;
}

Side Order::getOrderSide() const{
    return side;
}

int Order::getOrderShares() const{
    return orderShares;
}

int Order::getLimitPrice() const{
    return limitPrice;
}

Limit* Order::getParentLimit() const{
    return parentLimit;
}

Order* Order::getNextOrder() const{
    return nextOrder;
}

OrderType Order::getOrderType() const{
    return type;
}

TIF Order::getTIF() const{
    return tif;
}


void Order::showOrder() const{
    std::cout << "Following are the information of Order" << idNumber << std::endl;
    std::cout << "  Side: " << (Order::getOrderSide() == Side::Bid ? "Bid" : "Ask") << std::endl;
    std::cout << "  Number of shares: " << orderShares << std::endl;
    if (type == OrderType::LimitOrder)
        std::cout << "  Limit price: " << limitPrice << std::endl;
    std::cout << "  Order Type: " << (Order::getOrderType() == OrderType::LimitOrder ? "Limit Order" : 
        Order::getOrderType() == OrderType::MarketOrder ? "Market Order" : "Stop Order") << std::endl;
    //std::cout << "  Time in force: " << Order::getTIF() << std::endl;
}


void Order::amendOrder(int newShares, int newLimitPrice){
    /* An order is cancelled, then modified, and finally added to its limit
    When an order is updated, its submission time is updated */
    submissionTime = std::time(nullptr);

    if (limitPrice != newLimitPrice){
        limitPrice = newLimitPrice;
        parentLimit->numberOfOrders -= 1;
        parentLimit->totalShares -= orderShares;
        
        // The following pointers will be updated after calling this method in the order book using Limit::addOrder(Order* order)
        parentLimit = nullptr;
        previousOrder = nextOrder = nullptr;
    }
    
    if (orderShares != newShares)
        orderShares = newShares;
}

void Order::cancelOrder(){
    // Note on Cancelling & Executing orders: any order can be cancelled, BUT only the head order can be executed

    if (previousOrder != nullptr)
        previousOrder->nextOrder = nextOrder;
    else    // then we are cancelling the head order, hence parentLimit->headOrder should be updated
        parentLimit->headOrder = nextOrder;

    if (nextOrder != nullptr)
        nextOrder->previousOrder = previousOrder;
    else    // then we are cancelling the tail order, hence parentLimit->tailOrder should be updated
        parentLimit->tailOrder = previousOrder;
    
    // Finally, we remove the traded shares from the total shares of the limit level and the order
    parentLimit->numberOfOrders -= 1;
    parentLimit->totalShares -= orderShares;
    orderShares = 0;
}

void Order::executeOrder(int tradedShares){ // This implementatiion considers only Limit Orders
    /* Arguments:
        tradedShares (int): the number of shares from the other side of the book that will be traded */
        
    assert(tradedShares <= orderShares && "Error: The number of traded shares exceeds the number of order shares");
    
    // When executing an order there are two possibilities: Full execution OR Partial execution
    if (tradedShares == orderShares){ // Full execution -> Decrement number of trades by 1 & Eliminate this Order from its limit level
        parentLimit->numberOfOrders -= 1;

        Order* crnt_head = parentLimit->headOrder;
        
        if (crnt_head->nextOrder == nullptr)  // If there is only one order in the trade, then it is both head and tail, hence both should be updated
            parentLimit->headOrder = parentLimit->tailOrder = nullptr;
        else
            parentLimit->headOrder = crnt_head->nextOrder;
        
        previousOrder = nextOrder = nullptr;
    }

    // Finally, we remove the traded shares from the total shares of the order and the limit level in general
    orderShares -= tradedShares;
    parentLimit->totalShares -= tradedShares;
}

