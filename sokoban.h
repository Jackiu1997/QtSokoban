#ifndef SOKOBAN_H
#define SOKOBAN_H

#include <QObject>
#include <vector>
#include <stack>
#include <queue>
#include <cmath>
#include "windows.h"
using std::vector;
using std::queue;
using std::stack;
using std::pair;
using std::make_pair;

#define MapVec vector<vector<int> >

// 找到人的位置
struct Pos {
    int row;
    int col;

    Pos() : row(0), col(0) {}
    Pos(int r, int c) : row(r), col(c) {}
    double getDistance(Pos tar) {
        return sqrt(pow(row - tar.row, 2) + pow(col - tar.col, 2));
    }
};

struct PushMsg {
    Pos pos;
    int direction;

    PushMsg() {}
    PushMsg(Pos pos, int dir) : pos(pos), direction(dir) {}
    PushMsg(int row, int col, int dir) {
        pos = Pos(row, col);
        direction = dir;
    }
};

struct Status {
    MapVec mapStatus;
    int nowMsgIndex;
    vector<PushMsg> pushMsgs;

    Status(): nowMsgIndex(0) {}
    Status(MapVec mapData, vector<PushMsg> msgs): nowMsgIndex(0) {
        mapStatus = std::move(mapData);
        pushMsgs = std::move(msgs);
    }
};

class Sokoban : public QObject
{
    Q_OBJECT
public:
    bool win = false;
    MapVec gameMap; // 推箱子地图 -1：墙 0：地面 1：箱子 2：人 3：目标 4:人和目标 5：箱子和目标

    Sokoban();
    // 地图加载模块
    void loadMap(MapVec mapData, int height, int width);

    void actionByMan(int direction);

    void actionByAI();
    std::stack<Status> statusStack;
private:
    int mapWidth;
    int mapHeight;
    int boxNum;
    Pos manPos;

    // 推箱子规则模块
    bool updateGameMap(MapVec &mapData, PushMsg &msg);
    bool pushMan(MapVec &mapData, PushMsg &msg);
    bool pushBox(MapVec &mapData, PushMsg msg);
    bool checkTarget(MapVec mapData);

    // AI推箱子求解模块
    void searchTargetPath();
    // 辅助功能函数
    stack<Pos> manToTarget(MapVec mapData, Pos tarPos, bool &tag);
    std::vector<PushMsg> getPushMsgs(MapVec mapData);
    bool isSearched(MapVec mapData);
    bool getNewStatus(Status &oldStatus, Status &newStatus);
    bool checkDead(vector<vector<int> > mapData);

signals:
    void sendUpdate();
};

#endif // SOKOBAN_H
