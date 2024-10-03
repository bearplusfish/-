1. Describe how your exchange works.

   I split this project into three parts, the first part is the insert, remove and match operations on the node of the double linked list. The second part is the operations of market, include paser operation of string, and do the market sell/buy/amend/cancel operations, they will call the functions of node operations because orders are node in the list. The last part is the communication framework, exchange build two fifo (one for write and one for read) for each tarder, then fork and execl the trader process. They follow the rules of first write to the pipe, then signal SIGUSR1. So, exechange use signal handler for SIGUSR1 and SIGCHLD, after get SIGUSR1 then it read pipe and call market operation functions. When it get SIGCHLD, it will marked the child process is closed.

2. Describe your design decisions for the trader and how it's fault-tolerant.

   I implemented as a state machine. When receive SIGUSR1 then it read pipe and send the information to traderStateMachine. It has 4 states.

   **STATE_OPEN**: the init state, when receive "MARKET OPEN" it change to state STATE_WAITING

   **STATE_WAITING**: when receive "MARKET SELL product qty price" and qty < 1000 change to STATE_CONFIRMED, when receive "MARKET SELL product qty price" and qty >= 1000 change to STATE_CLOSED

   **STATE_CONFIRMED**: when receive "ACCEPTED orderId" and get correct orderId will change to STATE_WAITING

   **STATE_CLOSED**: exit trader

3. Describe your tests and how to run them.

   I write some functions in exchange to test the correctness of my double linked list and market operations.
