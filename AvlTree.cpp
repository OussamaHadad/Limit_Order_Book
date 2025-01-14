#include<algorithm>

#include "enums.h"
#include "Limit.h"
#include "OrderBook.h"


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

