# Bronze League - Bot Logic

## New features vs Ligue 2
| Feature | L2 | Bronze |
|---------|----|--------|
| Game length | 100 turns | **300 turns** |
| Resources | 4 fruits + iron/wood (always 0) | **iron & wood can be >0** |
| Actions | MOVE/HARVEST/DROP/PLANT/TRAIN/WAIT | +**CHOP**, +**MINE**, +**PICK** |
| Grid | GRASS + SHACK | +**WATER**(~), **ROCK**(#), **IRON**(+) |
| Scoring | Fruit=1pt | Fruit=1pt, **WOOD=4pt**, Iron=0pt |
| Training | 3 attributes (P,L,A) | **4 attributes** (+chopPower trained with IRON) |
| Troll carry | 4 items | **6 items** (+iron, +wood) |
| Tree cooldown | uniform | **varies by type**, faster near water |

---

## Priority chain

| Priority | Condition | Action |
|----------|-----------|--------|
| 1 | Carrying AND adjacent to shack | `DROP` |
| 2 | On tree with fruit AND free capacity | `HARVEST` |
| 3 | On tree with health>0, fruit=0, empty-handed AND chopPower>0 | `CHOP` |
| 4 | Carrying AND eligible (surplus≥8, ≤3 from shack, turn<100, <5 plants) | `PLANT` |
| 5 | Carrying | `MOVE` to shack |
| 6 | Near IRON AND need iron (inv < myCount+3) AND chopPower>0 | `MINE` |
| 7 | Empty-handed: best tree (fruit harvest > chop wood) | `MOVE` to tree |
| 8 | Empty-handed: anticipate size-4 tree with cooldown≤5 | `MOVE` to tree |
| 9 | Empty-handed: need iron AND chopPower>0 | `MOVE` near IRON |

---

## Tree scoring

**Harvest score** (tree.fruits > 0):
```
goCost = ceil(dist(troll, tree) / speed)
returnCost = ceil(dist(tree, shack) / speed)
needBonus[type] = max(0, deficit) × 50
score = fruits × 100 - goCost - returnCost/2 + needBonus[type]
```

**Chop score** (tree.health > 0, tree.fruits == 0, cooldown > 3):
```
chopTurns = ceil(health / chopPower)
woodValue = tree.size × 4 × 100  (4pt per wood, score is centi-points)
score = woodValue - goCost - chopTurns × 10
```

---

## Training options

| Priority | move | carry | power | chop | margin | Resources needed (n=1) |
|----------|------|-------|-------|------|--------|------------------------|
| 1 | 2 | 2 | 1 | 1 | +2 | 7P + 7L + 4A + 4I |
| 2 | 1 | 2 | 1 | 1 | +1 | 3P + 6L + 3A + 3I |
| 3 | 2 | 1 | 1 | 1 | +1 | 6P + 3L + 3A + 3I |
| 4 | 1 | 1 | 1 | 1 | +1 | 3P + 3L + 3A + 3I |
| 5 | 1 | 1 | 1 | 0 | +1 | 3P + 3L + 3A + 2I |

### When to train
- 2nd troll: before turn **290**
- 3rd troll: before turn **200**
- 4th troll: before turn **100**
- Max 3 trained (4 total)

---

## Key improvements

| # | Change | Why |
|---|--------|-----|
| 1 | `carry[6]` instead of `carry[4]` | Tracks iron+wood inventory |
| 2 | `CHOP` action | Wood is 4pt per unit — main score source |
| 3 | `MINE` action | Iron needed to train chopPower |
| 4 | Chop-only non-fruiting trees (cooldown>3) | Preserves productive trees |
| 5 | Move toward IRON when iron is low | Ensures steady iron income |
| 6 | 300-turn game: plant window 100, training 290/200/100 | Longer game rewards investment |
| 7 | Need bonus *50 per missing fruit | Drives exploration of scarce types |

---

## Key numbers

| Resource | Points | Source | Speed |
|----------|--------|--------|-------|
| Fruit | 1 | HARVEST | 1/turn |
| Wood | 4 | CHOP (tree dies) | health/chopPower turns |
| Iron | 0 | MINE (adjacent to IRON) | chopPower/turn |

### Tree cooldown (turns to fruit)

| Type | Normal | Near water |
|------|--------|------------|
| PLUM | 8 | 3 |
| LEMON | 8 | 3 |
| APPLE | 9 | 2 |
| BANANA | 6 | 4 |

### Tree health by size

| Size | PLUM/LEMON | APPLE | BANANA |
|------|-----------|-------|--------|
| 1 | 6 | 11 | 3 |
| 2 | 8 | 14 | 4 |
| 3 | 10 | 17 | 5 |
| 4 | 12 | 20 | 6 |

---

## Action order (per turn)
1. MOVE
2. HARVEST
3. PLANT
4. CHOP
5. PICK
6. TRAIN
7. DROP
8. MINE
9. Trees grow
