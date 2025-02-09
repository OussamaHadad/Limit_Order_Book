#include <algorithm>

#include "enums.h"
#include "Limit.h"
#include "OrderBook.h"


// Update the root of the AVL tree when a level is deleted
void OrderBook::updateTreeRoot(Limit* level, OrderCategory orderCategory) {
    auto& treeRoot = (orderCategory == OrderCategory::Limit) ? ((level->getOrderSide() == OrderSide::Bid) ? bidTree : askTree) 
        : ((level->getOrderSide() == OrderSide::Bid) ? stopBidTree : stopAskTree);

    if (level == treeRoot) {
        if (!treeRoot->getRightChildLimit())
            treeRoot = treeRoot->getLeftChildLimit();
        else {
            // The new root is the leftmost child of the right subtree
            treeRoot = treeRoot->getRightChildLimit();
            while (treeRoot->getLeftChildLimit())
                treeRoot = treeRoot->getLeftChildLimit();
        }
    }
}

// Update the book edge (highest bid or lowest ask) when a level is deleted
void OrderBook::updateBookEdge(Limit* level, OrderCategory orderCategory) {
    auto& bookEdge = (orderCategory == OrderCategory::Limit) ? ((level->getOrderSide() == OrderSide::Bid) ? highestBid : lowestAsk) 
        : ((level->getOrderSide() == OrderSide::Bid) ? lowestStopBid : highestStopAsk);

    if (level == bookEdge) {
        if (level->getOrderSide() == OrderSide::Bid && level->getLeftChildLimit())
            bookEdge = level->getLeftChildLimit();
        else if (level->getOrderSide() == OrderSide::Ask && level->getRightChildLimit())
            bookEdge = level->getRightChildLimit();
        else
            bookEdge = level->getParentLimit();
    }
}

// Get the height of a limit level in the AVL tree
int OrderBook::getLimitHeight(Limit* limit) const {
    if (!limit) 
        return 0;
    int leftHeight = getLimitHeight(limit->getLeftChildLimit());
    int rightHeight = getLimitHeight(limit->getRightChildLimit());
    return 1 + std::max(leftHeight, rightHeight);
}

// Calculate the height difference between left and right subtrees
int OrderBook::limitHeightDifference(Limit* limit) const {
    if (!limit) 
        return 0;
    int leftHeight = getLimitHeight(limit->getLeftChildLimit());
    int rightHeight = getLimitHeight(limit->getRightChildLimit());
    return leftHeight - rightHeight;
}

// Right rotation for AVL tree balancing
Limit* OrderBook::rRotate(Limit* parentLimit, OrderCategory orderCategory) {
    Limit* newParent = parentLimit->getRightChildLimit();
    parentLimit->setRightChildLimit(newParent->getLeftChildLimit());

    if (newParent->getLeftChildLimit())
        newParent->getLeftChildLimit()->setParentLimit(parentLimit);

    newParent->setLeftChildLimit(parentLimit);
    newParent->setParentLimit(parentLimit->getParentLimit());
    parentLimit->setParentLimit(newParent);

    if (!newParent->getParentLimit()) {
        if (orderCategory == OrderCategory::Limit)
            (parentLimit->getOrderSide() == OrderSide::Bid) ? setBidTree(newParent) : setAskTree(newParent);
        else
            (parentLimit->getOrderSide() == OrderSide::Bid) ? setStopBidTree(newParent) : setStopAskTree(newParent);
    }
    return newParent;
}

// Left rotation for AVL tree balancing
Limit* OrderBook::lRotate(Limit* parentLimit, OrderCategory orderCategory) {
    Limit* newParent = parentLimit->getLeftChildLimit();
    parentLimit->setLeftChildLimit(newParent->getRightChildLimit());

    if (newParent->getRightChildLimit())
        newParent->getRightChildLimit()->setParentLimit(parentLimit);

    newParent->setRightChildLimit(parentLimit);
    newParent->setParentLimit(parentLimit->getParentLimit());
    parentLimit->setParentLimit(newParent);

    if (!newParent->getParentLimit()) {
        if (orderCategory == OrderCategory::Limit)
            (parentLimit->getOrderSide() == OrderSide::Bid) ? setBidTree(newParent) : setAskTree(newParent);
        else
            (parentLimit->getOrderSide() == OrderSide::Bid) ? setStopBidTree(newParent) : setStopAskTree(newParent);
    }
    return newParent;
}

// Left-right rotation for AVL tree balancing
Limit* OrderBook::lrRotate(Limit* parentLimit, OrderCategory orderCategory) {
    parentLimit->setLeftChildLimit(rRotate(parentLimit->getLeftChildLimit(), orderCategory));
    return lRotate(parentLimit, orderCategory);
}

// Right-left rotation for AVL tree balancing
Limit* OrderBook::rlRotate(Limit* parentLimit, OrderCategory orderCategory) {
    parentLimit->setRightChildLimit(lRotate(parentLimit->getRightChildLimit(), orderCategory));
    return rRotate(parentLimit, orderCategory);
}

// Balance the AVL tree after insertion or deletion
Limit* OrderBook::balanceTree(Limit* limit, OrderCategory orderCategory) {
    int balanceFactor = limitHeightDifference(limit);

    if (balanceFactor > 1) { // Left-heavy
        if (limitHeightDifference(limit->getLeftChildLimit()) >= 0)
            return lRotate(limit, orderCategory);
        else
            return lrRotate(limit, orderCategory);
    } 
    else if (balanceFactor < -1) { // Right-heavy
        if (limitHeightDifference(limit->getRightChildLimit()) > 0)
            return rlRotate(limit, orderCategory);
        else
            return rRotate(limit, orderCategory);
    }
    return limit;
}

void OrderBook::traverseAndDisplay(Limit* root, bool reverseOrder, bool isStop) const {
    // Traverse the AVL tree and print orders
    if (!root) 
        return;

    if (reverseOrder) {
        // Reverse in-order (right, current, left) for descending prices (bids)
        traverseAndDisplay(root->getRightChildLimit(), reverseOrder, isStop);
        printLimitOrders(root, isStop);
        traverseAndDisplay(root->getLeftChildLimit(), reverseOrder, isStop);
    } 
    else {
        // In-order (left, current, right) for ascending prices (asks)
        traverseAndDisplay(root->getLeftChildLimit(), reverseOrder, isStop);
        printLimitOrders(root, isStop);
        traverseAndDisplay(root->getRightChildLimit(), reverseOrder, isStop);
    }
}

void OrderBook::printLimitOrders(Limit* limit, bool isStop) const {
    // Print all orders at a specific price level
    if (!limit || !limit->getHeadOrder()) 
        return;

    std::cout << "[Price: " << limit->getLimitPrice() 
              << "] | Side: " << (limit->getOrderSide() == OrderSide::Bid ? "Bid" : "Ask")
              << " | Type: " << (isStop ? "Stop" : "Limit") 
              << " | Orders:" << std::endl;

    Order* current = limit->getHeadOrder();
    while (current) {
        std::cout << "  Order ID: " << current->getOrderId()
                  << " | Shares: " << current->getOrderShares()
                  << " | TIF: ";
        switch (current->getTIF()) {
            case TimeInForce::GTC: std::cout << "GTC"; break;
            case TimeInForce::DAY: std::cout << "DAY"; break;
            case TimeInForce::IOC: std::cout << "IOC"; break;
            case TimeInForce::FOK: std::cout << "FOK"; break;
            default: std::cout << "Unknown";
        }
        std::cout << " | Submitted: " << current->getSubmissionTime() << std::endl;
        current = current->getNextOrder();
    }
    std::cout << "---------------------------------" << std::endl;
}