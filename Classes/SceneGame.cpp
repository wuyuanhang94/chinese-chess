#include "SceneGame.h"

SceneGame::SceneGame()
{
}

CCScene* SceneGame::scene(bool red)
{
    CCScene* scene = CCScene::create();
    SceneGame* layer = SceneGame::create(red);
    scene->addChild(layer);
    return scene;
}

SceneGame* SceneGame::create(bool red)
{
    SceneGame* pRet = new SceneGame();
    if(pRet)
    {
        pRet->init(red);
        pRet->autorelease();
    }
    return pRet;
}

bool SceneGame::init(bool red)
{
    CCLayer::init();

    //加背景
    this->_plateOffset = ccp(20,10);//棋盘的偏移
    CCSprite* desk = CCSprite::create("floor.jpg");
    this->addChild(desk);
    desk->setPosition(ccp(winSize.width/2, winSize.height/2));
    desk->setScaleX(winSize.width/desk->getContentSize().width);
    desk->setScaleY(winSize.height/desk->getContentSize().height);

    //加棋盘
    CCSprite* plate = CCSprite::create("background.png");
    this->addChild(plate);
    plate->setAnchorPoint(CCPointZero);
    plate->setPosition(_plateOffset);
    plate->setScale((winSize.height - _plateOffset.y*2)/plate->getContentSize().height);

    //加棋子
    this->_stoneOffset = ccp(60,33);
    this->_d = 46;
    for(int i=0;i<32;i++)
    {
        _s[i] = Stone::create(i, red);
        this->addChild(_s[i]);

        _s[i]->setPosition(ccp(_s[i]->getX()*_d, _s[i]->getY()*_d) + _stoneOffset);
    }
    return true;
}
