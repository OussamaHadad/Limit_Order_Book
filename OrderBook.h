#include <iostream>
#include <unordered_map>

#include "Order.h"

class Trade;
class Order;

class OrderBook {

private:
    // Limit Orders
    Trade* bidTree; // Bid == Buy
    Trade* askTree; // Ask == Sell
    Trade* lowestAsk;
    Trade* highestBid;

    // Stop Orders are activated, whether buy or sell, when a certain price is reached
    Trade* stopBidTree;
    Trade* stopAskTree;
    Trade* lowestStopAsk;
    Trade* highestStopBid;

    std::unordered_map<int, Order*> orderMap;
    std::unordered_map<int, Trade*> limitBidMap;
    std::unordered_map<int, Trade*> limitAskMap;

    // Limit Trades' methods
    void addLimitTrade(int limitPrice, Side side);
    void deleteLimitTrade(Trade* Trade);

    // Stop Trades' methods
    void addStopTrade(int stopPrice, Side side);
    void deleteStopTrade(Trade* Trade);

public:
    OrderBook();
    ~OrderBook();

    // Getters
    Trade* getBidTree() const;
    Trade* getAskTree() const;
    Trade* getLowestAsk() const;
    Trade* getHighestBid() const;
    Trade* getStopBidTree() const;
    Trade* getStopAskTree() const;
    Trade* getLowestStopAsk() const;
    Trade* getHighestStopBid() const;

    // Limit price order methods
    void addLimitOrder(int orderId, Side side, int limitPrice, int shares);
    void cancelLimitOrder(int orderId, Side side);
    void modifyLimitOrder(int orderId, Side side, int newLimitPrice, int newShares);

    // Market orders are executed immediately after adding them, hence it's not possible to modify them
    void addMarketOrder(int orderId, Side side, int shares);

    // Stop order methods
    void addLimitOrder(int orderId, Side side, int stopPrice, int shares);
    void cancelStopOrder(int orderId, Side side);
    void modifyStopOrder(int orderId, Side side, int newstopPrice, int newShares);

};