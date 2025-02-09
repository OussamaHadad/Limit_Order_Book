#include <iostream>

#include "Order.cpp"
#include "Limit.cpp"
#include "AvlTree.cpp"
#include "OrderBook.cpp"


int main(){
    OrderBook myOrderBook = OrderBook();
    myOrderBook.addLimitOrder(1, OrderSide::Bid, 100, 1);
    myOrderBook.addLimitOrder(55, OrderSide::Ask, 200, 2);

    myOrderBook.displayAllOrders();

    return 0;
}