#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <queue>
#include <set>
#include <map>
#include <cstdlib>
#include <climits>

using namespace std;

struct Cell {
    char type;
    bool walkable;
};

struct Tree {
    string type;
    int x, y, size, health, fruits, cooldown;
};

struct Troll {
    int id, player, x, y;
    int movementSpeed, carryCapacity, harvestPower, chopPower;
    int carry[6];
};

int W, H;
vector<vector<Cell>> grid;
int shackX, shackY;
int plantsThisGame = 0;

map<pair<int,int>, int> blockedCells;
map<int,pair<int,int>> prevPos, prevTarget;

vector<pair<int,int>> ironCells;
vector<pair<int,int>> ironApproach;
vector<pair<int,int>> shackAdjCells;
vector<pair<int,int>> waterCells;
vector<pair<int,int>> plantZones;

int dist(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

bool isAdjacentToShack(int x, int y) {
    return (abs(x - shackX) + abs(y - shackY)) == 1;
}

bool isAdjacentToIron(int x, int y) {
    if (x > 0 && grid[y][x-1].type == '+') return true;
    if (x < W-1 && grid[y][x+1].type == '+') return true;
    if (y > 0 && grid[y-1][x].type == '+') return true;
    if (y < H-1 && grid[y+1][x].type == '+') return true;
    return false;
}

bool isNearWater(int x, int y) {
    if (x > 0 && grid[y][x-1].type == '~') return true;
    if (x < W-1 && grid[y][x+1].type == '~') return true;
    if (y > 0 && grid[y-1][x].type == '~') return true;
    if (y < H-1 && grid[y+1][x].type == '~') return true;
    return false;
}

vector<pair<int,int>> getPath(int sx, int sy, int tx, int ty) {
    vector<pair<int,int>> emptyPath;
    if (sx == tx && sy == ty) return emptyPath;
    if (!grid[ty][tx].walkable) return emptyPath;

    vector<vector<pair<int,int>>> parent(H, vector<pair<int,int>>(W, {-1,-1}));
    vector<vector<bool>> visited(H, vector<bool>(W, false));
    queue<pair<int,int>> q;

    q.push({sx, sy});
    visited[sy][sx] = true;
    parent[sy][sx] = {sx, sy};

    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};

    while (!q.empty()) {
        pair<int,int> cur = q.front(); q.pop();
        int cx = cur.first, cy = cur.second;
        if (cx == tx && cy == ty) break;
        for (int d = 0; d < 4; d++) {
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            if (nx >= 0 && nx < W && ny >= 0 && ny < H && !visited[ny][nx] && grid[ny][nx].walkable) {
                visited[ny][nx] = true;
                parent[ny][nx] = {cx, cy};
                q.push({nx, ny});
            }
        }
    }

    if (!visited[ty][tx]) return emptyPath;

    vector<pair<int,int>> path;
    int cx = tx, cy = ty;
    while (!(cx == sx && cy == sy)) {
        path.push_back({cx, cy});
        pair<int,int> p = parent[cy][cx];
        cx = p.first; cy = p.second;
    }
    reverse(path.begin(), path.end());
    return path;
}

void doMove(int id, int fx, int fy, int tx, int ty, int speed, vector<string>& actions) {
    auto path = getPath(fx, fy, tx, ty);
    if (!path.empty()) {
        int steps = min(speed, (int)path.size());
        int nx = path[steps - 1].first;
        int ny = path[steps - 1].second;
        prevTarget[id] = {nx, ny};
        actions.push_back("MOVE " + to_string(id) + " " + to_string(nx) + " " + to_string(ny));
    }
}

int main() {
    cin >> W >> H; cin.ignore();

    grid.assign(H, vector<Cell>(W));

    for (int y = 0; y < H; y++) {
        string line;
        getline(cin, line);
        for (int x = 0; x < W; x++) {
            char c = line[x];
            grid[y][x].type = c;
            grid[y][x].walkable = (c == '.' || c == '1');
            if (c == '0') {
                shackX = x;
                shackY = y;
            }
            if (c == '~') waterCells.push_back({x, y});
        }
    }

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            if (grid[y][x].type == '+') {
                ironCells.push_back({x, y});
                int dx[] = {0, 1, 0, -1};
                int dy[] = {-1, 0, 1, 0};
                for (int d = 0; d < 4; d++) {
                    int nx = x + dx[d];
                    int ny = y + dy[d];
                    if (nx >= 0 && nx < W && ny >= 0 && ny < H && grid[ny][nx].walkable) {
                        ironApproach.push_back({nx, ny});
                    }
                }
            }
        }
    }

    for (int d = 0; d < 4; d++) {
        int dx[] = {0, 1, 0, -1};
        int dy[] = {-1, 0, 1, 0};
        int nx = shackX + dx[d];
        int ny = shackY + dy[d];
        if (nx >= 0 && nx < W && ny >= 0 && ny < H && grid[ny][nx].walkable) {
            shackAdjCells.push_back({nx, ny});
        }
    }

    {
        set<pair<int,int>> seen;
        for (int dy = -3; dy <= 3; dy++) {
            for (int dx = -3; dx <= 3; dx++) {
                int nx = shackX + dx, ny = shackY + dy;
                if (abs(dx) + abs(dy) > 3) continue;
                if (nx < 0 || nx >= W || ny < 0 || ny >= H) continue;
                if (!grid[ny][nx].walkable) continue;
                if (seen.find({nx, ny}) == seen.end()) {
                    seen.insert({nx, ny});
                    plantZones.push_back({nx, ny});
                }
            }
        }
        sort(plantZones.begin(), plantZones.end(), [](const pair<int,int> &a, const pair<int,int> &b) {
            int da = (abs(a.first - shackX) + abs(a.second - shackY));
            int db = (abs(b.first - shackX) + abs(b.second - shackY));
            bool wa = isNearWater(a.first, a.second);
            bool wb = isNearWater(b.first, b.second);
            if (wa != wb) return wa > wb;
            return da < db;
        });
    }

    int turn = 0;
    while (true) {
        turn++;
        int myInv[6];
        for (int i = 0; i < 2; i++) {
            int p, l, a, b, iron, wood;
            cin >> p >> l >> a >> b >> iron >> wood; cin.ignore();
            if (i == 0) { myInv[0]=p; myInv[1]=l; myInv[2]=a; myInv[3]=b; myInv[4]=iron; myInv[5]=wood; }
        }

        int treeCount;
        cin >> treeCount; cin.ignore();
        vector<Tree> trees(treeCount);
        for (int i = 0; i < treeCount; i++) {
            cin >> trees[i].type >> trees[i].x >> trees[i].y
                >> trees[i].size >> trees[i].health
                >> trees[i].fruits >> trees[i].cooldown;
            cin.ignore();
        }

        int trollCount;
        cin >> trollCount; cin.ignore();
        vector<Troll> trolls(trollCount);
        vector<Troll> myTrolls;
        for (int i = 0; i < trollCount; i++) {
            int id, player, x, y, ms, cc, hp, chop;
            int cp, cl, ca, cb, ci, cw;
            cin >> id >> player >> x >> y >> ms >> cc >> hp >> chop
                >> cp >> cl >> ca >> cb >> ci >> cw;
            cin.ignore();
            trolls[i] = {id, player, x, y, ms, cc, hp, chop, {cp, cl, ca, cb, ci, cw}};
            if (player == 0) myTrolls.push_back(trolls[i]);
        }

        vector<string> actions;
        map<int,int> reservedTrees;
        set<int> targetedCells;

        for (auto it = blockedCells.begin(); it != blockedCells.end();) {
            if (--it->second <= 0) blockedCells.erase(it++);
            else ++it;
        }

        for (auto &t : myTrolls) {
            auto pp = prevPos.find(t.id);
            auto pt = prevTarget.find(t.id);
            if (pp != prevPos.end() && pt != prevTarget.end()) {
                if (pp->second.first == t.x && pp->second.second == t.y) {
                    pair<int,int> target = pt->second;
                    if (target.first != t.x || target.second != t.y)
                        blockedCells[target] = 1;
                }
            }
            prevPos[t.id] = {t.x, t.y};
        }

        int myCount = (int)myTrolls.size();
        string types[4] = {"PLUM", "LEMON", "APPLE", "BANANA"};
        bool pickedThisTurn = false;

        set<pair<int,int>> takenPlantSpots;
        for (auto &tree : trees)
            takenPlantSpots.insert({tree.x, tree.y});

        for (auto &troll : myTrolls) {
            int totalCarry = 0;
            for (int k = 0; k < 6; k++) totalCarry += troll.carry[k];
            int freeCapacity = troll.carryCapacity - totalCarry;

            if (totalCarry > 0) {
                bool onlyFruits = true;
                for (int k = 4; k < 6; k++) if (troll.carry[k] > 0) onlyFruits = false;

                bool shouldPlant = false;
                int plantType = -1;
                if (onlyFruits && myCount >= 2 && turn < 100 && plantsThisGame < 12) {
                    int plantOrder[3] = {0, 1, 2};
                    int plantNeed[3];
                    int trainTarget = myCount + 2;
                    for (int f = 0; f < 3; f++) plantNeed[f] = trainTarget - myInv[f];
                    for (int a = 0; a < 2; a++)
                        for (int b = a+1; b < 3; b++)
                            if (plantNeed[plantOrder[a]] > plantNeed[plantOrder[b]])
                                swap(plantOrder[a], plantOrder[b]);
                    for (int pi = 0; pi < 3; pi++) {
                        int ft = plantOrder[pi];
                        int surplus = myInv[ft] - (myCount + 2);
                        if (surplus < 2) continue;
                        if (troll.carry[ft] > 0 && dist(troll.x, troll.y, shackX, shackY) <= 3) {
                            if (!grid[troll.y][troll.x].walkable) continue;
                            if (takenPlantSpots.count({troll.x, troll.y})) continue;
                            if (targetedCells.count(troll.x * 1000 + troll.y)) continue;
                            shouldPlant = true;
                            plantType = ft;
                            break;
                        }
                    }
                }
                if (shouldPlant) {
                    prevTarget.erase(troll.id);
                    takenPlantSpots.insert({troll.x, troll.y});
                    actions.push_back("PLANT " + to_string(troll.id) + " " + types[plantType]);
                    plantsThisGame++;
                    continue;
                }

                if (isAdjacentToShack(troll.x, troll.y)) {
                    prevTarget.erase(troll.id);
                    actions.push_back("DROP " + to_string(troll.id));
                    continue;
                }

                int bestD = 999999;
                int tx = -1, ty = -1;
                for (auto &cell : shackAdjCells) {
                    int ck = cell.first * 1000 + cell.second;
                    if (targetedCells.count(ck)) continue;
                    int dd = dist(troll.x, troll.y, cell.first, cell.second);
                    if (dd < bestD) { bestD = dd; tx = cell.first; ty = cell.second; }
                }
                if (tx == -1 && !shackAdjCells.empty()) {
                    int ck = shackAdjCells[0].first * 1000 + shackAdjCells[0].second;
                    if (!targetedCells.count(ck)) {
                        tx = shackAdjCells[0].first; ty = shackAdjCells[0].second;
                    }
                }
                if (tx == -1) {
                    for (int d = 0; d < 4; d++) {
                        int dx[] = {0, 1, 0, -1};
                        int dy[] = {-1, 0, 1, 0};
                        int nx = shackX + dx[d], ny = shackY + dy[d];
                        int ck = nx * 1000 + ny;
                        if (nx >= 0 && nx < W && ny >= 0 && ny < H && grid[ny][nx].walkable && !targetedCells.count(ck)) {
                            tx = nx; ty = ny; break;
                        }
                    }
                }
                if (tx != -1) {
                    targetedCells.insert(tx * 1000 + ty);
                    doMove(troll.id, troll.x, troll.y, tx, ty, troll.movementSpeed, actions);
                }
                continue;
            }

            Tree *treeHere = nullptr;
            for (auto &tree : trees) {
                if (tree.x == troll.x && tree.y == troll.y && tree.fruits > 0) {
                    treeHere = &tree;
                    break;
                }
            }
            if (treeHere && freeCapacity > 0) {
                prevTarget.erase(troll.id);
                actions.push_back("HARVEST " + to_string(troll.id));
                continue;
            }

            Tree *treeToChop = nullptr;
            for (auto &tree : trees) {
                if (tree.x == troll.x && tree.y == troll.y && tree.health > 0 && tree.fruits == 0 && troll.chopPower > 0 && totalCarry == 0) {
                    treeToChop = &tree;
                    break;
                }
            }
            if (treeToChop && freeCapacity > 0) {
                int woodFromTree = treeToChop->size;
                int woodHere = min(woodFromTree, freeCapacity);
                if (woodHere > 0) {
                    prevTarget.erase(troll.id);
                    actions.push_back("CHOP " + to_string(troll.id));
                    continue;
                }
            }

            {
                int pickOrder[3] = {0, 1, 2};
                int targets[3] = {myCount+2, myCount+2, myCount+2};
                for (int a = 0; a < 2; a++)
                    for (int b = a+1; b < 3; b++)
                        if (myInv[pickOrder[a]] - targets[pickOrder[a]] > myInv[pickOrder[b]] - targets[pickOrder[b]])
                            swap(pickOrder[a], pickOrder[b]);

                bool didPick = false;
                if (totalCarry == 0 && isAdjacentToShack(troll.x, troll.y) && myCount >= 2 && turn < 100 && plantsThisGame < 12 && !pickedThisTurn) {
                    int minInv = myCount + 2;
                    for (int pi = 0; pi < 3 && !didPick; pi++) {
                        int ft = pickOrder[pi];
                        if (myInv[ft] >= minInv) {
                            for (auto &pz : plantZones) {
                                if (takenPlantSpots.count(pz)) continue;
                                if (targetedCells.count(pz.first * 1000 + pz.second)) continue;
                                prevTarget.erase(troll.id);
                                actions.push_back("PICK " + to_string(troll.id) + " " + types[ft]);
                                didPick = true;
                                pickedThisTurn = true;
                                break;
                            }
                        }
                    }
                }
                if (didPick) continue;
            }

            int needBonus[4] = {0,0,0,0};
            if (myCount < 12) {
                int targets[4] = {myCount+2, myCount+2, myCount+2, 0};
                for (int f = 0; f < 4; f++)
                    needBonus[f] = max(0, targets[f] - myInv[f]) * 50;
            }

            int bestScore = -1;
            int bestTx = -1, bestTy = -1;
            string bestAction;

            int ironNeeded = max(0, (myCount + 2) - myInv[4]);
            if (ironNeeded > 0 && isAdjacentToIron(troll.x, troll.y) && troll.chopPower > 0 && freeCapacity > 0) {
                bestScore = 3000 + ironNeeded * 500;
                bestAction = "MINE";
            }

            for (auto &tree : trees) {
                if (reservedTrees[tree.x * 1000 + tree.y] >= 1) continue;
                if (!grid[tree.y][tree.x].walkable) continue;
                if (targetedCells.count(tree.x * 1000 + tree.y)) continue;
                if (blockedCells.count({tree.x, tree.y})) continue;

                int d = dist(troll.x, troll.y, tree.x, tree.y);
                int speed = max(1, troll.movementSpeed);
                int goCost = (d + speed - 1) / speed;

                if (tree.fruits > 0) {
                    int returnDist = dist(tree.x, tree.y, shackX, shackY);
                    int returnCost = (returnDist + speed - 1) / speed;
                    int typeIdx = (tree.type == "PLUM") ? 0 : (tree.type == "LEMON") ? 1 : (tree.type == "APPLE") ? 2 : 3;
                    int score = tree.fruits * 100 - goCost - returnCost / 2 + needBonus[typeIdx];
                    if (d <= speed && needBonus[typeIdx] > 0) score = max(score, 5000);
                    if (score > bestScore) {
                        bestScore = score;
                        bestTx = tree.x; bestTy = tree.y;
                        bestAction = "MOVE";
                    }
                }

                if (tree.health > 0 && tree.fruits == 0 && troll.chopPower > 0 && tree.cooldown > 3) {
                    int chopTurns = (tree.health + troll.chopPower - 1) / troll.chopPower;
                    int woodValue = tree.size * 400;
                    int score = woodValue - goCost - chopTurns * 10;
                    if (tree.size >= 3) score += 150;
                    if (tree.size >= 4) score += 200;
                    if (score > bestScore) {
                        bestScore = score;
                        bestTx = tree.x; bestTy = tree.y;
                        bestAction = "MOVE";
                    }
                }
            }

            if (ironNeeded > 0 && troll.chopPower > 0 && !isAdjacentToIron(troll.x, troll.y)) {
                for (auto &cell : ironApproach) {
                    int ck = cell.first * 1000 + cell.second;
                    if (targetedCells.count(ck)) continue;
                    if (blockedCells.count({cell.first, cell.second})) continue;
                    int d2 = dist(troll.x, troll.y, cell.first, cell.second);
                    int speed = max(1, troll.movementSpeed);
                    int goCost = (d2 + speed - 1) / speed;
                    int score = 500 - goCost + ironNeeded * 200;
                    if (score > bestScore) {
                        bestScore = score;
                        bestTx = cell.first; bestTy = cell.second;
                        bestAction = "MOVE";
                    }
                }
            }

            if (bestAction == "MINE") {
                prevTarget.erase(troll.id);
                actions.push_back("MINE " + to_string(troll.id));
            }
            else if (bestAction == "MOVE") {
                int ck = bestTx * 1000 + bestTy;
                targetedCells.insert(ck);
                doMove(troll.id, troll.x, troll.y, bestTx, bestTy, troll.movementSpeed, actions);
            }
            else if (totalCarry == 0 && isAdjacentToShack(troll.x, troll.y) && myCount >= 2 && turn < 100 && plantsThisGame < 12) {
                bool didPick = false;
                {
                    int pickOrder[3] = {0, 1, 2};
                    int pickNeed[3];
                    int pickTarget = myCount + 2;
                    for (int f = 0; f < 3; f++) pickNeed[f] = pickTarget - myInv[f];
                    for (int a = 0; a < 2; a++)
                        for (int b = a+1; b < 3; b++)
                            if (pickNeed[pickOrder[a]] > pickNeed[pickOrder[b]])
                                swap(pickOrder[a], pickOrder[b]);
                    for (int pi = 0; pi < 3 && !didPick; pi++) {
                        int ft = pickOrder[pi];
                        int surplus = myInv[ft] - (myCount + 2);
                        if (surplus >= 2) {
                            for (auto &pz : plantZones) {
                                if (takenPlantSpots.count(pz)) continue;
                                if (targetedCells.count(pz.first * 1000 + pz.second)) continue;
                                prevTarget.erase(troll.id);
                                actions.push_back("PICK " + to_string(troll.id) + " " + types[ft]);
                                didPick = true;
                                break;
                            }
                        }
                    }
                }
                if (didPick) { pickedThisTurn = true; continue; }
                if (!didPick) {
                    Tree *bestFuture = nullptr;
                    int bestFutureScore = -1;
                    for (auto &tree : trees) {
                        if (tree.size < 4) continue;
                        if (tree.cooldown > 5) continue;
                        if (targetedCells.count(tree.x * 1000 + tree.y)) continue;
                        int d = dist(troll.x, troll.y, tree.x, tree.y);
                        int turnCost = (d + max(1, troll.movementSpeed) - 1) / max(1, troll.movementSpeed);
                        int score = 100 - tree.cooldown * 10 - turnCost;
                        if (score > bestFutureScore) {
                            bestFutureScore = score;
                            bestFuture = &tree;
                        }
                    }
                    if (bestFuture) {
                        int fk = bestFuture->x * 1000 + bestFuture->y;
                        if (!targetedCells.count(fk)) {
                            targetedCells.insert(fk);
                            doMove(troll.id, troll.x, troll.y, bestFuture->x, bestFuture->y, troll.movementSpeed, actions);
                        }
                    }
                }
            }
            else {
                Tree *bestFuture = nullptr;
                int bestFutureScore = -1;
                for (auto &tree : trees) {
                    if (tree.size < 4) continue;
                    if (tree.cooldown > 5) continue;
                    if (targetedCells.count(tree.x * 1000 + tree.y)) continue;
                    int d = dist(troll.x, troll.y, tree.x, tree.y);
                    int turnCost = (d + max(1, troll.movementSpeed) - 1) / max(1, troll.movementSpeed);
                    int score = 100 - tree.cooldown * 10 - turnCost;
                    if (score > bestFutureScore) {
                        bestFutureScore = score;
                        bestFuture = &tree;
                    }
                }
                if (bestFuture) {
                    int fk = bestFuture->x * 1000 + bestFuture->y;
                    if (!targetedCells.count(fk)) {
                        targetedCells.insert(fk);
                        doMove(troll.id, troll.x, troll.y, bestFuture->x, bestFuture->y, troll.movementSpeed, actions);
                    }
                }
            }
        }

        const int maxTrolls = 12;
        if (myCount < maxTrolls) {
            int turnLimit = 290;
            if (turn < turnLimit) {
                struct Opt { int m, c, p, ch, margin; };
                Opt opts[6] = {
                    {2, 2, 1, 1, 2},
                    {1, 2, 1, 1, 1},
                    {2, 1, 1, 1, 1},
                    {1, 1, 1, 1, 1},
                    {1, 1, 1, 0, 1},
                    {1, 1, 1, 0, 0},
                };
                for (int i = 0; i < 6; i++) {
                    int cp = myCount + opts[i].m * opts[i].m;
                    int cl = myCount + opts[i].c * opts[i].c;
                    int ca = myCount + opts[i].p * opts[i].p;
                    int ci = myCount + opts[i].ch * opts[i].ch;
                    if (myInv[0] >= cp + opts[i].margin &&
                        myInv[1] >= cl + opts[i].margin &&
                        myInv[2] >= ca + opts[i].margin &&
                        myInv[4] >= ci + opts[i].margin) {
                        actions.push_back("TRAIN " + to_string(opts[i].m) + " " +
                            to_string(opts[i].c) + " " + to_string(opts[i].p) + " " +
                            to_string(opts[i].ch));
                        break;
                    }
                }
            }
        }

        if (actions.empty()) {
            cout << "WAIT" << endl;
        } else {
            string result;
            for (size_t i = 0; i < actions.size(); i++) {
                if (i > 0) result += ";";
                result += actions[i];
            }
            cout << result << endl;
        }
    }

    return 0;
}
