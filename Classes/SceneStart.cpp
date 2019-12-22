#include "SceneStart.h"
#include "SceneGame.h"

SceneStart::SceneStart()
{
}

CCScene* SceneStart::scene()
{
    CCScene* scene = CCScene::create();
    SceneStart* layer = SceneStart::create();
    scene->addChild(layer);
    return scene;
}

bool SceneStart::init()
{
    CCLayer::init();


    CCSprite* bkr = CCSprite::create("bkg2.png");
    this->addChild(bkr);
    CCSprite* bkb = CCSprite::create("bkg1.png");
    this->addChild(bkb);

    _red = bkr;
    _black = bkb;

    bkr->setPosition(ccp(winSize.width/2-100, winSize.height/2));
    bkb->setPosition(ccp(winSize.width/2+100, winSize.height/2));

    setTouchEnabled(true);
    setTouchMode(kCCTouchesOneByOne);

    return true;
}

bool SceneStart::ccTouchBegan(CCTouch*, CCEvent*)
{
    return true;
}

void SceneStart::ccTouchEnded(CCTouch *pTouch, CCEvent *)
{
    CCPoint ptClick = pTouch->getLocation();
    bool bClickStone = false;
    if(_red->boundingBox().containsPoint(ptClick))
    {
        _selected = true;
        bClickStone = true;
    }
    else if(_black->boundingBox().containsPoint(ptClick))
    {
        _selected = false;
        bClickStone = true;
    }
    if(bClickStone)
    {
        //移动两枚棋子
        CCMoveTo* moveTo1 = CCMoveTo::create(1, ccp(winSize.width/2, winSize.height/2));
        CCMoveTo* moveTo2 = CCMoveTo::create(1, ccp(winSize.width/2, winSize.height/2));
        //滚动两枚棋子
        CCRotateBy* rotate1 = CCRotateBy::create(1,360);
        CCRotateBy* rotate2 = CCRotateBy::create(1,-360);

        CCSpawn* spawn1 = CCSpawn::create(moveTo1, rotate1, NULL);
        CCSpawn* spawn2 = CCSpawn::create(moveTo2, rotate2, NULL);

        _red->runAction(spawn1);
        _black->runAction(spawn2);

        scheduleUpdate();
    }
}

void SceneStart::update(float delta)
{
    float x1 = _red->getPositionX();
    float x2 = _black->getPositionX();
    if(abs(x1-x2) <_red->getContentSize().width)
    {
        CCDirector::sharedDirector()->replaceScene(SceneGame::scene(this->_selected));
    }
}
