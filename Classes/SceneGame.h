#ifndef SCENEGAME_H
#define SCENEGAME_H

#include "cocos2d.h"
USING_NS_CC;
#include "Stone.h"

class Step : public CCObject
{
public:
    int _moveid;
    int _killid;
    int _xFrom;
    int _yFrom;
    int _xTo;
    int _yTo;
    static Step* create(int moveid, int killid, int xFrom, int yFrom, int xTo, int yTo)
    {
        Step* step = new Step();
        step->_moveid = moveid;
        step->_killid = killid;
        step->_xFrom = xFrom;
        step->_yFrom = yFrom;
        step->_xTo = xTo;
        step->_yTo = yTo;
        step->autorelease();
        return step;
    }
};

class SceneGame : public CCLayer
{
public:
    SceneGame();
    ~SceneGame()
    {
        _steps->release();
    }

    CCSize winSize = CCDirector::sharedDirector()->getWinSize();

    // cocos相关的
    static CCScene* scene(bool red);
    static SceneGame* create(bool red);
    bool init(bool red);

    //棋盘相关的
    CCPoint _plateOffset; //背景偏移
    CCPoint _stoneOffset; //第一个棋子的显示坐标
    float _d;

    //游戏状态
    bool _redTurn;
    bool _redSide;
    int _selectid = -1;
    CCSprite* _selectSprite;
    CCArray* _steps;

    Stone* _s[32];

    //选棋+走棋
    void setSelectID(int id);
    //x,y是dest coordinates
    void moveStone(int moveid, int killid, int x, int y);
    void moveComplete(CCNode* , void*);

    //走棋判断
    bool canMove(int moveid, int killid, int x, int y);
    bool canMoveJiang(int moveid, int killid, int x, int y);
    bool canMoveShi(int moveid, int x, int y);
    bool canMoveXiang(int moveid, int x, int y);
    bool canMoveChe(int moveid, int x, int y);
    bool canMoveMa(int moveid, int x, int y);
    bool canMovePao(int moveid, int killid, int x, int y);
    bool canMoveBing(int moveid, int x, int y);

    //坐标转换/辅助计算/内部调用
    bool getClickPos(CCPoint ptInWin, int& x, int& y);
    int getStone(int x, int y);
    CCPoint getStonePos(int x, int y);
    void setRealPos(Stone* s);
    int getStoneCount(int xo, int yo, int x, int y);

    //cocos相关
    bool ccTouchBegan(CCTouch *pTouch, CCEvent*);
    void back(CCObject*);
};

#endif // SCENEGAME_H
