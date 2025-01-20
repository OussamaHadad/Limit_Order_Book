#ifndef LIMIT_H
#define LIMIT_H


class Order;    // We declare Order class before defining its attributes and the prototypes of its methods in Order.h

class Limit {   // a limit is a set of orders of the same limit price
private:
    friend class Order; // Order class is a friend of Trade class, thus it can access the private attributes of Trade class

    int limitPrice; // this limit price is common between all orders of the same limit level
    Side side; // side of the trade, bid or ask
    int numberOfOrders; // number of orders in the limit
    int totalShares; // total number of shares in the limit (sum of shares of all orders)
    
    Order* headOrder;   // head of the linked list of orders of the limit; The first order to be executed within the limit orders of the limit
    Order* tailOrder;
    
    Limit* parentLimit;
    Limit* leftChildLimit;
    Limit* rightChildLimit;

public:
    Limit(int _limitPrice, Side _side, int _numberOfOrders = 0, int _totalShares = 0);
    ~Limit();

    void Limit::showLimit() const;

    // Getters
    int getLimitPrice() const;
    Side getSide() const;
    int getNumberOfOrders() const;
    int getTotalShares() const;
    Order* getHeadOrder() const;
    Order* getTailOrder() const;
    Limit* getParentLimit() const;
    Limit* getLeftChildLimit() const;
    Limit* getRightChildLimit() const;

    // Setters
    void setParentLimit(Limit* parent);
    void setLeftChildLimit(Limit* leftChild);
    void setRightChildLimit(Limit* rightChild);

    void addOrder(Order* order);   // add an order of limit price limitPrice to this limit level
};

#endif