# Human explanation — Ligue 1 bot

## What does the bot do, in simple terms?

You have **one troll** and a **shack** on a small grassy map with fruit trees.
Each turn the bot decides:

1. **"Am I next to my shack with fruit?"** → Drop it off (score points).
2. **"Am I standing on a tree that has fruit?"** → Pick the fruit.
3. **"Is there a tree with fruit somewhere?"** → Walk towards the best one.
4. **"Nothing to do."** → Wait.

---

## Algorithms used

### BFS (Breadth-First Search) for pathfinding
- **What it does:** From the troll's position, it "ripples out" in all four directions, one cell at a time, until it reaches the target tree.
- **Why BFS?** It's the simplest algorithm that guarantees the **shortest path** on a grid where every step costs the same (1 move).
- **Impact:** The troll always takes the most efficient route — no wasted steps. On a 16×8 grid the search is instant (< 1 ms).

### Greedy tree selection
- **What it does:** Score each tree as `fruits × 100 - distance`. Pick the highest score.
- **Why greedy?** Simple, fast, and good enough for L1 where there's only 1 troll and few trees.
- **Impact:** The troll goes to trees with many fruits that are close by. It avoids walking across the whole map for a single plum.

### Priority-chain decision making
- **What it does:** Hard-coded if/else chain: DROP > HARVEST > MOVE > WAIT.
- **Why?** Drop is the only way to score. Harvest fills your carry. Move finds new trees. Wait is the fallback.
- **Impact:** Predictable, easy to debug, no weird behaviour.

### Reservation system
- **What it does:** Once a tree is chosen, it's "reserved" so other trolls don't pick the same one.
- **Why?** Prevents two trolls blocking each other next to the same tree.
- **Impact:** Better resource distribution. (Not critical with 1 troll, but ready for future ligues.)

---

## What could go wrong?

- **Troll leaves shack and can't return** — The game says a troll can't step back on the shack cell after leaving. So if the troll wanders far, DROP becomes impossible. We need to stay close, or eventually train new trolls.
- **Tree has no fruit** — The bot skips it, but doesn't check cooldown. Might walk to a tree that's about to fruit but currently empty.
- **Only one troll** — Can only carry ~3-5 fruits per trip. Later ligues let us train more.

---

## Summary for non-coders

| Concept | Plain English |
|---------|--------------|
| BFS | Like dropping ink on paper — the stain spreads evenly until it hits the target. Shortest path guaranteed. |
| Greedy | Always pick the best option right now, don't plan ahead. |
| Priority chain | "If A, do A. Otherwise if B, do B. Else C." — like a to-do list. |
| Reservation | "I call dibs on this tree." Stops two trolls chasing the same fruit. |
