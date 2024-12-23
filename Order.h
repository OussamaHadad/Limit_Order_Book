#include <iostream>
#include <ctime>


enum class Side {Buy, Sell};    // 0 for Buy & 1 for Sell
enum class OrderType {LimitOrder, MarketOrder, StopOrder};
enum class TIF {GTC, DAY, IOC, FOK};    // Time in force : {Good Till Cancel, Day, Immediate or Cancel Order, Fill or Kill Order}


class Trade;    // We declare Trade class before defining its attributes and the prototypes of its methods in Trade.h

class Order {
    
    friend class Trade; // Trade class is a friend of Order class, thus it can access the private attributes of Order class

private:
    // Following are the primary attributes of an order
    int idNumber;
    Side side;
    int shares; // number of shares in the order
    int limitPrice;
    OrderType type;
    TIF tif;
    std::time_t submissionTime;

    // The following attributes are introduced to improve the time complexity of recurrent manipulations
    Trade* parentTrade; // The trade to which the order belongs
    Order* previousOrder;   // The previous order in the linked list of orders of the same trade
    Order* nextOrder;   // The next order in the linked list of orders of the same trade

public:
    Order(int _idNumber, Side _side, int _shares, int _limitPrice, OrderType _type = OrderType::LimitOrder, TIF _tif = TIF::GTC); // Class constructor

    void showOrder() const;

    int getOrderId() const;
    Side getOrderSide() const;
    int getShares() const;
    int getLimitPrice() const;
    OrderType getOrderType() const;
    TIF getTIF() const;
    Trade* getParentTrade() const;
    Order* getNextOrder() const;

    void amendOrder(int newShares, int newLimitPrice);  // Modify order, either by changing the number of shares or the limit price
    void cancelOrder();
    void executeOrder(int tradedShares);
};


class IcebergOrder  :   public Order    {
    // an Iceberg order has a visible number of shares that appears in the order book meanwhile its "real" number of shares is hidden 
    int visibleShares;

public:
    void amendOrder(int newVisibleShares, int newShares, int newLimitPrice);
    void executeOrder(int tradedShares);
};