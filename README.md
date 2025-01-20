# Limit Order Book
This Limit Order Book is developed using C++11 from scratch. It is mainly inspired from this other project: https://github.com/brprojects/Limit-Order-Book.

# Order Types:
1° Limit Order: An order to trade (i.e: bid or ask) a number of shares at a certain price.
2° Stop Order: An order to trade a number of shares once a stop price is exceeded when buying or subceeded when selling.
3° Market Order: An order to trade a number of shares at the market price.

# Data Structures Choices:
Initially, we have 4 empty AVL trees: bid tree, ask tree, stop bid tree and stop ask tree. AVL trees are self-balancing binary search trees where the height difference between the left subtree and the right subtree is at most 1. This property is essential to have a logarithmic complexity when adding, removing and cancelling orders.

# Complexity:
1° Add Order: O(log(M)), where M is the number of levels (e.g: limit prices from buy side for limit buy orders, stop prices from ask side for stop ask orders, etc.) for a new limit level as this level should be added to the corresponding AVL tree in O(log(M)). If the level isn't new, then O(1).
2° Remove Order: O(1) as the order is simply removed from the orders map; but if its level is emptied by this operation, this level will be removed from its tree in O(log(M)).
3° Modify Order: O(1); but it can be O(log(M)) if the previous level was emptied or the next level is new.
