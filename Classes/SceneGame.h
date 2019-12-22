#ifndef SCENEGAME_H
#define SCENEGAME_H

#include "cocos2d.h"
USING_NS_CC;
#include "Stone.h"

class SceneGame : public CCLayer
{
public:
    SceneGame();

    CCSize winSize = CCDirector::sharedDirector()->getWinSize();

    static CCScene* scene(bool red);
    static SceneGame* create(bool red);

    bool init(bool red);

    CCPoint _plateOffset;
    CCPoint _stoneOffset; //第一个棋子的显示坐标
    float _d;
    Stone* _s[32];
};

#endif // SCENEGAME_H
