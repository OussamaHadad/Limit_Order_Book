#include <iostream>

#include "Limit.h"
#include "Order.h"

Limit::Limit(int _limitPrice, Side _side, int _numberOfOrders, int _totalShares):
    limitPrice(_limitPrice), side(_side), numberOfOrders(_numberOfOrders), totalShares(_totalShares), 
    headOrder(nullptr), tailOrder(nullptr), parentLimit(nullptr), leftChildLimit(nullptr), rightChildLimit(nullptr) 
    {}

Limit::~Limit(){
    /* Destructor of Limit level from the Order Book AVL tree
    When deleting a Limit level we face 3 cases: the Limit has no children (leaf node), or it has one child node, or it has 2 child nodes 
    This decstructor replaces the destructed node by the bottom left node of its right subtree  */

    if (parentLimit == nullptr){ // The Root node
        // Case 1: Node has at most one child node
        if (leftChildLimit == nullptr && rightChildLimit == nullptr){ // Leaf Node -> No changes
            return;
        }
        else if (leftChildLimit == nullptr){ // Node has only right child node
            rightChildLimit->setParentLimit(nullptr);
            return;
        }
        else if (rightChildLimit == nullptr){ // Node has only left child node
            leftChildLimit->setParentLimit(nullptr);
            return;
        }

        // Case 2: Node has two child nodes; We choose the new root node to be the most left node in the right subtree
        Limit* temp = rightChildLimit;
        while (temp->leftChildLimit != nullptr)
            temp = temp->getLeftChildLimit();
        
        if (rightChildLimit->getLeftChildLimit() != nullptr){ // else, then we simply set rightChildLimit as the new root node
            temp->getParentLimit()->setLeftChildLimit(temp->getRightChildLimit());  // Update temp's parent's left child
            if (temp->getRightChildLimit()) // Update temp's right child's parent
                temp->getRightChildLimit()->setParentLimit(temp->getParentLimit());
            temp->setRightChildLimit(rightChildLimit);
            rightChildLimit->setParentLimit(temp);
        }
        temp->setLeftChildLimit(leftChildLimit);
        leftChildLimit->setParentLimit(temp);
    }

    else{ // not the Root node
        bool isLeftChild = (limitPrice < parentLimit->getLimitPrice()); // true if this node is a left child, else false
        
        // Case 1: Node has at most one child node
        if (leftChildLimit == nullptr){ // Node has no left child
            if (isLeftChild)
                parentLimit->setLeftChildLimit(rightChildLimit);
            else
                parentLimit->setRightChildLimit(rightChildLimit);

            if (rightChildLimit != nullptr)
                rightChildLimit->setParentLimit(parentLimit);
            return;
        }
        else if (rightChildLimit == nullptr){ // Node has no right child
            if (isLeftChild)
                parentLimit->setLeftChildLimit(leftChildLimit);
            else
                parentLimit->setRightChildLimit(leftChildLimit);

            if (leftChildLimit != nullptr)
                leftChildLimit->setParentLimit(parentLimit);
            return;
        }

        // Case 2: Node has two child nodes; We choose the new root node to be the most left node in the right subtree
        Limit* temp = rightChildLimit;
        while (temp->leftChildLimit != nullptr)
            temp = temp->getLeftChildLimit();

        if (rightChildLimit->getLeftChildLimit() != nullptr){ // else, then we simply set rightChildLimit as a replacement for the node
            temp->getParentLimit()->setLeftChildLimit(temp->getRightChildLimit());  // Update temp's parent's left child
            if (temp->getRightChildLimit()) // Update temp's right child's parent
                temp->getRightChildLimit()->setParentLimit(temp->getParentLimit());
            temp->setRightChildLimit(rightChildLimit);
            rightChildLimit->setParentLimit(temp);
        }
        temp->setParentLimit(parentLimit);
        temp->setLeftChildLimit(leftChildLimit);
        leftChildLimit->setParentLimit(temp);
        if (isLeftChild)
            parentLimit->setLeftChildLimit(temp);
        else
            parentLimit->setRightChildLimit(temp);
    }
}

void Limit::showLimit() const{
    Order* pseudoHeadOrder = headOrder;
    std::cout << "Following are the IDs of Orders part of this limit level" << std::endl;
    while (pseudoHeadOrder != nullptr){
        std::cout << "Order ID: " << pseudoHeadOrder->getOrderId() << std::endl;
        pseudoHeadOrder = pseudoHeadOrder->getNextOrder();
    }
}

int Limit::getLimitPrice() const{
    return limitPrice;
}

Side Limit::getSide() const{
    return side;
}

int Limit::getNumberOfOrders() const{
    return numberOfOrders;
}

int Limit::getTotalShares() const{
    return numberOfOrders;
}

Order* Limit::getHeadOrder() const{
    return headOrder;
}

Order* Limit::getTailOrder() const{
    return tailOrder;
}

Limit* Limit::getParentLimit() const{
    return parentLimit;
}

Limit* Limit::getLeftChildLimit() const{
    return leftChildLimit;
}

Limit* Limit::getRightChildLimit() const{
    return rightChildLimit;
}

void Limit::setParentLimit(Limit* newParentLimit){
    parentLimit = newParentLimit;
}

void Limit::setLeftChildLimit(Limit* newLeftChildLimit){
    leftChildLimit = newLeftChildLimit;
}

void Limit::setRightChildLimit(Limit* newRightChildLimit){
    rightChildLimit = newRightChildLimit;
}

void Limit::addOrder(Order* order){
    if (headOrder == nullptr)
        headOrder = tailOrder = order;
    else{   // order is the new tailOrder of this limit level
        tailOrder->nextOrder = order;
        order->previousOrder = tailOrder;
        order->nextOrder = nullptr;
        tailOrder = order;
    }
    ++numberOfOrders;
    totalShares += order->getOrderShares();
    order->parentLimit = this;
}

