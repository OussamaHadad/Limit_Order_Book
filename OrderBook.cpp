#include <iostream>

#include "Order.h"
#include "Limit.h"
#include "OrderBook.h"


OrderBook::OrderBook():
    bidTree(nullptr), highestBid(nullptr), askTree(nullptr), lowestAsk(nullptr), 
    stopBidTree(nullptr), lowestStopBid(nullptr), stopAskTree(nullptr), highestStopAsk(nullptr)
    {}

OrderBook::~OrderBook(){
    // When deleting the order book, we should go through each order (limit or stop) and each limit in the corresponding map and delete it
    
    for (auto& [orderId, order] : orderMap)
        delete order;

    for (auto& [stopPrice, stopLevel] : stopMap)
        delete stopLevel;

    for (auto& [limitBidPrice, limitBid] : limitBidMap)
        delete limitBid;

    for (auto& [limitAskPrice, limitAsk] : limitAskMap)
        delete limitAsk;
}


// Getters
Limit* OrderBook::getBidTree() const{
    return bidTree;
}

Limit* OrderBook::getAskTree() const{
    return askTree;
}

Limit* OrderBook::getLowestAsk() const{
    return lowestAsk;
}

Limit* OrderBook::getHighestBid() const{
    return highestBid;
}

Limit* OrderBook::getStopBidTree() const{
    return stopBidTree;
}

Limit* OrderBook::getStopAskTree() const{
    return stopAskTree;
}

Limit* OrderBook::getLowestStopBid() const{
    return lowestStopBid;
}

Limit* OrderBook::getHighestStopAsk() const{
    return highestStopAsk;
}


// Setters
void OrderBook::setBidTree(Limit* newBidTree){
    bidTree = newBidTree;
}

void OrderBook::setAskTree(Limit* newAskTree){
    askTree = newAskTree;
}

void OrderBook::setStopBidTree(Limit* newStopBidTree){
    stopBidTree = newStopBidTree;
}

void OrderBook::setStopAskTree(Limit* newStopAskTree){
    stopAskTree = newStopAskTree;
}


void OrderBook::stopOrderToLimitOrder(Order* Order, Side side){
    ;
}

// Execute orders method
void OrderBook::executeStopOrders(Side side){
    ;
}


// Limit tree's methods
void OrderBook::addLimit(int limitPrice, Side side){
    ;
}

void OrderBook::deleteLimit(Limit* limit){
    ;
}

void OrderBook::deleteFromLimitMaps(int limitPrice, Side side){
    ;
}


// Stop tree's methods
Limit* OrderBook::insertNewStop(Limit* root, Limit* newStop, Limit* parentStop){
    /* Recursive function used to insert a new stop level in the Bid or Ask stop AVL tree
    Returns the root of the Stop tree (or sub-tree for recursive calls) newStop is inserted in. */
    if (root == nullptr){ // First stop level to insert in this stop tree
        newStop->setParentLimit(parentStop); 
        return newStop;
    }
    else if (newStop->getLimitPrice() < root->getLimitPrice()){ // then move to the left subtree
        root->setLeftChildLimit(insertNewStop(root->getLeftChildLimit(), newStop, root));
        root = balanceTree(root, limitORstop::stop);
    }
    else if (newStop->getLimitPrice() > root->getLimitPrice()){ // then move to the right subtree
        root->setRightChildLimit(insertNewStop(root->getRightChildLimit(), newStop, root));
        root = balanceTree(root, limitORstop::stop);
    }
    return root;
}

void OrderBook::addStopLevel(int stopPrice, Side side){
    auto* stopTree = (side == Side::Bid) ? stopBidTree : stopAskTree;
    auto* bookEdge = (side == Side::Bid) ? lowestStopBid : highestStopAsk;

    Limit* newStop = new Limit(stopPrice, side);
    stopMap[stopPrice] = newStop;

    if (stopTree == nullptr){ // i.e: There is no stop level in this tree yet 
        stopTree = newStop;
        bookEdge = newStop;
    }
    else{ // There are stop levels in this tree, and this stopPrice should be inserted in the corresponding tree
        Limit* unused = insertNewStop(stopTree, newStop);  // Insert the new stop in its tree
         
        if (side == Side::Bid){ // Then update the book edge
            if (stopPrice < lowestStopBid->getLimitPrice())
                lowestStopBid = newStop;
        }
        else{
            if (stopPrice > highestStopAsk->getLimitPrice())
                highestStopAsk = newStop;
        }
    }
}

void OrderBook::deleteStopLevel(Limit* stopLevel){
    ;
}



// Limit price order methods
void OrderBook::addLimitOrder(int orderId, Side side, int limitPrice, int shares){
    ;
}

void OrderBook::cancelLimitOrder(int orderId){
    // Cancel order, Delete limit level if empty, then Delete order from orderMap and deallocate memory 
    Order* order = orderMap[orderId];
    
    order->cancelOrder();
    
    if (order->getParentLimit()->getNumberOfOrders() == 0)
        deleteLimit(order->getParentLimit());
    
    orderMap.erase(orderId);
    delete order; 
}

void OrderBook::modifyLimitOrder(int orderId, int newShares, int newLimitPrice){
    Order* order = orderMap[orderId];
    Limit* parentLimit = order->getParentLimit();

    order->cancelOrder();
    if (parentLimit->getNumberOfOrders() == 0)
        deleteLimit(parentLimit);

    order->amendOrder(newShares, newLimitPrice);

    Side orderSide = order->getOrderSide();
    auto limitMap = (orderSide == Side::Bid) ? limitBidMap : limitAskMap;

    if (limitMap.find(newLimitPrice) == limitMap.end()) // New limit price
        addLimit(newLimitPrice, orderSide);

    addLimitOrder(orderId, orderSide, newLimitPrice, newShares);
}


// Stop order methods
int OrderBook::addStopOrder(int orderId, Side side, int stopPrice, int shares){ 
    ;
}

void OrderBook::cancelStopOrder(int orderId){
    // Cancel order, Delete limit level if empty, then Delete order from orderMap and deallocate memory 
    Order* order = orderMap[orderId];
    
    order->cancelOrder();
    
    if (order->getParentLimit()->getNumberOfOrders() == 0)
        deleteStopLevel(order->getParentLimit());
    
    orderMap.erase(orderId);
    delete order; 
}

void OrderBook::modifyStopOrder(int orderId, int newShares, int newstopPrice){
    Order* order = orderMap[orderId];
    Limit* parentLimit = order->getParentLimit();
    Side orderSide = order->getOrderSide();

    order->cancelOrder();
    if (parentLimit->getNumberOfOrders() == 0)
        deleteStopLevel(parentLimit);

    order->amendOrder(newShares, newstopPrice);

    if (stopMap.find(newstopPrice) == stopMap.end()) // New Stop price
        addStopLevel(newstopPrice, orderSide);

    addStopOrder(orderId, orderSide, newstopPrice, newShares);
}


void OrderBook::executeMarketOrder(Side side, int shares){
    Limit* bookEdge = (side == Side::Bid) ? lowestAsk : highestBid;

    while (bookEdge != nullptr && shares > 0){
        Order* headOrder = bookEdge->getHeadOrder(); // The first order to be executed from the bookEdge level
        int tradedShares = std::min(headOrder->getOrderShares(), shares);
        
        headOrder->executeOrder(tradedShares); // Head Order is executed
        shares -= tradedShares; // The remaining number of shares from the market order

        if (headOrder->getOrderShares() == 0){ // headOrder was completely executed
            orderMap.erase(headOrder->getOrderId());
            delete headOrder;

            if (bookEdge->getNumberOfOrders() == 0)
                deleteLimit(bookEdge);
        }
    }
}

void OrderBook::addMarketOrder(int orderId, Side side, int shares){
    // First, we execute the market order
    executeMarketOrder(side, shares);
    // Then we see if the changes that happened to the order book can trigger a stop order
    executeStopOrders(side);
}