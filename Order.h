#include <iostream>
#include <ctime>

#include "enums.h"


class Limit;    // We declare Limit class before defining its attributes and the prototypes of its methods in Limit.h

class Order {
    
    friend class Limit; // Limit class is a friend of Order class, thus it can access the private attributes of Order class

private:
    // Following are the primary attributes of an order
    int idNumber;
    Side side;
    int orderShares; // number of order shares
    int limitPrice;
    OrderType type;
    TIF tif;
    std::time_t submissionTime;

    // The following attributes are introduced to improve the time complexity of recurrent manipulations
    Limit* parentLimit; // The limit (or limit level) to which the order belongs
    Order* previousOrder;   // The previous order in the doubly linked list of orders of the same limit level
    Order* nextOrder;   // The next order in the doubly linked list of orders of the same limit level

public:
    Order(int _idNumber, Side _side, int _orderShares, int _limitPrice, OrderType _type = OrderType::LimitOrder, TIF _tif = TIF::GTC);

    // Getters
    int getOrderId() const;
    Side getOrderSide() const;
    int getOrderShares() const;
    int getLimitPrice() const;
    OrderType getOrderType() const;
    TIF getTIF() const;
    Limit* getParentLimit() const;
    Order* getNextOrder() const;

    void showOrder() const;

    void amendOrder(int newShares, int newLimitPrice);  // Modify order by changing the number of shares, or the limit price, or both
    void cancelOrder();
    void executeOrder(int tradedShares);
};



