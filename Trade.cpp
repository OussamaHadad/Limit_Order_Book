#include <iostream>
#include "Trade.h"


Trade::Trade(int _limitPrice, Side _side, int _numberOfTrades, int _totalShares):
    limitPrice(_limitPrice), side(_side), numberOfTrades(_numberOfTrades), totalShares(_totalShares), 
    headOrder(nullptr), tailOrder(nullptr), parentTrade(nullptr), leftChildTrade(nullptr), rightChildTrade(nullptr) 
    {}

Trade::~Trade(){
    ;
}

void Trade::showTrade() const{
    Order* pseudoHeadOrder = headOrder;
    std::cout << "Following are the IDs of Orders part of this Trade" << std::endl;
    while (pseudoHeadOrder != nullptr){
        std::cout << "Order ID: " << pseudoHeadOrder->getOrderId() << std::endl;
        pseudoHeadOrder = pseudoHeadOrder->getNextOrder();
    }
}

int Trade::getLimitPrice() const{
    return limitPrice;
}

int Trade::getNumberOfTrades() const{
    return numberOfTrades;
}

int Trade::getTotalShares() const{
    return numberOfTrades;
}

Order* Trade::getHeadOrder() const{
    return headOrder;
}

Order* Trade::getTailOrder() const{
    return tailOrder;
}

Trade* Trade::getParentTrade() const{
    return parentTrade;
}

Trade* Trade::getLeftChildTrade() const{
    return leftChildTrade;
}

Trade* Trade::getRightChildTrade() const{
    return rightChildTrade;
}

void Trade::setParentTrade(Trade* newParentTrade){
    parentTrade = newParentTrade;
}

void Trade::setLeftChildTrade(Trade* newLeftChildTrade){
    leftChildTrade = newLeftChildTrade;
}

void Trade::setRightChildTrade(Trade* newRightChildTrade){
    rightChildTrade = newRightChildTrade;
}

void Trade::addOrder(Order* order){
    if (headOrder == nullptr)
        headOrder = tailOrder = order;
    else{   // _order is the new tailOrder
        tailOrder->nextOrder = order;
        order->previousOrder = tailOrder;
        order->nextOrder = nullptr;
    }
    ++numberOfTrades;
    totalShares += order->getShares();
    order->parentTrade = this;
}

