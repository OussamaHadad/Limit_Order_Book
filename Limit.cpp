#include <iostream>

#include "Limit.h"
#include "Order.h"

Limit::Limit(int _limitPrice, OrderSide _orderSide) : 
    limitPrice(_limitPrice), orderSide(_orderSide), 
    numberOfOrders(0), totalShares(0),  // number of orders and total shares initialized to 0
    headOrder(nullptr), tailOrder(nullptr),
    parentLimit(nullptr), leftChildLimit(nullptr), rightChildLimit(nullptr) 
{}

Limit::~Limit() {   // Destroy all orders of this limit
    Order* current = headOrder;
    while (current) {
        Order* next = current->getNextOrder();
        delete current;
        current = next;
    }
}

void Limit::showLimit() const {
    Order* current = headOrder;
    std::cout << "Following are the IDs of Orders part of this limit level" << std::endl;
    while (current) {
        std::cout << "Order ID: " << current->getOrderId() << std::endl;
        current = current->getNextOrder();
    }
}

void Limit::addOrder(Order* order) {
    if (!order)
        throw std::invalid_argument("Order cannot be null");

    if (!headOrder)
        headOrder = tailOrder = order;
    else {
        tailOrder->setNextOrder(order);
        order->setPreviousOrder(tailOrder);
        order->setNextOrder(nullptr);
        tailOrder = order;
    }
    
    ++numberOfOrders;
    totalShares += order->getOrderShares();
    order->setParentLimit(this);
}

void Limit::removeOrder(Order* order) {
    if (!order || !headOrder) 
        return;

    if (order == headOrder) {
        headOrder = order->getNextOrder();
        if (headOrder) 
            headOrder->setPreviousOrder(nullptr);
    } 
    else if (order == tailOrder) {
        tailOrder = order->getPreviousOrder();
        if (tailOrder) 
            tailOrder->setNextOrder(nullptr);
    } 
    else {
        Order* prev = order->getPreviousOrder();
        Order* next = order->getNextOrder();
        if (prev) 
            prev->setNextOrder(next);
        if (next) 
            next->setPreviousOrder(prev);
    }

    --numberOfOrders;
    totalShares -= order->getOrderShares();
    delete order;
}
