#include <algorithm>

#include "enums.h"
#include "Limit.h"
#include "OrderBook.h"


void OrderBook::updateBookRoot(Limit* level, limitORstop limit_or_stop){
    /* If level (limit or stop) is deleted and it is the root of one of the bid/ask trees, 
    this function is used to update the root of the corresponding tree */

    Limit* treeRoot = (limit_or_stop == limitORstop::limit) ? 
                            ((level->getSide() == Side::Bid) ? bidTree : askTree) 
                        :
                            ((level->getSide() == Side::Bid) ? stopBidTree : stopAskTree);

    if (level == treeRoot){

        if (treeRoot->getRightChildLimit() == nullptr)
            treeRoot = treeRoot->getLeftChildLimit();

        else{ // The root node has both a left and a right child, hence the new root will be the most left child of its right subtree
            treeRoot = treeRoot->getRightChildLimit();
            
            while (treeRoot->getLeftChildLimit() != nullptr)
                treeRoot = treeRoot->getLeftChildLimit();
        }
    }
}

void OrderBook::updateBookEdge(Limit* level, limitORstop limit_or_stop){
    // If the book edge is empty, this function replaces it with the next book edge in the order of Limits

    Limit* bookEdge = (limit_or_stop == limitORstop::limit) ? 
                            ((level->getSide() == Side::Bid) ? bidTree : askTree) 
                        :
                            ((level->getSide() == Side::Bid) ? stopBidTree : stopAskTree);

    if (level == bookEdge){
        if (level->getSide() == Side::Bid && level->getLeftChildLimit() != nullptr) // bookedge from buy side can't have a right child
            bookEdge = level->getLeftChildLimit();
        else if (level->getSide() == Side::Ask && level->getRightChildLimit() != nullptr) // bookedge from buy side can't have a left child
            bookEdge = level->getRightChildLimit();
        else
            bookEdge = level->getParentLimit();
    }
}


int OrderBook::getLimitHeight(Limit* limit) const{
    // Note: the height of the bottom level's limits is 1 not 0
    if (limit == nullptr)
        return 0;

    int l_child_height = getLimitHeight(limit->getLeftChildLimit());
    int r_child_height = getLimitHeight(limit->getRightChildLimit());

    return 1 + std::max(l_child_height, r_child_height);
}

int OrderBook::limitHeightDifference(Limit* limit) const{
    // Note: By definition of AVL Tree, the height of the left side is greater than or equal to the height of the right side
    int l_side_height = getLimitHeight(limit->getLeftChildLimit());
    int r_side_height = getLimitHeight(limit->getRightChildLimit());

    return std::abs(l_side_height - r_side_height);
}

Limit* OrderBook::rRotate(Limit* parentLimit, limitORstop limit_or_stop){
    Limit* newParentLimit = parentLimit->getRightChildLimit();

    parentLimit->setRightChildLimit(newParentLimit->getLeftChildLimit());
    if (newParentLimit->getLeftChildLimit() != nullptr)
        newParentLimit->getLeftChildLimit()->setParentLimit(parentLimit);

    newParentLimit->setLeftChildLimit(parentLimit);

    if (parentLimit->getParentLimit() != nullptr)
        newParentLimit->setParentLimit(parentLimit->getParentLimit());
    else{
        newParentLimit->setParentLimit(nullptr);

        if (limit_or_stop == limitORstop::limit)
            (parentLimit->getSide() == Side::Bid) ? setBidTree(newParentLimit) : setAskTree(newParentLimit);
        else // We assume: limit_or_stop == limitORstop::limit
            (parentLimit->getSide() == Side::Bid) ? setStopBidTree(newParentLimit) : setStopAskTree(newParentLimit);
    }
    parentLimit->setParentLimit(newParentLimit);
    return newParentLimit;
}

Limit* OrderBook::lRotate(Limit* parentLimit, limitORstop limit_or_stop){
    Limit* newParentLimit = parentLimit->getLeftChildLimit();

    parentLimit->setLeftChildLimit(newParentLimit->getRightChildLimit());

    if (newParentLimit->getRightChildLimit() != nullptr)
        newParentLimit->getRightChildLimit()->setParentLimit(parentLimit);
    
    newParentLimit->setRightChildLimit(parentLimit);

    if (parentLimit->getParentLimit() != nullptr)
        newParentLimit->setParentLimit(parentLimit->getParentLimit());
    else{
        newParentLimit->setParentLimit(nullptr);

        if (limit_or_stop == limitORstop::limit)
            (parentLimit->getSide() == Side::Bid) ? setBidTree(newParentLimit) : setAskTree(newParentLimit);
        else // We assume: limit_or_stop == limitORstop::limit
            (parentLimit->getSide() == Side::Bid) ? setStopBidTree(newParentLimit) : setStopAskTree(newParentLimit);
    }
    parentLimit->setParentLimit(newParentLimit);
    return newParentLimit;
}

Limit* OrderBook::lrRotate(Limit* parentLimit, limitORstop limit_or_stop){
    Limit* newParentLimit = parentLimit->getLeftChildLimit();
    parentLimit->setLeftChildLimit(rRotate(newParentLimit, limit_or_stop));
    return lRotate(parentLimit, limit_or_stop);
}

Limit* OrderBook::rlRotate(Limit* parentLimit, limitORstop limit_or_stop){
    Limit* newParentLimit = parentLimit->getRightChildLimit();
    parentLimit->setRightChildLimit(lRotate(newParentLimit, limit_or_stop));
    return rRotate(parentLimit, limit_or_stop);
}

Limit* OrderBook::balanceTree(Limit* limit, limitORstop limit_or_stop){
    int balanceFactor = limitHeightDifference(limit);

    if (balanceFactor > 1) {
        if (limitHeightDifference(limit->getLeftChildLimit()) >= 0)
            limit = lRotate(limit, limit_or_stop);
        else
            limit = lrRotate(limit, limit_or_stop);
    } 
    else if (balanceFactor < -1) {
        if (limitHeightDifference(limit->getRightChildLimit()) > 0)
            limit = rlRotate(limit, limit_or_stop);
        else
            limit = rRotate(limit, limit_or_stop);
    }
    return limit;
}

