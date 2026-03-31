# Position & PnL Engine

A WebSocket server that tracks client positions and calculates profit/loss (realized and unrealized) using weighted average cost. Written in C++17 with Boost libraries.

## Overview

All fields are stored per client in an unordered_map (client_id → Position), providing O(1) average lookup.
NetQty (Net quantity of the position (positive = long, negative = short, zero = flat)):	Updated on each FILL. The engine maintains a running total based on buy/sell operations.
AvgPrice : Volume‑weighted average price of the open position.	Recalculated on each FILL when the position increases in the same direction (e.g., buying when long). When a trade reduces or flips the position, the average price may change accordingly (e.g., after covering a short, the remaining short keeps the original average).
RealizedPnL	: Profit/loss already locked in from closed portions of the position.	Updated during FILL when a trade reduces the existing position (e.g., selling part of a long, or buying to cover part of a short). The realized PnL is calculated using the difference between the trade price and the average price of the portion being closed.
UnrealizedPnL	: Profit/loss of the open position based on the current market price.	Calculated on the fly when a PRINT command is received. Uses the formula unrealizedPnL = netQty * (currentPrice – avgPrice). The currentPrice is set by the last PRICE command.

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
