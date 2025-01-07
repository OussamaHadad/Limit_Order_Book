#include <iostream>
#include "Order.cpp"


int main(){
    Order myOrder = Order(1, Side::Bid, 1, 100);
    myOrder.showOrder();

    return 0;
}