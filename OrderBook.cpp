#include <iostream>

#include "OrderBook.h"


OrderBook::OrderBook(){
    ;
}

OrderBook::~OrderBook(){
    ;
}

// Getters
Trade* OrderBook::getBidTree() const{
    return bidTree;
}

Trade* OrderBook::getAskTree() const{
    return askTree;
}

Trade* OrderBook::getLowestAsk() const{
    return lowestAsk;
}

Trade* OrderBook::getHighestBid() const{
    return highestBid;
}

Trade* OrderBook::getStopBidTree() const{
    return stopBidTree;
}

Trade* OrderBook::getStopAskTree() const{
    return stopAskTree;
}

Trade* OrderBook::getLowestStopAsk() const{
    return lowestStopAsk;
}

Trade* OrderBook::getHighestStopBid() const{
    return highestStopBid;
}



// Limit price order methods
void addLimitOrder(int orderId, Side side, int limitPrice, int shares){
    ;
}

void cancelLimitOrder(int orderId, Side side){
    ;
}

void modifyLimitOrder(int orderId, Side side, int newLimitPrice, int newShares){
    ;
}


// Market orders are executed immediately after adding them, hence it's not possible to modify them
void addMarketOrder(int orderId, Side side, int shares){
    ;
}


// Stop order methods
void addLimitOrder(int orderId, Side side, int stopPrice, int shares){
    ;
}

void cancelStopOrder(int orderId, Side side){
    ;
}

void modifyStopOrder(int orderId, Side side, int newstopPrice, int newShares){
    ;
}

