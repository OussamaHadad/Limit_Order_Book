#include <iostream>

#include "Order.h"


class Order;    // We declare Order class before defining its attributes and the prototypes of its methods in Order.h

class Trade {   // a trade is a set of orders of the same limit price

    friend class Order; // Order class is a friend of Trade class, thus it can access the private attributes of Trade class

private:
    int limitPrice; // this limit price is common between all orders of the same trade
    Side side; // side of the trade
    int numberOfTrades; // number of orders in the trade
    int totalShares; // total number of shares in the trade (sum of shares of all orders)
    Order* headOrder;   // head of the linked list of orders of the trade; The first order to be executed within the limit orders of the trade
    Order* tailOrder;
    Trade* parentTrade;
    Trade* leftChildTrade;
    Trade* rightChildTrade;

public:
    Trade(int _limitPrice, Side side, int _numberOfTrades, int _totalShares);
    ~Trade();

    void Trade::showTrade() const;

    int getLimitPrice() const;
    int getNumberOfTrades() const;
    int getTotalShares() const;
    Order* getHeadOrder() const;
    Order* getTailOrder() const;
    Trade* getParentTrade() const;
    Trade* getLeftChildTrade() const;
    Trade* getRightChildTrade() const;

    void setParentTrade(Trade* parent);
    void setLeftChildTrade(Trade* leftChild);
    void setRightChildTrade(Trade* rightChild);

    void addOrder(Order* _order);   // add an order to the trade

};