#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <unordered_map>

#include "enums.h"

class Limit;
class Order;

class OrderBook {
private:
    // Limit Orders
    Limit* bidTree; // Bid == Buy
    Limit* highestBid;
    Limit* askTree; // Ask == Sell
    Limit* lowestAsk;
    
    /* How do Stop Orders work: a Stop Order is activated when its stop price is exceeded for a Bid or subceeded for an Ask
        Stop Bid Order -> if the lowest ask price goes above this stop order's limit price, then this stop order is executed
        Stop Ask Order -> if the highest bid price goes below ...   
    */
    Limit* stopBidTree;
    Limit* lowestStopBid; // triggered at a lower limit price from the Ask side, hence 1st to be executed
    Limit* stopAskTree;
    Limit* highestStopAsk; // triggered at a higher limit price from the Bid side, hence 1st to be executed

    std::unordered_map<int, Order*> orderMap;
    std::unordered_map<int, Limit*> limitBidMap;
    std::unordered_map<int, Limit*> limitAskMap;
    std::unordered_map<int, Limit*> stopMap;

    // Limit & Stop trees' methods
    void addLimit(int limitPrice, OrderSide orderSide); // Add a new limit level
    void addStopLevel(int stopPrice, OrderSide orderSide); // Add a new stop price

    // Limit and Stop trees' shared methods
    Limit* insertNewLevel(Limit* root, Limit* newLevel, Limit* parentLevel, OrderCategory  orderCategory);
    void deleteLevel(Limit* level, OrderCategory  orderCategory);

    // Auxiliary methods
    void stopOrderToLimitOrder(Order* Order, OrderSide orderSide); 
    void executeStopOrders(OrderSide orderSide); // Used for limit & stop orders

    // AVL Tree methods; Note: OrderBook is an AVL Tree
    int limitHeightDifference(Limit* limit) const;
    Limit* balanceTree(Limit* parentLimit, OrderCategory  orderCategory);
    // Rotations happen at the node where the unbalance happens
    Limit* rRotate(Limit* parentLimit, OrderCategory  orderCategory); // for a "right"-right-heavy tree
    Limit* lRotate(Limit* parentLimit, OrderCategory  orderCategory);
    Limit* lrRotate(Limit* parentLimit, OrderCategory  orderCategory);
    Limit* rlRotate(Limit* parentLimit, OrderCategory  orderCategory);

    void updateTreeRoot(Limit* level, OrderCategory  orderCategory);
    void updateBookEdge(Limit* level, OrderCategory  orderCategory);

    void traverseAndDisplay(Limit* root, bool isBid, bool isStop) const;
    void printLimitOrders(Limit* limit, bool isStop) const;

public:
    OrderBook();
    ~OrderBook();

    // Getters
    inline Limit* getBidTree() const { return bidTree; }
    inline Limit* getAskTree() const { return askTree; }
    inline Limit* getLowestAsk() const { return lowestAsk; }
    inline Limit* getHighestBid() const { return highestBid; }
    inline Limit* getStopBidTree() const { return stopBidTree; }
    inline Limit* getStopAskTree() const { return stopAskTree; }
    inline Limit* getLowestStopBid() const { return lowestStopBid; }
    inline Limit* getHighestStopAsk() const { return highestStopAsk; }
    inline std::unordered_map<int, Order*> getOrderMap() const { return orderMap; }

    // Setters
    inline void setBidTree(Limit* newBidTree) { bidTree = newBidTree; }
    inline void setAskTree(Limit* newAskTree) { askTree = newAskTree; }
    inline void setStopBidTree(Limit* newStopBidTree) { stopBidTree = newStopBidTree; }
    inline void setStopAskTree(Limit* newStopAskTree) { stopAskTree = newStopAskTree; }

    // Limit order methods
    void addLimitOrder(int orderId, OrderSide orderSide, int limitPrice, int shares); // Note: For any order type, OrderSide is needed only when adding an order
    void cancelLimitOrder(int orderId);
    void modifyLimitOrder(int orderId, int newShares, int newLimitPrice);

    // Stop order methods
    void addStopOrder(int orderId, OrderSide orderSide, int stopPrice, int shares); // Once stopPrice is reached the order is executed with the market price
    void cancelStopOrder(int orderId);
    void modifyStopOrder(int orderId, int newShares, int newstopPrice);

    // Market orders are executed immediately after adding them, hence it's not possible to cancel or modify them
    // We assume a market order is filled completely or partially, and then removed
    void executeMarketOrder(OrderSide orderSide, int& shares);
    void addMarketOrder(OrderSide orderSide, int shares);

    // AVL Tree methods
    int getLimitHeight(Limit* limit) const;

    void displayAllOrders(bool includeStopOrders = false) const;
};

#endif