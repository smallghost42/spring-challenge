# Ligue 1 - Bot Logic

## Overview
First iteration of the bot. Focus: **gather fruits and deposit them at the shack** as fast as possible. No buildings, no training, no combat. Pure PvE resource collection.

## Files
- `../bot.cpp` — main bot code (shared across ligues)
- `logic.md` — this file, design decisions & technical details
- `human_explain.md` — plain-english explanation of algorithms

---

## Core Algorithm

### 1. Map Parsing (initialisation)
- Read `width` and `height` (= 8 × 16 in L1).
- Build a 2D `grid` where each cell stores:
  - `type` — the raw character (`.` / `0` / `1`)
  - `walkable` — `true` only for `.`, `0`, `1` (only grass is traversable; shack is walkable until left)
- Locate `shackX, shackY` from the `0` cell.

### 2. Per-turn Game Loop

#### State parsing
- Read inventories (2 lines — ours then opponent's).
- Read `treeCount` then each tree's attributes.
- Read `trollCount` then each troll's attributes.
- Collect **our** trolls into `myTrolls` vector.

#### Decision logic for each troll
A simple **priority chain**:

| Priority | Condition | Action |
|----------|-----------|--------|
| 1 | Carrying fruit **AND** adjacent to shack | `DROP` |
| 2 | Standing on a tree with fruit > 0 | `HARVEST` |
| 3 | A fruit-bearing tree is reachable | `MOVE` towards it |
| 4 | Nothing to do | `WAIT` |

### 3. Pathfinding — BFS (Breadth-First Search)
- **Why BFS?** Unweighted grid, guarantees shortest path in O(W×H).
- Explores 4-directionally (up/down/left/right).
- Only traverses `walkable` cells.
- Returns the shortest path as a vector of (x,y) steps.
- The troll moves up to `movementSpeed` steps along that path per turn.

### 4. Tree Targeting
- A **reservation set** (`reservedTrees`) prevents two trolls targeting the same tree.
- Tree score: `fruits × 100 - distance` (prioritises close trees with many fruits).
- First troll with distance 0 on a fruiting tree gets priority to `HARVEST`.

### 5. DROP
- Troll must be **orthogonally adjacent** (Manhattan distance 1) to the shack.
- Transfers **all** carried fruit to the inventory (scoring 1 point each).

---

## Constraints respected
- Grid ≤ 16×8
- Max 100 turns per game
- Response per turn ≤ 50 ms (BFS on 128 cells is negligible)
- Multiple commands joined with `;`
- `cin.ignore()` after every read to avoid stream desync

---

## Known limitations (to fix in higher ligues)
- No training of new trolls.
- No tree planting.
- No opponent interaction / stealing.
- Troll that leaves the shack can never return (by game rules) — DROP is only possible if troll hasn't wandered too far.
- No handling of `chopPower` — trees are never cut.
- No cooldown-aware scheduling (harvests every turn even if tree is on cooldown).
