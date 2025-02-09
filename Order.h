#ifndef ORDER_H
#define ORDER_H

#include "enums.h"


class Limit;

class Order {
private:
    friend class Limit; // Limit class is a friend of Order class, thus it can access the private attributes of Order class

    // Following are the primary attributes of an order
    int idNumber; // Unique identifier for the order
    OrderSide orderSide; // Bid or Ask side of the order
    int orderShares; // Number of shares in the order
    int limitPrice; // Price level for limit orders
    OrderType orderType; // Type of order (Limit, Market, Stop)
    TimeInForce tif; // Time-in-force for the order (GTC, DAY, IOC, FOK)
    std::time_t submissionTime; // Timestamp when the order was submitted

    Limit* parentLimit;    // The limit level to which this order belongs
    Order* previousOrder;  // Previous order in the doubly linked list
    Order* nextOrder;      // Next order in the doubly linked list
    //      nextOrder  -> Order ->  previousOrder   ; next is next to be executed   &   previous is previously executed

public:
    Order(int _idNumber, OrderSide _orderSide, int _orderShares, int _limitPrice, OrderType _type = OrderType::LimitOrder, TimeInForce _tif = TimeInForce::GTC);

    // Getters
    inline int getOrderId() const { return idNumber; }
    inline OrderSide getOrderSide() const { return orderSide; }
    inline int getOrderShares() const { return orderShares; }
    inline int getLimitPrice() const { return limitPrice; }
    inline Limit* getParentLimit() const { return parentLimit; }
    inline Order* getNextOrder() const { return nextOrder; }
    inline Order* getPreviousOrder() const { return previousOrder; }
    inline OrderType getOrderType() const { return orderType; }
    inline TimeInForce getTIF() const { return tif; }
    inline std::time_t getSubmissionTime() const { return submissionTime; }

    // Setters
    inline void setPreviousOrder(Order* newPreviousOrder) { previousOrder = newPreviousOrder; }
    inline void setNextOrder(Order* newNextOrder) { nextOrder = newNextOrder; }
    inline void setParentLimit(Limit* newParentLimit) { parentLimit = newParentLimit; }

    void displayOrder() const; // Show order details

    void amendOrder(int newShares, int newLimitPrice); // Modify order
    void cancelOrder(); // Cancel order
    void executeOrder(int tradedShares); // Execute order
};

#endif