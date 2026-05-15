# Ligue 2 - Bot Logic

## New features vs Ligue 1
| Feature | L1 | L2 |
|---------|----|----|
| Troll count | 1 fixed | Train up to 6 |
| Actions | MOVE/HARVEST/DROP/WAIT | +TRAIN, +PLANT |
| Inventory tracking | ignored | read & used for TRAIN cost |
| Cost system | none | `cost = trollCount + attribute²` |

---

## Inventory tracking
```cpp
int myInv[4]; // [PLUM, LEMON, APPLE, BANANA]
```
Read every turn from the first 2 input lines. Used to decide TRAIN affordability and PLANT eligibility.

---

## TRAIN logic — Adaptive model

### Philosophy
Try **6 options ranked by quality**, pick the first affordable one. Adapts to whatever fruit we've accumulated.

### Option list (best → cheapest)

| Priority | move | carry | power | margin | Cost (n=1) | Why this |
|----------|------|-------|-------|--------|-------------|----------|
| 1 | 2 | 2 | 1 | +3 | 5P + 5L + 2A | Balanced all-rounder |
| 2 | 1 | 2 | 1 | +1 | 2P + 5L + 2A | Cheap carry (carry=2, margin=1) |
| 3 | 2 | 1 | 1 | +1 | 5P + 2L + 2A | Cheap speed (speed=2, margin=1) |
| 4 | 1 | 3 | 1 | +3 | 2P + 10L + 2A | Bulk hauler |
| 5 | 3 | 1 | 1 | +3 | 10P + 2L + 2A | Fast scout |

**No minimum stat training** — if none of the above are affordable, skip training.
The fruit saved from not training yields more points than a slow {1,1,1} troll could earn.

### When to train
- 2nd troll: before turn **95** (always worth it)
- 3rd troll: before turn **40**
- 4th troll: before turn **25**
- Max 3 trained (4 total)

### Why carry first
A troll with carry=2 (speed=1) has *higher throughput* than one with speed=2 (carry=1):

| Troll | Harvest turns | Return turns | Cycle | Fruits | Rate |
|-------|:---:|:---:|:---:|:---:|:---:|
| speed=2, carry=1 | ceil(D/2) | ceil(D/2) + harvest + drop | 3 + 2×ceil(D/2) | 1 | 1/(3+2×ceil(D/2)) |
| speed=1, carry=2 | D | D + 2×harvest + drop | 3 + 2D | 2 | 2/(3+2D) |

For D=6: speed=2 gives 1 fruit / 9 turns (0.111), carry=2 gives 2 fruits / 15 turns (0.133). **Carry=2 wins for any D > 2.**

---

## Troll priority chain

| Priority | Condition | Action |
|----------|-----------|--------|
| 1 | Carrying AND adjacent to shack | `DROP` |
| 2 | On tree with fruit AND free capacity > 0 | `HARVEST` (fill up) |
| 3 | Carrying fruit AND inventory >= 8 of that type AND ≤3 cells from shack AND no tree on cell | `PLANT` |
| 4 | Carrying fruit (can't harvest more) | `MOVE` to shack (adjacent cell) |
| 5 | Empty-handed, tree with fruit found | `MOVE` to best tree (round-trip cost) |
| 6 | Empty-handed, no fruit trees → tree size=4 with cooldown ≤5 found | `MOVE` anticipatory |
| 7 | Nothing to do | skip (no WAIT in list) |

### Key changes from previous version

**Bug 1 — Shack walkable in BFS:** Trolls can't re-enter their own shack after leaving. BFS could route through the shack cell. Fixed: own shack is now non-walkable (`grid[y][x].walkable = (c == '.' || c == '1')`).

**Bug 2 — WAIT without id:** Per-troll `WAIT` was pushed into the action list alongside real commands. WAIT is a team-wide command with no troll ID — mixing it with MOVE/HARVEST is ambiguous. Fixed: only output WAIT when ALL actions are empty.

**Bug 3 — Plant distance check:** `dist <= 1` was blocking planting if ANY tree existed within 1 cell (too conservative). Fixed: only check same cell (`dist == 0`).

**Bug 4 — Plant surplus vs deficit:** Planted fruits we were low on (`myInv <= 5`), which conflicted with training needs. Fixed: only plant surplus fruits (`myInv >= 4`).

**Improvement 1 — Co-harvesting:** Reservation changed from `set` (1 troll per tree) to `map<int,int>` (up to 2 trolls per tree). Rules allow two trolls to harvest the same tree simultaneously, with the last fruit duplicating.

**Improvement 2 — Anticipatory movement:** When no trees have fruit, find the closest size-4 tree with cooldown ≤ 5 and move toward it. Trolls arrive right as fruit spawns instead of idling.

**Improvement 3 — Training deficit bonus:** Tree scores get +50 per missing fruit needed for training. Forces exploration of under-visited fruit types.

---

## Need-based tree scoring

### Before
```
score = fruits × 100 - distance
```
→ Bot always went back to the closest replenishing tree (harvest rut).

### After
```
needBonus[type] = max(0, deficit) × 50
goCost = ceil(dist(troll, tree) / speed)
returnCost = ceil(dist(tree, shack) / speed)
score = fruits × 100 - goCost - returnCost + needBonus[type]
```
→ Round-trip cost: prioritize trees close to shack for faster cycles.
→ Deficit bonus forces exploration of missing fruit types needed for training.

---

## Multi-troll coordination
- `map<int,int> reservedTrees` — keyed by `x*1000+y`, value = number of trolls assigned (max 2)
- Allows co-harvesting while preventing 3+ trolls clumping

---

## PLANT logic (priority 3)
- Only with **≥2 trolls** (focus on training first)
- Only with **surplus fruit** (myInv[ft] ≥ 8)
- Only if **no tree on this cell**
- Only at **≤3 cells from shack** (nearby = shorter harvest trips)
- Only **before turn 25** (trees need time to grow & fruit)
- **Max 3 plants per game** (limited ROI in 100-turn game)
- Uses fruit from troll's carry (not inventory) — the fruit was already harvested

---

## Improvements (v2)

| # | Change | Why |
|---|--------|-----|
| 1 | Skip minimum stat training | {1,1,1} troll costs fruit but earns less than saving it for points |
| 2 | Reduced margins (2→1) for {1,2,1}/{2,1,1} | Ensures training happens in low-inventory starts |
| 3 | Populate `targetedCells` with troll positions every turn | Prevents "target blocked" collisions at tree cells |
| 4 | Check `targetedCells` in tree scoring loop | No two trolls target the same cell |
| 5 | Return cost halved in scoring (`go - return/2`) | Stops all trolls converging on the sole nearby tree |
| 6 | Plant ≤3 cells from shack, max 3, cutoff turn 25 | Short harvest trips, limited ROI in 100 turns |
| 7 | Tighter training limits (3rd at 40, 4th at 25) | Each troll needs ~60+ turns to pay off |

## Limitations & upgrade path
| Issue | Future fix |
|-------|-----------|
| No PICK for targeted planting | Add PICK for newly spawned trolls |
| No opponent awareness | Tree stealing, blocking |
| No cooldown-aware harvest scheduling | Only visit trees when fruit is ready |
| No troll specialization | Dedicated harvesters / planters |
| 100→300 turn limit | L3 extends game length |
