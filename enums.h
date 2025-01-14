
enum class Side {Bid, Ask};    // 0 for Bid & 1 for Ask

enum class OrderType {LimitOrder, MarketOrder, StopOrder}; // Sell/Buy at least/most at the limit price; Sell/Buy immediately; Sell/Buy immediately after the other side's price goes above/below the stop price

enum class TIF {GTC, DAY, IOC, FOK};    // Time in force : {Good Till Cancel, Day, Immediate or Cancel Order, Fill or Kill Order}

enum class limitORstop {limit, stop}; // Used to know if the concerned AVL tree is a stop tree or a limit tree