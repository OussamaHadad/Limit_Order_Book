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
        Stop Ask Order -> if the highest bid price goes below ...   */
    Limit* stopBidTree;
    Limit* lowestStopBid; // triggered at a lower limit price from the Ask side, hence 1st to be executed
    Limit* stopAskTree;
    Limit* highestStopAsk; // triggered at a higher limit price from the Bid side, hence 1st to be executed

    // Orders are added to orderMap, and the Limits are added to either Limit Bid or Limit Ask map
    std::unordered_map<int, Order*> orderMap;
    std::unordered_map<int, Limit*> limitBidMap;
    std::unordered_map<int, Limit*> limitAskMap;
    std::unordered_map<int, Limit*> stopMap;

    // Limit tree's methods
    void addLimit(int limitPrice, Side side); // Add a new limit level; Used when adding a limit order with a new limit price

    // Stop tree's methods
    void addStopLevel(int stopPrice, Side side); // Add a new stop price

    // Auxiliary methods used inside other methods
    void stopOrderToLimitOrder(Order* Order, Side side); 
    void executeStopOrders(Side side); // Used for both limit and stop orders

    // Limit and Stop trees' shared methods
    Limit* insertNewLevel(Limit* root, Limit* newLevel, Limit* parentLevel, limitORstop limit_or_stop);
    void deleteLevel(Limit* level, limitORstop limit_or_stop);

    // AVL Tree methods; Note: OrderBook is an AVL Tree
    int limitHeightDifference(Limit* limit) const;
    Limit* balanceTree(Limit* parentLimit, limitORstop limit_or_stop);
    // Rotations happen at the node where the unbalance happens
    Limit* rRotate(Limit* parentLimit, limitORstop limit_or_stop); // for a "right"-right-heavy tree
    Limit* lRotate(Limit* parentLimit, limitORstop limit_or_stop);
    Limit* lrRotate(Limit* parentLimit, limitORstop limit_or_stop);
    Limit* rlRotate(Limit* parentLimit, limitORstop limit_or_stop);

    void updateTreeRoot(Limit* level, limitORstop limit_or_stop);
    void updateBookEdge(Limit* level, limitORstop limit_or_stop);

public:
    OrderBook();
    ~OrderBook();

    // Getters
    Limit* getBidTree() const;
    Limit* getAskTree() const;
    Limit* getLowestAsk() const;
    Limit* getHighestBid() const;
    Limit* getStopBidTree() const;
    Limit* getStopAskTree() const;
    Limit* getLowestStopBid() const;
    Limit* getHighestStopAsk() const;

    // Setters
    void setBidTree(Limit* newBidTree);
    void setAskTree(Limit* newAskTree);
    void setStopBidTree(Limit* newStopBidTree);
    void setStopAskTree(Limit* newStopAskTree);

    // Limit order methods
    void addLimitOrder(int orderId, Side side, int limitPrice, int shares); // Note: For any order type, Side is needed only when adding an order
    void cancelLimitOrder(int orderId);
    void modifyLimitOrder(int orderId, int newShares, int newLimitPrice);

    // Stop order methods
    void addStopOrder(int orderId, Side side, int stopPrice, int shares); // Once stopPrice is reached the order is executed with the market price
    void cancelStopOrder(int orderId);
    void modifyStopOrder(int orderId, int newShares, int newstopPrice);

    // Market orders are executed immediately after adding them, hence it's not possible to cancel or modify them
    // We assume a market order is filled completely or partially, and then removed
    void executeMarketOrder(Side side, int& shares);
    void addMarketOrder(Side side, int shares);

    // AVL Tree methods
    int getLimitHeight(Limit* limit) const;
};

#endif