#include <iostream>

#include "Order.cpp"
#include "Limit.cpp"
#include "AvlTree.cpp"
#include "OrderBook.cpp"
#include "OrderBookBenchmark.cpp"

int main(){
    /*
    OrderBook myOrderBook = OrderBook();
    myOrderBook.addLimitOrder(1, OrderSide::Bid, 100, 1);
    myOrderBook.addLimitOrder(55, OrderSide::Ask, 200, 2);

    myOrderBook.displayAllOrders();
    */

    // Warm-up run (cache warmup)
    OrderBookBenchmark::run_benchmark(1000);

    // Actual measurement
    OrderBookBenchmark::run_benchmark(1000000);

    return 0;
}