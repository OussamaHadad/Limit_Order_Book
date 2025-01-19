#include <iostream>
#include <assert.h>
#include <algorithm> 

#include "Order.h"
#include "Limit.h"
#include "OrderBook.h"


OrderBook::OrderBook():
    bidTree(nullptr), highestBid(nullptr), askTree(nullptr), lowestAsk(nullptr), 
    stopBidTree(nullptr), lowestStopBid(nullptr), stopAskTree(nullptr), highestStopAsk(nullptr)
    {}

OrderBook::~OrderBook(){
    // When deleting the order book, we should go through each order (limit or stop) and each limit in the corresponding map and delete it
    
    for (auto& pair : orderMap) // pair.first = orderId && pair.second = order
        delete pair.second;

    for (auto& pair : limitBidMap) // pair.first = limitBidPrice && pair.second = limitBid
        delete pair.second;

    for (auto& pair : limitAskMap) // pair.first = limitAskPrice && pair.second = limitAsk
        delete pair.second;

    for (auto& pair : stopMap) // pair.first = stopPrice && pair.second = stopLevel
        delete pair.second;
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


void OrderBook::stopOrderToLimitOrder(Order* order, Side side){
    /* Turn a stop limit order into a limit order.
    First, we see if the stop limit order can be executed, and only then we turn the remaining shares into a limit order */
    
    Limit* bookEdge = (side == Side::Bid) ? lowestStopBid : highestStopAsk;

    int tradedShares = std::min(order->getOrderShares(), bookEdge->getTotalShares());
    order->executeOrder(tradedShares);

    if (tradedShares == bookEdge->getTotalShares()) // All shares from the current book edge were traded, hence delete the book edge limit
        deleteStopLevel(bookEdge);

    if (order->getOrderShares() != 0){
        auto limitMap = (side == Side::Bid) ? limitBidMap : limitAskMap;

        if (limitMap.find(order->getLimitPrice()) == limitMap.end())
            addLimit(order->getLimitPrice(), side);

        limitMap[order->getLimitPrice()]->addOrder(order);
    }
    
}

// Execute orders method
void OrderBook::executeStopOrders(Side side){
    /*  This function goes through the Stop orders map and executes the triggered stop orders   */
    if (side == Side::Bid){
        while (lowestStopBid != nullptr && (lowestAsk == nullptr || lowestStopBid->getLimitPrice() <= lowestAsk->getLimitPrice())){
            
            Order* headOrder = lowestStopBid->getHeadOrder();

            if (headOrder->getLimitPrice() != 0){ // i.e: no order is left at this limit level as only the predefined empty orders have 0 as a price
                int tradedShares = std::min(headOrder->getOrderShares(), lowestStopBid->getTotalShares());

                headOrder->executeOrder(tradedShares);
                
                if (tradedShares == lowestStopBid->getTotalShares())
                    deleteStopLevel(lowestStopBid);
                
                if (tradedShares == headOrder->getOrderShares()){
                    orderMap.erase(headOrder->getOrderId());
                    delete headOrder;
                }

                //executeMarketOrder(side, shares);
            }
            else
                stopOrderToLimitOrder(headOrder, side);
        }
    }
    else{ // side == Side::Ask
        while (highestStopAsk != nullptr && highestBid != nullptr && highestStopAsk->getLimitPrice() >= highestBid->getLimitPrice()){
            
            Order* headOrder = highestStopAsk->getHeadOrder();

            if (headOrder->getOrderShares() != 0){ // i.e: no order is left at this limit level as only the predefined empty orders have 0 as a price
                int tradedShares = std::min(headOrder->getOrderShares(), highestStopAsk->getTotalShares());

                headOrder->executeOrder(tradedShares);
                
                if (tradedShares == highestStopAsk->getTotalShares())
                    deleteStopLevel(highestStopAsk);

                if (tradedShares == headOrder->getOrderShares()){
                    orderMap.erase(headOrder->getOrderId());
                    delete headOrder;
                }

                //executeMarketOrder(side, shares);
            }
            else
                stopOrderToLimitOrder(headOrder, side);
        }
    }
}


// Limit tree's methods

Limit* OrderBook::insertNewLevel(Limit* root, Limit* newLevel, Limit* parentLevel, limitORstop limit_or_stop){
    /* Recursive function used to insert a new stop level in the Bid or Ask stop AVL tree
    Returns the root of the tree (or sub-tree for recursive calls) where newLevel is inserted. 
    Note: newLevel is either a new Stop or a new Limit */
    if (root == nullptr){ // First stop level to insert in this stop tree
        newLevel->setParentLimit(parentLevel); 
        return newLevel;
    }
    else if (newLevel->getLimitPrice() < root->getLimitPrice()){ // then move to the left subtree
        root->setLeftChildLimit(insertNewLevel(root->getLeftChildLimit(), newLevel, root, limit_or_stop));
        root = balanceTree(root, limit_or_stop);
    }
    else if (newLevel->getLimitPrice() > root->getLimitPrice()){ // then move to the right subtree
        root->setRightChildLimit(insertNewLevel(root->getRightChildLimit(), newLevel, root, limit_or_stop));
        root = balanceTree(root, limit_or_stop);
    }
    return root;
}

void OrderBook::addLimit(int limitPrice, Side side){
    // When adding a new limit, in addition to adding it to the new limit map and limit tree, we check if it can be the new book edge
    auto& limitMap = (side == Side::Bid) ? limitBidMap : limitAskMap;
    Limit* tree = (side == Side::Bid) ? bidTree : askTree;
    Limit* bookEdge = (side == Side::Bid) ? highestBid : lowestAsk;

    Limit* newLimit = new Limit(limitPrice, side);
    limitMap.emplace(limitPrice, newLimit);

    if (tree == nullptr){
        tree = bookEdge = newLimit;
    }
    else{
        // Check if tree root should be updated, and update it if needed
        Limit* unused = insertNewLevel(tree, newLimit, nullptr, limitORstop::limit);
        // Check if book edge should be updated, and update it if needed
        if (side == Side::Bid){
            if (highestBid->getLimitPrice() < limitPrice)
                highestBid = newLimit;
        }
        else{ // Side::Limit
            if (lowestAsk->getLimitPrice() > limitPrice)
                lowestAsk = newLimit;
        }
    }
}

void OrderBook::deleteLimit(Limit* limit){
    /* When deleting a stop/limit level we make sure to update book edge and root if this limit corresponds to one or both,
    and then we rebalance and update the parent/children of connected nodes if needed */

    updateBookEdge(limit, limitORstop::limit);
    updateBookRoot(limit, limitORstop::limit);

    Limit* parentLimit = limit->getParentLimit();
    int limitPrice = limit->getLimitPrice();

    (limit->getSide() == Side::Bid) ? limitBidMap.erase(limitPrice) : limitAskMap.erase(limitPrice);
    delete limit;

    while (parentLimit != nullptr){
        parentLimit = balanceTree(parentLimit, limitORstop::limit);
        
        if (parentLimit->getParentLimit() != nullptr){

            if (parentLimit->getParentLimit()->getLimitPrice() > limitPrice)
                parentLimit->getParentLimit()->setLeftChildLimit(parentLimit);
            else
                parentLimit->getParentLimit()->setRightChildLimit(parentLimit);
        }

        parentLimit = parentLimit->getParentLimit();
    }
}


// Stop tree's methods

/* Limit* OrderBook::insertNewStop(Limit* root, Limit* newStop, Limit* parentStop){
    // Recursive function used to insert a new stop level in the Bid or Ask stop AVL tree
    //Returns the root of the Stop tree (or sub-tree for recursive calls) newStop is inserted in.
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
} */

void OrderBook::addStopLevel(int stopPrice, Side side){
    auto* stopTree = (side == Side::Bid) ? stopBidTree : stopAskTree;
    auto* bookEdge = (side == Side::Bid) ? lowestStopBid : highestStopAsk;

    Limit* newStop = new Limit(stopPrice, side);
    stopMap.emplace(stopPrice, newStop);

    if (stopTree == nullptr){ // i.e: There is no stop level in this tree yet 
        stopTree = newStop;
        bookEdge = newStop;
    }
    else{ // There are stop levels in this tree, and this stopPrice should be inserted in the corresponding tree
        Limit* unused = insertNewLevel(stopTree, newStop, nullptr, limitORstop::stop);
         
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
    /* When deleting a stop/limit level we make sure to update book edge and root if this limit corresponds to one or both,
    and then we rebalance and update the parent/children of connected nodes if needed */

    updateBookEdge(stopLevel, limitORstop::stop);
    updateBookRoot(stopLevel, limitORstop::stop);

    Limit* parentLimit = stopLevel->getParentLimit();
    int stopPrice = stopLevel->getLimitPrice();

    stopMap.erase(stopPrice);
    delete stopLevel;

    while (parentLimit != nullptr){
        parentLimit = balanceTree(parentLimit, limitORstop::stop);
        
        if (parentLimit->getParentLimit() != nullptr){

            if (parentLimit->getParentLimit()->getLimitPrice() > stopPrice)
                parentLimit->getParentLimit()->setLeftChildLimit(parentLimit);
            else
                parentLimit->getParentLimit()->setRightChildLimit(parentLimit);
        }

        parentLimit = parentLimit->getParentLimit();
    }
}


// Limit order methods
void OrderBook::addLimitOrder(int orderId, Side side, int limitPrice, int shares){
    /* First, we trade the biggest possible number of shares (0 if the order doesn't match the best bid/ask),
    and then, the remaining left are used to make a limit order */
    if (side == Side::Bid){
        while (lowestAsk != nullptr && shares != 0 && limitPrice >= lowestAsk->getLimitPrice()){
            executeMarketOrder(side, shares);
        }
    }   
    else{ // side == Side::Ask
        while (highestBid != nullptr && shares != 0 && limitPrice <= highestBid->getLimitPrice()){
            executeMarketOrder(side, shares);
        }
    }

    if (shares != 0){ // some (or all) shares are left
        Order* newOrder = new Order(orderId, side, shares, limitPrice);
        orderMap.emplace(orderId, newOrder);

        auto& limitMap = (side == Side::Bid) ? limitBidMap : limitAskMap;

        if (limitMap.find(limitPrice) == limitMap.end()){
            addLimit(limitPrice, side);
        }
        
        limitMap[limitPrice]->addOrder(newOrder);
    }
    else{ // all shares were traded, then we can see if some stop orders can be executed
        executeStopOrders(side);
    }
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
    assert(order != nullptr && "Error: This order Id doesn't exist");

    order->cancelOrder();

    Limit* parentLimit = order->getParentLimit();
    if (parentLimit->getNumberOfOrders() == 0)
        deleteLimit(parentLimit);

    order->amendOrder(newShares, newLimitPrice);

    Side side = order->getOrderSide();
    auto& limitMap = (side == Side::Bid) ? limitBidMap : limitAskMap;

    if (limitMap.find(newLimitPrice) == limitMap.end()) // New limit price
        addLimit(newLimitPrice, side);

    limitMap[newLimitPrice]->addOrder(order);
}


// Stop order methods
void OrderBook::addStopOrder(int orderId, Side side, int stopPrice, int shares){
    // Note: If a stop order isn't completely executed, it is conserved with the remaining number of shares
    
    // First, we try to execute the stop order if possible
    if (side == Side::Bid && lowestAsk != nullptr && stopPrice <= lowestAsk->getLimitPrice()){ // the last condition triggers the stop order
        executeMarketOrder(side, shares);
    }
    else if (side == Side::Ask && highestBid != nullptr && stopPrice >= highestBid->getLimitPrice()){ // ...
        executeMarketOrder(side, shares);
    }

    if (shares != 0){ // The remaining shares can be turned into a stop order
        Order* newOrder = new Order(orderId, side, shares, stopPrice);
        assert(newOrder != nullptr && "Error: This order Id doesn't exist");
        orderMap.emplace(orderId, newOrder);

        if (stopMap.find(stopPrice) == stopMap.end())
            addStopLevel(stopPrice, side);

        stopMap[stopPrice]->addOrder(newOrder);
    }
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

    assert(order != nullptr && "Error: This order Id doesn't exist");

    Limit* parentLimit = order->getParentLimit();
    Side orderSide = order->getOrderSide();

    order->cancelOrder();
    if (parentLimit->getNumberOfOrders() == 0)
        deleteStopLevel(parentLimit);

    order->amendOrder(newShares, newstopPrice);

    if (stopMap.find(newstopPrice) == stopMap.end()) // New Stop price
        addStopLevel(newstopPrice, orderSide);

    stopMap[newstopPrice]->addOrder(order);
}


void OrderBook::executeMarketOrder(Side side, int& shares){
    // The max possible number of shares is traded. At the end, shares equals the remaining number of shares, or 0 if none is left
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

void OrderBook::addMarketOrder(Side side, int shares){
    // First, we execute the market order
    executeMarketOrder(side, shares);
    // Then we see if the changes that happened to the order book can trigger a stop order
    executeStopOrders(side);
}

