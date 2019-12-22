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

    static CCScene* scene(bool red);
    static SceneGame* create(bool red);

    bool init(bool red);

    CCPoint _plateOffset;
    CCPoint _stoneOffset; //第一个棋子的显示坐标
    float _d;
    Stone* _s[32];

    bool ccTouchBegan(CCTouch *pTouch, CCEvent*);
    int _selectid = -1;
    bool getClickPos(CCPoint ptInWin, int& x, int& y);
    int getStone(int x, int y);
    void setSelectID(int id);
    CCSprite* _selectSprite;
    void moveStone(int moveid, int killid, int x, int y);
    CCPoint getStonePos(int x, int y);
    bool _redTurn;
    bool _redSide;

    void back(CCObject*);
    CCArray* _steps;
};

#endif // SCENEGAME_H
