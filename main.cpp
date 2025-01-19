#include <iostream>

#include "Order.cpp"
#include "Limit.cpp"
#include "AvlTree.cpp"
#include "OrderBook.cpp"


int main(){
    OrderBook myOrderBook = OrderBook();
    Order myOrder = Order(1, Side::Bid, 1, 100);
    myOrder.showOrder();

    return 0;
}