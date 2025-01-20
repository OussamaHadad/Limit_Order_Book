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
    // When destructing the order book, we go through every limit/stop order and every limit/stop level from their map and delete it
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


// Auxiliary methods used inside other methods
void OrderBook::stopOrderToLimitOrder(Order* order, Side side){
    /* Turn a stop limit order into a limit order.
    First, we execute the stop limit order if possible, and then we add a limit order using the remaining number of shares */

    auto& bookEdge = (side == Side::Bid) ? lowestStopBid : highestStopAsk;

    int tradedShares = std::min(order->getOrderShares(), bookEdge->getTotalShares());
    order->executeOrder(tradedShares);

    if (tradedShares == bookEdge->getTotalShares()) // All shares from the current book edge were traded, hence delete the book edge limit
        deleteLevel(bookEdge, limitORstop::stop);

    if (order->getOrderShares() != 0){
        auto& limitMap = (side == Side::Bid) ? limitBidMap : limitAskMap;

        if (limitMap.find(order->getLimitPrice()) == limitMap.end())
            addLimit(order->getLimitPrice(), side);

        limitMap[order->getLimitPrice()]->addOrder(order);
    }
}

// Execute orders method
void OrderBook::executeStopOrders(Side side){
    /* We go through Stop orders and execute those that were triggered if there are enough shares in the order book
        If a stop order is partially executed, then we make a limit order from the remaining shares */
    if (side == Side::Bid){
        while (lowestStopBid != nullptr && lowestAsk != nullptr && lowestStopBid->getLimitPrice() <= lowestAsk->getLimitPrice()){
            Order* headOrder = lowestStopBid->getHeadOrder();

            int tradedShares = std::min(headOrder->getOrderShares(), lowestAsk->getTotalShares());
            headOrder->executeOrder(tradedShares);

            while (tradedShares > 0 && lowestAsk->getHeadOrder() != nullptr){
                Order* askHeadOrder = lowestAsk->getHeadOrder();
                int _tradedShares = std::min(askHeadOrder->getOrderShares(), tradedShares);
                askHeadOrder->executeOrder(_tradedShares);
                tradedShares -= _tradedShares;
                
                if (askHeadOrder->getOrderShares() == 0){
                    orderMap.erase(askHeadOrder->getOrderId());
                    askHeadOrder->cancelOrder();
                    delete askHeadOrder;
                }
            }

            if (lowestAsk->getHeadOrder() == nullptr){
                updateBookEdge(lowestAsk, limitORstop::limit);
                deleteLevel(lowestAsk, limitORstop::limit);
            }

           if (tradedShares == headOrder->getOrderShares()){
                orderMap.erase(headOrder->getOrderId());
                headOrder->cancelOrder(); // We cancel headOrder in order to update both head and tail orders of lowestStopBid
                delete headOrder;

                if (lowestStopBid->getHeadOrder() == nullptr){
                    updateBookEdge(lowestStopBid, limitORstop::stop);
                    deleteLevel(lowestStopBid, limitORstop::stop);
                }
            }
            else    
                stopOrderToLimitOrder(headOrder, side);
        }
    }
    else{ // side == Side::Ask
        while (highestStopAsk != nullptr && highestBid != nullptr && highestStopAsk->getLimitPrice() >= highestBid->getLimitPrice()){
            Order* headOrder = highestStopAsk->getHeadOrder();

            int tradedShares = std::min(headOrder->getOrderShares(), highestBid->getTotalShares());
            headOrder->executeOrder(tradedShares);
            
            while (tradedShares > 0 && highestBid->getHeadOrder() != nullptr){
                Order* askHeadOrder = highestBid->getHeadOrder();
                int _tradedShares = std::min(askHeadOrder->getOrderShares(), tradedShares);
                askHeadOrder->executeOrder(_tradedShares);
                tradedShares -= _tradedShares;
                
                if (askHeadOrder->getOrderShares() == 0){
                    orderMap.erase(askHeadOrder->getOrderId());
                    askHeadOrder->cancelOrder();
                    delete askHeadOrder;
                }
            }

            if (highestBid->getHeadOrder() == nullptr){
                updateBookEdge(highestBid, limitORstop::limit);
                deleteLevel(highestBid, limitORstop::limit);
            }

           if (tradedShares == headOrder->getOrderShares()){
                orderMap.erase(headOrder->getOrderId());
                headOrder->cancelOrder(); // We cancel headOrder in order to update both head and tail orders of lowestStopBid
                delete headOrder;

                if (lowestStopBid->getHeadOrder() == nullptr){
                    updateBookEdge(highestStopAsk, limitORstop::stop);
                    deleteLevel(highestStopAsk, limitORstop::stop);
                }
            }
            else    
                stopOrderToLimitOrder(headOrder, side);
        }
    }
}


// Limit tree's methods
void OrderBook::addLimit(int limitPrice, Side side){
    // Add a new limit level to its limit map and limit tree, then check if it can its book's new edge
    auto& limitMap = (side == Side::Bid) ? limitBidMap : limitAskMap;
    auto& tree = (side == Side::Bid) ? bidTree : askTree;
    auto& bookEdge = (side == Side::Bid) ? highestBid : lowestAsk;

    Limit* newLimit = new Limit(limitPrice, side);
    limitMap.emplace(limitPrice, newLimit);

    if (tree == nullptr) // This limit's tree is empty
        tree = bookEdge = newLimit;
    else{
        // Update tree's root if needed
        Limit* unused = insertNewLevel(tree, newLimit, nullptr, limitORstop::limit);
        // Update book's edge if needed
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


// Stop tree's methods
void OrderBook::addStopLevel(int stopPrice, Side side){
    auto& stopTree = (side == Side::Bid) ? stopBidTree : stopAskTree;
    auto& bookEdge = (side == Side::Bid) ? lowestStopBid : highestStopAsk;

    Limit* newStop = new Limit(stopPrice, side);
    stopMap.emplace(stopPrice, newStop);

    if (stopTree == nullptr) // There is no stop level in this tree yet 
        stopTree = bookEdge = newStop;
    else{
        // Update tree's root if needed
        Limit* unused = insertNewLevel(stopTree, newStop, nullptr, limitORstop::stop);
        // Update book's edge if needed
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


// Limit and Stop trees' shared methods
Limit* OrderBook::insertNewLevel(Limit* root, Limit* newLevel, Limit* parentLevel, limitORstop limit_or_stop){
    /* Recursive function used to insert a new limit/stop level in the Bid/Ask limit/stop AVL tree.
    Returns the root of the tree (or sub-tree for recursive calls) where newLevel (stop or a limit) is inserted. */

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

void OrderBook::deleteLevel(Limit* level, limitORstop limit_or_stop){
    /* When deleting a stop/limit level we do the following (all if needed):
            Update book edge  ->  Update tree root  ->  Rebalance AVL tree -&- Update parent/children of connected nodes */

    updateBookEdge(level, limit_or_stop);
    updateTreeRoot(level, limit_or_stop);

    Limit* parentLimit = level->getParentLimit();
    int levelPrice = level->getLimitPrice();

    (limit_or_stop == limitORstop::stop) ? stopMap.erase(levelPrice) 
        : (level->getSide() == Side::Bid) ? limitBidMap.erase(levelPrice) : limitAskMap.erase(levelPrice);
    delete level;

    while (parentLimit != nullptr){
        parentLimit = balanceTree(parentLimit, limitORstop::stop);
        
        if (parentLimit->getParentLimit() != nullptr){

            if (parentLimit->getParentLimit()->getLimitPrice() > levelPrice)
                parentLimit->getParentLimit()->setLeftChildLimit(parentLimit);
            else
                parentLimit->getParentLimit()->setRightChildLimit(parentLimit);
        }

        parentLimit = parentLimit->getParentLimit();
    }
}


// Limit order methods
void OrderBook::addLimitOrder(int orderId, Side side, int limitPrice, int shares){
    // We trade the biggest possible number of shares, and then we make a limit order from the remaining shares
    if (side == Side::Bid){
        while (shares != 0 && lowestAsk != nullptr && limitPrice >= lowestAsk->getLimitPrice())
            executeMarketOrder(side, shares);
    }   
    else{ // side == Side::Ask
        while (shares != 0 && highestBid != nullptr && limitPrice <= highestBid->getLimitPrice())
            executeMarketOrder(side, shares);
    }

    if (shares != 0){ // some or all shares are left
        Order* newOrder = new Order(orderId, side, shares, limitPrice);
        orderMap.emplace(orderId, newOrder);

        auto& limitMap = (side == Side::Bid) ? limitBidMap : limitAskMap;

        if (limitMap.find(limitPrice) == limitMap.end())
            addLimit(limitPrice, side);
        
        limitMap[limitPrice]->addOrder(newOrder);
    }
    else // all shares were traded, hence we check if some stop orders can be executed now that the order book was updated
        executeStopOrders(side);
}

void OrderBook::cancelLimitOrder(int orderId){
    // Cancel order, Delete limit level if empty, then Delete order from orderMap and deallocate memory 
    Order* order = orderMap[orderId];
    
    order->cancelOrder();
    
    if (order->getParentLimit()->getNumberOfOrders() == 0)
        deleteLevel(order->getParentLimit(), limitORstop::limit);
    
    orderMap.erase(orderId);
    delete order; 
}

void OrderBook::modifyLimitOrder(int orderId, int newShares, int newLimitPrice){
    Order* order = orderMap[orderId];
    assert(order != nullptr && "Error: This order Id doesn't exist");

    order->cancelOrder();

    Limit* parentLimit = order->getParentLimit();
    if (parentLimit->getNumberOfOrders() == 0)
        deleteLevel(parentLimit, limitORstop::limit);

    order->amendOrder(newShares, newLimitPrice);

    Side side = order->getOrderSide();
    auto& limitMap = (side == Side::Bid) ? limitBidMap : limitAskMap;

    if (limitMap.find(newLimitPrice) == limitMap.end()) // New limit price
        addLimit(newLimitPrice, side);

    limitMap[newLimitPrice]->addOrder(order);
}


// Stop order methods
void OrderBook::addStopOrder(int orderId, Side side, int stopPrice, int shares){    
    // First, we execute the stop order if possible, and then we make a new stop order from the remaining shares
    if (side == Side::Bid && lowestAsk != nullptr && stopPrice <= lowestAsk->getLimitPrice()) // the last condition triggers the stop order
        executeMarketOrder(side, shares);
    else if (side == Side::Ask && highestBid != nullptr && stopPrice >= highestBid->getLimitPrice()) // ...
        executeMarketOrder(side, shares);

    if (shares != 0){ // The remaining shares are turned into a stop order
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
        deleteLevel(order->getParentLimit(), limitORstop::stop);
    
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
        deleteLevel(parentLimit, limitORstop::stop);

    order->amendOrder(newShares, newstopPrice);

    if (stopMap.find(newstopPrice) == stopMap.end()) // New Stop price
        addStopLevel(newstopPrice, orderSide);

    stopMap[newstopPrice]->addOrder(order);
}


void OrderBook::executeMarketOrder(Side side, int& shares){
    // The max possible number of shares is traded. At the end, shares takes as a value the number of remaining shares
    auto& bookEdge = (side == Side::Bid) ? lowestAsk : highestBid;

    while (shares > 0 && bookEdge != nullptr){
        Order* headOrder = bookEdge->getHeadOrder(); // The first order to be executed from the bookEdge level
        int tradedShares = std::min(headOrder->getOrderShares(), shares);
        
        headOrder->executeOrder(tradedShares); // Head Order is executed
        shares -= tradedShares; // The remaining number of shares from the market order

        if (headOrder->getOrderShares() == 0){ // headOrder was completely executed
            orderMap.erase(headOrder->getOrderId());
            headOrder->cancelOrder(); // We cancel headOrder in order to update both head and tail orders of bookEdge
            delete headOrder;

            if (bookEdge->getNumberOfOrders() == 0){
                // This limit level has no more orders left, hence we move to the next book edge, and the current is deleted 
                Limit* prevBookEdge = bookEdge;
                updateBookEdge(bookEdge, limitORstop::limit);
                deleteLevel(prevBookEdge, limitORstop::limit);
            }
        }
    }
}

void OrderBook::addMarketOrder(Side side, int shares){
    // First, we execute the market order
    executeMarketOrder(side, shares);
    // Then we check if any stop orders were triggered after the order book was updated
    executeStopOrders(side);
}

