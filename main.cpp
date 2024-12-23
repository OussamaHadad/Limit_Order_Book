#include <iostream>
#include "Order.cpp"


int main(){
    Order myOrder = Order(1, Side::Buy, 1, 100);
    myOrder.showOrder();

    return 0;
}