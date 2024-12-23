#include <iostream>


class Trade;


class OrderBook {

private:
    // Limit Orders
    Trade* bidTree; // Bid == Buy
    Trade* askTree; // Ask == Sell
    Trade* lowestAsk;
    Trade* highestBid;

    // Stop Orders are activated, whether buy or sell, when a certain price is reached
    Trade* stopBidTree; // Bid == Buy
    Trade* stopAskTree; // Ask == Sell
    Trade* lowestStopAsk;
    Trade* highestStopBid;


public:



};