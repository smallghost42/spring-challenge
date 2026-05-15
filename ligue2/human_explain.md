# Human explanation — Ligue 2 bot

## What changed from Ligue 1?

L1: 1 troll, harvest 1 fruit, run home, repeat.
L2: Up to 5 trolls, harvest until full, plant trees, anticipate future fruit.

---

## The BIG harvest fix

**Problem:** Harvest 1 fruit → immediately return to shack. With carry capacity 2, we wasted half our potential.

**Fix:** Trolls now stay on the tree and keep harvesting until their backpack is full OR the tree is empty. Only then do they return to shack. Throughput doubled.

---

## Why we couldn't train (critical)

Our bot had 1 troll for the entire game. The boss had 2 from turn 1. Here's why:

**Bad logic:** Training required 7 plums + 7 lemons + 4 apples. With 1 troll harvesting 1 fruit per 6 turns, that's **impossible in 100 turns**.

**Worse:** The bot got stuck in a **harvest rut** — it kept going back to the same banana tree over and over. It never collected lemons or apples, so it could never afford training.

**Fix 1 — Adaptive training:** The bot tries 6 stat combos from best to cheapest. The minimum option (speed=1, carry=1, power=1) needs only **3 of each fruit** — achievable in ~10 turns.

**Fix 2 — Need-based tree selection:** When we're short on lemons, lemon trees get a huge score bonus (+50 per missing fruit). The bot will walk past a banana tree to find lemons. This forces diverse fruit collection.

---

## All bugs fixed in this update

| Bug | What was wrong | What changed |
|-----|---------------|--------------|
| **Shack in pathfinding** | BFS could route trolls THROUGH their own shack (can't re-enter after leaving) | Shack marked as non-walkable in grid |
| **WAIT without ID** | Per-troll `WAIT` mixed with real commands — ambiguous | Only output WAIT if ALL trolls idle |
| **Over-conservative planting** | Wouldn't plant if ANY tree within 1 cell (not just same cell) | Only check same cell |
| **Planting needed fruits** | Planted fruits we were low on (conflicts with training) | Only plant surplus (≥4 in inventory) |
| **1 troll per tree** | Reservation prevented co-harvesting (rules allow 2) | Map counter instead of set, max 2 |
| **Idle when no fruit** | Trolls WAIT when all trees have 0 fruit | Move toward size-4 trees about to produce (cooldown ≤5) |
| **Harvest rut** | Always returned to closest tree | Need-based bonus forces exploration |

---

## When does the bot plant trees?

Only when ALL these are true:
- We have ≥2 trolls (training first)
- We have ≥4 of that fruit in inventory (surplus)
- We're ≥3 cells from the shack (don't waste a DROP spot)
- No tree already exists on this exact cell

The fruit comes from the troll's carry (already harvested), not from inventory.

---

## Anticipatory movement

When no trees have fruit, instead of WAITing, the bot:
1. Finds the closest tree that's **size 4** (max size = produces fruit)
2. With **cooldown ≤ 5** (will fruit within 5 turns)
3. Moves toward it

The troll arrives right around the time fruit spawns — no wasted turns.

---

## Match summary (why we lost vs BOSS 2)

| Factor | Us | Boss | Impact |
|--------|-----|------|--------|
| 2nd troll | Never trained (impossible cost) | Turn 1 (min stats) | Boss 2x throughput all game |
| Fruit diversity | Only bananas + plums | All types | Could train, we couldn't |
| Harvest efficiency | 1 fruit/trip | ~full capacity | 2x trips for same yield |
| Tree planting | 0 | 4 trees | Boss created future income |
