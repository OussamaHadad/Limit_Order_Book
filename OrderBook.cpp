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
    for (auto& pair : limitBidMap) // pair.first = limitBidPrice && pair.second = limitBid
        delete pair.second;     // ~Limit will delete its Orders

    for (auto& pair : limitAskMap) // pair.first = limitAskPrice && pair.second = limitAsk
        delete pair.second;     // ~Limit will delete its Orders

    for (auto& pair : stopMap) // pair.first = stopPrice && pair.second = stopLevel
        delete pair.second;

    // OrderMap now contains dangling pointers (Orders are owned by Limits)
    // No need to delete Orders manually here!
    orderMap.clear();  // Optional: Clear dangling pointers
}

// Auxiliary methods used inside other methods
void OrderBook::stopOrderToLimitOrder(Order* order, OrderSide orderSide){
    /* Turn a stop limit order into a limit order.
    First, we execute the stop limit order if possible, and then we add a limit order using the remaining number of shares */

    auto& bookEdge = (orderSide == OrderSide::Bid) ? lowestStopBid : highestStopAsk;

    int tradedShares = std::min(order->getOrderShares(), bookEdge->getTotalShares());
    order->executeOrder(tradedShares);

    if (tradedShares == bookEdge->getTotalShares()) // All shares from the current book edge were traded, hence delete the book edge limit
        deleteLevel(bookEdge, OrderCategory::Stop);

    if (order->getOrderShares() != 0){
        auto& limitMap = (orderSide == OrderSide::Bid) ? limitBidMap : limitAskMap;

        if (limitMap.find(order->getLimitPrice()) == limitMap.end())
            addLimit(order->getLimitPrice(), orderSide);

        limitMap[order->getLimitPrice()]->addOrder(order);
    }
}

// Execute orders method
void OrderBook::executeStopOrders(OrderSide orderSide){
    /* We go through Stop orders and execute those that were triggered if there are enough shares in the order book
        If a stop order is partially executed, then we make a limit order from the remaining shares */
    if (orderSide == OrderSide::Bid){
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
                updateBookEdge(lowestAsk, OrderCategory::Limit);
                deleteLevel(lowestAsk, OrderCategory::Limit);
            }

           if (tradedShares == headOrder->getOrderShares()){
                orderMap.erase(headOrder->getOrderId());
                headOrder->cancelOrder(); // We cancel headOrder in order to update both head and tail orders of lowestStopBid
                delete headOrder;

                if (lowestStopBid->getHeadOrder() == nullptr){
                    updateBookEdge(lowestStopBid, OrderCategory::Stop);
                    deleteLevel(lowestStopBid, OrderCategory::Stop);
                }
            }
            else    
                stopOrderToLimitOrder(headOrder, orderSide);
        }
    }
    else{ // orderSide == OrderSide::Ask
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
                updateBookEdge(highestBid, OrderCategory::Limit);
                deleteLevel(highestBid, OrderCategory::Limit);
            }

           if (tradedShares == headOrder->getOrderShares()){
                orderMap.erase(headOrder->getOrderId());
                headOrder->cancelOrder(); // We cancel headOrder in order to update both head and tail orders of lowestStopBid
                delete headOrder;

                if (lowestStopBid->getHeadOrder() == nullptr){
                    updateBookEdge(highestStopAsk, OrderCategory::Stop);
                    deleteLevel(highestStopAsk, OrderCategory::Stop);
                }
            }
            else    
                stopOrderToLimitOrder(headOrder, orderSide);
        }
    }
}


// Limit tree's methods
void OrderBook::addLimit(int limitPrice, OrderSide orderSide){
    // Add a new limit level to its limit map and limit tree, then check if it can its book's new edge
    auto& limitMap = (orderSide == OrderSide::Bid) ? limitBidMap : limitAskMap;
    auto& tree = (orderSide == OrderSide::Bid) ? bidTree : askTree;
    auto& bookEdge = (orderSide == OrderSide::Bid) ? highestBid : lowestAsk;

    Limit* newLimit = new Limit(limitPrice, orderSide);
    limitMap.emplace(limitPrice, newLimit);

    if (tree == nullptr) // This limit's tree is empty
        tree = bookEdge = newLimit;
    else{
        // Update tree's root if needed
        Limit* unused = insertNewLevel(tree, newLimit, nullptr, OrderCategory::Limit);
        // Update book's edge if needed
        if (orderSide == OrderSide::Bid){
            if (highestBid->getLimitPrice() < limitPrice)
                highestBid = newLimit;
        }
        else{ // OrderSide::Limit
            if (lowestAsk->getLimitPrice() > limitPrice)
                lowestAsk = newLimit;
        }
    }
}


// Stop tree's methods
void OrderBook::addStopLevel(int stopPrice, OrderSide orderSide){
    auto& stopTree = (orderSide == OrderSide::Bid) ? stopBidTree : stopAskTree;
    auto& bookEdge = (orderSide == OrderSide::Bid) ? lowestStopBid : highestStopAsk;

    Limit* newStop = new Limit(stopPrice, orderSide);
    stopMap.emplace(stopPrice, newStop);

    if (stopTree == nullptr) // There is no stop level in this tree yet 
        stopTree = bookEdge = newStop;
    else{
        // Update tree's root if needed
        Limit* unused = insertNewLevel(stopTree, newStop, nullptr, OrderCategory::Stop);
        // Update book's edge if needed
        if (orderSide == OrderSide::Bid){ // Then update the book edge
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
Limit* OrderBook::insertNewLevel(Limit* root, Limit* newLevel, Limit* parentLevel, OrderCategory orderCategory){
    /* Recursive function used to insert a new limit/stop level in the Bid/Ask limit/stop AVL tree.
    Returns the root of the tree (or sub-tree for recursive calls) where newLevel (stop or a limit) is inserted. */

    if (root == nullptr){ // First stop level to insert in this stop tree
        newLevel->setParentLimit(parentLevel); 
        return newLevel;
    }
    else if (newLevel->getLimitPrice() < root->getLimitPrice()){ // then move to the left subtree
        root->setLeftChildLimit(insertNewLevel(root->getLeftChildLimit(), newLevel, root, orderCategory));
        root = balanceTree(root, orderCategory);
    }
    else if (newLevel->getLimitPrice() > root->getLimitPrice()){ // then move to the right subtree
        root->setRightChildLimit(insertNewLevel(root->getRightChildLimit(), newLevel, root, orderCategory));
        root = balanceTree(root, orderCategory);
    }
    return root;
}

void OrderBook::deleteLevel(Limit* level, OrderCategory orderCategory){
    /* When deleting a stop/limit level we do the following (all if needed):
            Update book edge  ->  Update tree root  ->  Rebalance AVL tree -&- Update parent/children of connected nodes */

    updateBookEdge(level, orderCategory);
    updateTreeRoot(level, orderCategory);

    Limit* parentLimit = level->getParentLimit();
    int levelPrice = level->getLimitPrice();

    (orderCategory == OrderCategory::Stop) ? stopMap.erase(levelPrice) 
        : (level->getOrderSide() == OrderSide::Bid) ? limitBidMap.erase(levelPrice) : limitAskMap.erase(levelPrice);
    delete level;

    while (parentLimit != nullptr){
        parentLimit = balanceTree(parentLimit, OrderCategory::Stop);
        
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
void OrderBook::addLimitOrder(int orderId, OrderSide orderSide, int limitPrice, int shares){
    // We trade the biggest possible number of shares, and then we make a limit order from the remaining shares
    if (orderSide == OrderSide::Bid){
        while (shares != 0 && lowestAsk != nullptr && limitPrice >= lowestAsk->getLimitPrice())
            executeMarketOrder(orderSide, shares);
    }   
    else{ // orderSide == OrderSide::Ask
        while (shares != 0 && highestBid != nullptr && limitPrice <= highestBid->getLimitPrice())
            executeMarketOrder(orderSide, shares);
    }

    if (shares != 0){ // some or all shares are left
        Order* newOrder = new Order(orderId, orderSide, shares, limitPrice);
        orderMap.emplace(orderId, newOrder);

        auto& limitMap = (orderSide == OrderSide::Bid) ? limitBidMap : limitAskMap;

        if (limitMap.find(limitPrice) == limitMap.end())
            addLimit(limitPrice, orderSide);
        
        limitMap[limitPrice]->addOrder(newOrder);
    }
    else // all shares were traded, hence we check if some stop orders can be executed now that the order book was updated
        executeStopOrders(orderSide);
}

void OrderBook::cancelLimitOrder(int orderId){
    // Cancel order, Delete limit level if empty, then Delete order from orderMap and deallocate memory 
    Order* order = orderMap[orderId];
    
    order->cancelOrder();
    
    if (order->getParentLimit()->getNumberOfOrders() == 0)
        deleteLevel(order->getParentLimit(), OrderCategory::Limit);
    
    orderMap.erase(orderId);
    //delete order; Handled in ~Limit() 
}

void OrderBook::modifyLimitOrder(int orderId, int newShares, int newLimitPrice){
    Order* order = orderMap[orderId];
    assert(order != nullptr && "Error: This order Id doesn't exist");

    order->cancelOrder();

    Limit* parentLimit = order->getParentLimit();
    if (parentLimit->getNumberOfOrders() == 0)
        deleteLevel(parentLimit, OrderCategory::Limit);

    order->amendOrder(newShares, newLimitPrice);

    OrderSide orderSide = order->getOrderSide();
    auto& limitMap = (orderSide == OrderSide::Bid) ? limitBidMap : limitAskMap;

    if (limitMap.find(newLimitPrice) == limitMap.end()) // New limit price
        addLimit(newLimitPrice, orderSide);

    limitMap[newLimitPrice]->addOrder(order);
}


// Stop order methods
void OrderBook::addStopOrder(int orderId, OrderSide orderSide, int stopPrice, int shares){    
    // First, we execute the stop order if possible, and then we make a new stop order from the remaining shares
    if (orderSide == OrderSide::Bid && lowestAsk != nullptr && stopPrice <= lowestAsk->getLimitPrice()) // the last condition triggers the stop order
        executeMarketOrder(orderSide, shares);
    else if (orderSide == OrderSide::Ask && highestBid != nullptr && stopPrice >= highestBid->getLimitPrice()) // ...
        executeMarketOrder(orderSide, shares);

    if (shares != 0){ // The remaining shares are turned into a stop order
        Order* newOrder = new Order(orderId, orderSide, shares, stopPrice);
        assert(newOrder != nullptr && "Error: This order Id doesn't exist");
        orderMap.emplace(orderId, newOrder);

        if (stopMap.find(stopPrice) == stopMap.end())
            addStopLevel(stopPrice, orderSide);

        stopMap[stopPrice]->addOrder(newOrder);
    }
}

void OrderBook::cancelStopOrder(int orderId){
    // Cancel order, Delete limit level if empty, then Delete order from orderMap and deallocate memory 
    Order* order = orderMap[orderId];
    
    order->cancelOrder();
    
    if (order->getParentLimit()->getNumberOfOrders() == 0)
        deleteLevel(order->getParentLimit(), OrderCategory::Stop);
    
    orderMap.erase(orderId);
    delete order; 
}

void OrderBook::modifyStopOrder(int orderId, int newShares, int newstopPrice){
    Order* order = orderMap[orderId];

    assert(order != nullptr && "Error: This order Id doesn't exist");

    Limit* parentLimit = order->getParentLimit();
    OrderSide orderSide = order->getOrderSide();

    order->cancelOrder();
    if (parentLimit->getNumberOfOrders() == 0)
        deleteLevel(parentLimit, OrderCategory::Stop);

    order->amendOrder(newShares, newstopPrice);

    if (stopMap.find(newstopPrice) == stopMap.end()) // New Stop price
        addStopLevel(newstopPrice, orderSide);

    stopMap[newstopPrice]->addOrder(order);
}


void OrderBook::executeMarketOrder(OrderSide orderSide, int& shares){
    // The max possible number of shares is traded. At the end, shares takes as a value the number of remaining shares
    auto& bookEdge = (orderSide == OrderSide::Bid) ? lowestAsk : highestBid;

    while (shares > 0 && bookEdge != nullptr){
        Order* headOrder = bookEdge->getHeadOrder(); // The first order to be executed from the bookEdge level
        int tradedShares = std::min(headOrder->getOrderShares(), shares);
        
        headOrder->executeOrder(tradedShares); // Head Order is executed
        shares -= tradedShares; // The remaining number of shares from the market order

        if (headOrder->getOrderShares() == 0){ // headOrder was completely executed
            orderMap.erase(headOrder->getOrderId());
            headOrder->cancelOrder(); // We cancel headOrder in order to update both head and tail orders of bookEdge
            //delete headOrder; Handled in ~Limit()

            if (bookEdge->getNumberOfOrders() == 0){
                // This limit level has no more orders left, hence we move to the next book edge, and the current is deleted 
                Limit* prevBookEdge = bookEdge;
                updateBookEdge(bookEdge, OrderCategory::Limit);
                deleteLevel(prevBookEdge, OrderCategory::Limit);
            }
        }
    }
}

void OrderBook::addMarketOrder(OrderSide orderSide, int shares){
    // First, we execute the market order
    executeMarketOrder(orderSide, shares);
    // Then we check if any stop orders were triggered after the order book was updated
    executeStopOrders(orderSide);
}

// In OrderBook.cpp
void OrderBook::displayAllOrders(bool includeStopOrders) const {
    std::cout << "=== LIMIT ORDERS ===" << std::endl;
    std::cout << "\nBid Orders (Highest to Lowest):" << std::endl;
    traverseAndDisplay(bidTree, true, false); // Reverse in-order for bids

    std::cout << "\nAsk Orders (Lowest to Highest):" << std::endl;
    traverseAndDisplay(askTree, false, false); // In-order for asks

    if (includeStopOrders) {
        std::cout << "\n=== STOP ORDERS ===" << std::endl;
        std::cout << "\nStop Bid Orders (Lowest Stop Price First):" << std::endl;
        traverseAndDisplay(stopBidTree, false, true); // In-order for stop bids

        std::cout << "\nStop Ask Orders (Highest Stop Price First):" << std::endl;
        traverseAndDisplay(stopAskTree, true, true); // Reverse in-order for stop asks
    }
}
