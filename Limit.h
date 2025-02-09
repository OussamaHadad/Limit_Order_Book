#ifndef LIMIT_H
#define LIMIT_H

#include "enums.h"

class Order;

class Limit {   // a limit is a set of orders of the same limit price
private:
    friend class Order; // Order class is a friend of Trade class, thus it can access the private attributes of Trade class

    int limitPrice; // Price level for this limit
    OrderSide orderSide; // Bid or Ask side
    int numberOfOrders; // number of orders in the limit
    int totalShares; // total number of shares in the limit (sum of shares of all orders)
    
    Order* headOrder;   // head of the linked list of orders of the limit; The first order to be executed within the limit orders of the limit
    Order* tailOrder;
    
    Limit* parentLimit;
    Limit* leftChildLimit;
    Limit* rightChildLimit;

public:
    Limit(int _limitPrice, OrderSide _orderSide);
    ~Limit();

    void showLimit() const;

    // Getters
    inline int getLimitPrice() const { return limitPrice; }
    inline OrderSide getOrderSide() const { return orderSide; }
    inline int getNumberOfOrders() const { return numberOfOrders; }
    inline int getTotalShares() const { return totalShares; }
    inline Order* getHeadOrder() const { return headOrder; }
    inline Order* getTailOrder() const { return tailOrder; }
    inline Limit* getParentLimit() const { return parentLimit; }
    inline Limit* getLeftChildLimit() const { return leftChildLimit; }
    inline Limit* getRightChildLimit() const { return rightChildLimit; }

    // Setters
    inline void setParentLimit(Limit* parent) { parentLimit = parent; }
    inline void setLeftChildLimit(Limit* leftChild) { leftChildLimit = leftChild; }
    inline void setRightChildLimit(Limit* rightChild) { rightChildLimit = rightChild; }
    inline void setHeadOrder(Order* newHeadOrder) { headOrder = newHeadOrder; }
    inline void setTailOrder(Order* newTailOrder) { tailOrder = newTailOrder; }
    
    void addOrder(Order* order);   // Add an order to this limit level
    void removeOrder(Order* order); // Remove an order from this limit level
};

#endif