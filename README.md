# Position & PnL Engine

A WebSocket server that tracks client positions and calculates profit/loss (realized and unrealized) using weighted average cost.

Realized PnL: recorded when a trade reduces an existing position. For long positions, realized = qty * (sellPrice - avgPrice). For short positions, realized = qty * (avgPrice - buyPrice). When flipping sides, all of the original position is closed first, then a new position opens.

Unrealized PnL: computed on demand as netQty * (currentPrice - avgPrice). Explanation that this works for both long and short (negative netQty yields positive when price drops).We have also mention that average price is maintained as volume-weighted average, and that the engine uses in FIFO logic.

If a trade flips the position (e.g., selling more than the current long position), the entire remaining long is closed first, realising its full profit/loss, and then the excess quantity opens a new short position with the trade price as its average entry.

## Overview

All fields are stored per client in an unordered_map (client_id → Position), providing O(1) average lookup.All operations are performed in‑memory, and the server runs as a long‑lived process, accepting WebSocket connections on a specified port.

NetQty (Net quantity of the position (positive = long, negative = short, zero = flat)):	Updated on each FILL. The engine maintains a running total based on buy/sell operations.


AvgPrice : Volume‑weighted average price of the open position.	Recalculated on each FILL when the position increases in the same direction (e.g., buying when long). When a trade reduces or flips the position, the average price may change accordingly (e.g., after covering a short, the remaining short keeps the original average).


RealizedPnL	: Profit/loss already locked in from closed portions of the position.	Updated during FILL when a trade reduces the existing position (e.g., selling part of a long, or buying to cover part of a short). The realized PnL is calculated using the difference between the trade price and the average price of the portion being closed.


UnrealizedPnL	: Profit/loss of the open position based on the current market price.	Calculated on the fly when a PRINT command is received. Uses the formula unrealizedPnL = netQty * (currentPrice – avgPrice). The currentPrice is set by the last PRICE command.


## Average Price Maintenance
The average price is recalculated only when the position increases in the same direction (e.g., buying when long, selling when short). When a trade reduces or flips the position, the average price remains unchanged for the remaining portion (or is set to the trade price for a newly opened position).

Adding to a long:
newAvg = (netQty * oldAvg + quantity * tradePrice) / (netQty + quantity)

Adding to a short (selling more when already short):
newAvg = (|netQty| * oldAvg + quantity * tradePrice) / (|netQty| + quantity)
(the engine uses netQty negative, but the logic is symmetric.)

## Technical Architecture

### Components

- **`PositionEngine`** – core logic for managing client positions. Stores positions in an `unordered_map` keyed by `client_id`.
- **`WebSocketServer`** – handles incoming WebSocket connections, parses JSON commands, and delegates to the engine. Each connection is handled by a `Session` object.
- **`main.cpp`** – sets up the I/O context, starts the server, and handles signals.

### Concurrency

The server runs a single `io_context` thread. All sessions are processed asynchronously on that same thread, so no locking is required. The engine is accessed directly without additional synchronization.

## Calculation Logic

### Net Quantity and Average Price

- **Buy orders** – if netQty ≥ 0 (long or flat), the position is averaged:
