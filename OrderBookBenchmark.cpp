#include <iostream>
#include <chrono>
#include <random>
#include "OrderBook.h"

class OrderBookBenchmark {
public:
    static void run_benchmark(int num_orders) {
        OrderBook book;
        auto start = std::chrono::high_resolution_clock::now();

        // Test pattern: 80% limit orders, 20% cancellations
        std::mt19937 gen(42);
        std::uniform_int_distribution<> price_dist(1, 1000);
        std::uniform_int_distribution<> shares_dist(1, 100);
        std::bernoulli_distribution cancel_dist(0.2);

        for(int i = 1; i <= num_orders; ++i) {
            int price = price_dist(gen);
            int shares = shares_dist(gen);
            OrderSide orderSide = (i % 2) ? OrderSide::Bid : OrderSide::Ask;

            if(cancel_dist(gen) && !book.getOrderMap().empty()) {
                // Cancel random existing order
                auto it = book.getOrderMap().begin();
                std::advance(it, rand() % book.getOrderMap().size());
                book.cancelLimitOrder(it->first);
            } else {
                // Add new limit order
                book.addLimitOrder(i, orderSide, price, shares);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        double tps = num_orders / (duration / 1000.0);
        std::cout << "Processed " << num_orders << " transactions in " 
                  << duration << "ms (" << tps << " tps)\n";
    }
};