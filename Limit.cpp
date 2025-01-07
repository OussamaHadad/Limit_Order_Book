#include <iostream>

#include "Limit.h"
#include "Order.h"

Limit::Limit(int _limitPrice, Side _side, int _numberOfOrders, int _totalShares):
    limitPrice(_limitPrice), side(_side), numberOfOrders(_numberOfOrders), totalShares(_totalShares), 
    headOrder(nullptr), tailOrder(nullptr), parentLimit(nullptr), leftChildLimit(nullptr), rightChildLimit(nullptr) 
    {}

Limit::~Limit(){
    ;
}

void Limit::showLimit() const{
    Order* pseudoHeadOrder = headOrder;
    std::cout << "Following are the IDs of Orders part of this limit level" << std::endl;
    while (pseudoHeadOrder != nullptr){
        std::cout << "Order ID: " << pseudoHeadOrder->getOrderId() << std::endl;
        pseudoHeadOrder = pseudoHeadOrder->getNextOrder();
    }
}

int Limit::getLimitPrice() const{
    return limitPrice;
}

int Limit::getNumberOfOrders() const{
    return numberOfOrders;
}

int Limit::getTotalShares() const{
    return numberOfOrders;
}

Order* Limit::getHeadOrder() const{
    return headOrder;
}

Order* Limit::getTailOrder() const{
    return tailOrder;
}

Limit* Limit::getParentLimit() const{
    return parentLimit;
}

Limit* Limit::getLeftChildLimit() const{
    return leftChildLimit;
}

Limit* Limit::getRightChildLimit() const{
    return rightChildLimit;
}

void Limit::setParentLimit(Limit* newParentLimit){
    parentLimit = newParentLimit;
}

void Limit::setLeftChildLimit(Limit* newLeftChildLimit){
    leftChildLimit = newLeftChildLimit;
}

void Limit::setRightChildLimit(Limit* newRightChildLimit){
    rightChildLimit = newRightChildLimit;
}

void Limit::addOrder(Order* order){
    if (headOrder == nullptr)
        headOrder = tailOrder = order;
    else{   // _order is the new tailOrder
        tailOrder->nextOrder = order;
        order->previousOrder = tailOrder;
        order->nextOrder = nullptr;
    }
    ++numberOfOrders;
    totalShares += order->getOrderShares();
    order->parentLimit = this;
}

