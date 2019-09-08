#include "sokoban.h"

Sokoban::Sokoban() {}

void Sokoban::loadMap(MapVec mapData, int height, int width) {
    mapHeight = height;
    mapWidth = width;
    win = false;
    boxNum = 0;

    gameMap = std::move(mapData);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (gameMap[i][j] == 2) {
                manPos = Pos(i, j);
            }
            if (gameMap[i][j] == 1) {
                boxNum++;
            }
        }
    }
}

// direction: 1:up 2:down 3:left 4:right
void Sokoban::actionByMan(int direction) {
    if (win) return;

    PushMsg pushMsg(manPos, direction);
    updateGameMap(gameMap, pushMsg);
    manPos = pushMsg.pos;

    win = checkTarget(gameMap);
}

void Sokoban::actionByAI() {
    if (win) return;

    searchTargetPath();

    stack<Status> reverseStack;
    while (!statusStack.empty()) {
        reverseStack.push(statusStack.top());
        statusStack.pop();
    }

    while (!reverseStack.empty()) {
        Status cur = reverseStack.top();
        PushMsg msg = cur.pushMsgs[cur.nowMsgIndex - 1];

        bool tag;
        Pos tarPos = msg.pos;
        switch (msg.direction) {
        case 1: tarPos.row++; break;
        case 2: tarPos.row--; break;
        case 3: tarPos.col++; break;
        case 4: tarPos.col--; break;
        }
        msg.pos = tarPos;
        stack<Pos> walkPath = manToTarget(gameMap, tarPos, tag);
        while (!walkPath.empty()) {
            PushMsg newPush = PushMsg(walkPath.top(), 0);
            updateGameMap(gameMap, newPush);
            manPos = newPush.pos;
            walkPath.pop();

            emit sendUpdate();
            Sleep(500);
        }

        reverseStack.pop();
        updateGameMap(gameMap, msg);
        manPos = msg.pos;
        emit sendUpdate();
        Sleep(500);
    }

    win = checkTarget(gameMap);
}


// 游戏地图更改
bool Sokoban::updateGameMap(MapVec &mapData, PushMsg &msg) {
    int manRow = msg.pos.row, manCol = msg.pos.col, oldRow, oldCol;
    // 清除人位置并存储oldManPos
    for (int row = 0; row < mapHeight; row++) {
        for (int col = 0; col < mapWidth; col++) {
            if (mapData[row][col] == 2) {
                mapData[row][col] = 0;
                oldRow = row;
                oldCol = col;
            } else if (mapData[row][col] == 4) {
                mapData[row][col] = 3;
                oldRow = row;
                oldCol = col;
            }
        }
    }
    // 给与新的位置
    int testEle = mapData[manRow][manCol];
    if (msg.direction == 0) {
        if (testEle == 0) mapData[manRow][manCol] = 2;
        else if (testEle == 3) mapData[manRow][manCol] = 4;
        return true;
    }

    if (testEle == -1 || testEle == 1 || testEle == 5) {
        // 新位置错误还原
        mapData[oldRow][oldCol] = mapData[oldRow][oldCol] == 3 ? 4 : 2;
        return false;
    }
    else if (testEle == 0) mapData[manRow][manCol] = 2;
    else if (testEle == 3) mapData[manRow][manCol] = 4;

    if (pushMan(mapData, msg)) return true;
    else {
        // 无法移动时还原
        mapData[oldRow][oldCol] = mapData[oldRow][oldCol] == 3 ? 4 : 2;
        return false;
    }
}

// 人移动逻辑
bool Sokoban::pushMan(MapVec &mapData, PushMsg &msg) {
    int fRow = msg.pos.row, fCol = msg.pos.col;
    int manRow = fRow, manCol = fCol;

    switch (msg.direction) {
    case 1: fRow--; break;
    case 2: fRow++; break;
    case 3: fCol--; break;
    case 4: fCol++; break;
    }

    int srcTar = mapData[msg.pos.row][msg.pos.col] == 4 ? 3 : 0;
    switch (mapData[fRow][fCol]) {
    case 0:// 空地
        mapData[manRow][manCol] = srcTar;
        mapData[fRow][fCol] = 2;
        msg.pos = Pos(fRow, fCol);
        return true;
    case 1:// 箱子
        if (pushBox(mapData, PushMsg(Pos(fRow, fCol), msg.direction))) {
            mapData[manRow][manCol] = srcTar;
            mapData[fRow][fCol] = 2;
            msg.pos = Pos(fRow, fCol);
            return true;
        }
        return false;
    case 3:// 目标
        mapData[manRow][manCol] = srcTar;
        mapData[fRow][fCol] = 4;
        msg.pos = Pos(fRow, fCol);
        return true;
    case 5:// 箱子和目标
        if (pushBox(mapData, PushMsg(Pos(fRow, fCol), msg.direction))) {
            mapData[manRow][manCol] = srcTar;
            mapData[fRow][fCol] = 4;
            msg.pos = Pos(fRow, fCol);
            return true;
        }
        return false;
    default:// 墙
        return false;
    }
}

// 箱子移动逻辑
bool Sokoban::pushBox(MapVec &mapData, PushMsg msg) {
    int fRow = msg.pos.row, fCol = msg.pos.col;

    switch (msg.direction) {
    case 1: fRow--; break;
    case 2: fRow++; break;
    case 3: fCol--; break;
    case 4: fCol++; break;
    }

    // 仅管理前方物体
    switch (mapData[fRow][fCol]) {
    case 0: mapData[fRow][fCol] = 1; return true;
    case 3: mapData[fRow][fCol] = 5; return true;
    default: return false;
    }
}

// 判断是否是目标状态
bool Sokoban::checkTarget(MapVec mapData) {
    int box_t_num = 0;
    for (int row = 0; row < mapHeight; row++) {
        for (int col = 0; col < mapWidth; col++) {
            if (mapData[row][col] == 1) {
                return false;
            }
            if (mapData[row][col] == 5) {
                box_t_num++;
            }
        }
    }
    return box_t_num == boxNum;
}

// 判断是否是目标状态
bool Sokoban::checkDead(MapVec mapData) {
    // 找到人能推箱子的地点和方向
    vector<Pos> boxPos;

    for (int row = 0; row < mapHeight; row++) {
        for (int col = 0; col < mapWidth; col++) {
            if (mapData[row][col] == 1 || mapData[row][col] == 5) {
                boxPos.push_back(Pos(row, col));
            }
        }
    }

    // 遍历所有box
    for (auto box: boxPos) {
        // 搜索箱子四个方向
        if (mapData[box.row][box.col] == 5) continue;

        for (int dir = 1; dir <= 4; dir++) {
            int fRow = box.row, fCol = box.col, bRow = fRow, bCol = fCol;

            switch (dir) {
            case 1: fRow--; bRow++; break; // 上
            case 2: fRow++; bRow--; break; // 下
            case 3: fCol--; bCol++; break; // 左
            case 4: fCol++; bCol--; break; // 右
            }

            if ((mapData[fRow][fCol] == 0 || mapData[fRow][fCol] == 2
                    || mapData[fRow][fCol] == 3 || mapData[fRow][fCol] == 4) &&
                (mapData[bRow][bCol] == 0 || mapData[bRow][bCol] == 2
                    || mapData[bRow][bCol] == 3 || mapData[bRow][bCol] == 4))
                return false;
        }
        return true;
    }

    return false;
}


// AI PushBox
void Sokoban::searchTargetPath() {
    Status root(gameMap, getPushMsgs(gameMap));
    statusStack.push(root);

    while (!statusStack.empty()) {
        Status curStatus = statusStack.top();

        // 判断是否是目标状态
        if (checkTarget(curStatus.mapStatus)) {
            statusStack.pop();
            return;
        }

        // 判断是否死状态
        if (checkDead(curStatus.mapStatus)) {
            statusStack.pop();
            continue;
        }

        // 判断当前状态以前是否出现过
        if (isSearched(curStatus.mapStatus)) {
            statusStack.pop();
            continue;
        }

        // 寻找当前状态下所有可能的移动
        // 在所有可能移动中取第一个，把需要的信息压栈
        Status newStatus;
        if (getNewStatus(curStatus, newStatus)) {
            // 可以获取移动
            statusStack.pop();
            statusStack.push(curStatus);
            statusStack.push(newStatus);
        } else {
            // 不可获取移动
            statusStack.pop();
            continue;
        }
    }
}

// 获取当前状态下所有可能移动方式
std::vector<PushMsg> Sokoban::getPushMsgs(MapVec mapData) {
    // 找到人能推箱子的地点和方向
    vector<Pos> boxPos;
    std::vector<PushMsg> resMsgs;
    for (int row = 0; row < mapHeight; row++) {
        for (int col = 0; col < mapWidth; col++) {
            if (mapData[row][col] == 1 || mapData[row][col] == 5) {
                boxPos.push_back(Pos(row, col));
            }
        }
    }

    // 遍历所有box
    for (auto box: boxPos) {
        // 搜索箱子四个方向
        for (int dir = 1; dir <= 4; dir++) {
            int fRow = box.row, fCol = box.col, bRow = fRow, bCol = fCol;

            switch (dir) {
            case 1: fRow--; bRow++; break; // 上
            case 2: fRow++; bRow--; break; // 下
            case 3: fCol--; bCol++; break; // 左
            case 4: fCol++; bCol--; break; // 右
            }

            // 箱子可移动至 (fRow, fCol)
            if (mapData[fRow][fCol] == 0 || mapData[fRow][fCol] == 2 // 空地 或 人
                    || mapData[fRow][fCol] == 3 || mapData[fRow][fCol] == 4) // 目标 或 人和目标
            {
                // 人可移动至 (bRow, bCol)
                if (mapData[bRow][bCol] == 0 || mapData[bRow][bCol] == 2 // 空地 或 人
                        || mapData[bRow][bCol] == 3 || mapData[bRow][bCol] == 4) // 目标 或 人和目标
                {
                    bool canMove = false;
                    manToTarget(mapData, Pos(bRow, bCol), canMove);
                    if (canMove) {
                        if (mapData[fRow][fCol] == 3 || mapData[fRow][fCol] == 4) {
                            resMsgs.insert(resMsgs.begin(), PushMsg(bRow, bCol, dir));
                        }
                        else resMsgs.push_back(PushMsg(bRow, bCol, dir));
                    }
                }
            }
        }
    }

    return resMsgs;
}

stack<Pos> Sokoban::manToTarget(MapVec mapData, Pos tarPos, bool &tag)
{
    tag = false;
    // 获取人的状态初始位置
    Pos manPos;
    MapVec visited = mapData; // 已访问数组
    vector<vector<double> > distance; // a star启发表
    for (int row = 0; row < mapHeight; row++) {
        vector<double> lineDis;
        for (int col = 0; col < mapWidth; col++) {
            visited[row][col] = 0;
            lineDis.push_back(tarPos.getDistance(Pos(row, col)));
            if (mapData[row][col] == 2 || mapData[row][col] == 4) {
                manPos = Pos(row, col);
                mapData[row][col] = 0;
            }
        }
        distance.push_back(lineDis);
    }

    // 搜索寻找路径
    stack<Pos> nodeStack;
    nodeStack.push(manPos);
    while (!nodeStack.empty()) {
        Pos nowNode = nodeStack.top();
        int row = nowNode.row, col = nowNode.col;

        // 人可到达目标 (bRow, bCol)
        if (row == tarPos.row && col == tarPos.col) {
            tag = true;
            break;
        }

        // 节点被完全访问 人移动到障碍 (wall, box, box_target)
        if (visited[row][col] == 4 || mapData[row][col] == -1
                || mapData[row][col] == 1 || mapData[row][col] == 5)
        {
            nodeStack.pop();
            continue;
        }

        visited[row][col]++;

        vector<pair<double, Pos> > lessSort;
        lessSort.push_back( make_pair(distance[row-1][col], Pos(row-1, col)) ); // 上
        lessSort.push_back( make_pair(distance[row+1][col], Pos(row+1, col)) ); // 下
        lessSort.push_back( make_pair(distance[row][col-1], Pos(row, col-1)) ); // 左
        lessSort.push_back( make_pair(distance[row][col+1], Pos(row, col+1)) ); // 右
        std::sort(lessSort.begin(), lessSort.end(),
                  [](pair<double, Pos> v1, pair<double, Pos> v2) {
            return v1.first < v2.first;
        });

        nodeStack.push(lessSort[visited[row][col] - 1].second);
    }

    stack<Pos> walkPath;
    if (true) {
        while(!nodeStack.empty()) {
            walkPath.push(nodeStack.top());
            nodeStack.pop();
        }
        if (!walkPath.empty()) walkPath.pop();
    }

    return walkPath;
}

// 判断当前状态以前是否出现过
bool Sokoban::isSearched(MapVec mapData) {
    stack<Status> tempStack = statusStack;
    tempStack.pop();

    while (!tempStack.empty()) {
        Status temp = tempStack.top();
        tempStack.pop();

        bool equal = true;
        for (int row = 0; row < mapHeight; row++) {
            for (int col = 0; col < mapWidth; col++) {
                if (temp.mapStatus[row][col] != mapData[row][col]) {
                    equal = false;
                }
            }
        }

        if (equal) return true;
    }

    return false;
}

// 在所有可能移动中取下一个生成新状态
bool Sokoban::getNewStatus(Status &oldStatus, Status &newStatus) {
    if ((int)oldStatus.pushMsgs.size() == oldStatus.nowMsgIndex) {
        return false;
    }

    newStatus.mapStatus = oldStatus.mapStatus;
    updateGameMap(newStatus.mapStatus, oldStatus.pushMsgs[oldStatus.nowMsgIndex]);
    newStatus.pushMsgs = getPushMsgs(newStatus.mapStatus);
    oldStatus.nowMsgIndex++;
    return true;
}

