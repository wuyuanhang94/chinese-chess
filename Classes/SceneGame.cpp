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
    _selectid = -1;
    _selectSprite = CCSprite::create("selected.png");
    this->addChild(_selectSprite);
    _selectSprite->setVisible(false);
    _selectSprite->setScale(.8f);
    _redTurn = true;

    //enable touching
    setTouchEnabled(true);
    setTouchMode(kCCTouchesOneByOne);

    //加悔棋按钮
    CCMenu* menu = CCMenu::create();
    CCMenuItem* item = CCMenuItemImage::create("regret.jpg","regret.jgp",this,menu_selector(SceneGame::back));
    menu->addChild(item);
    menu->setPosition(ccp(winSize.width/2+200, winSize.height/2));
    addChild(menu);
    //创建悔棋array
    _steps = CCArray::create();
    _steps->retain();

    //干死老将后换边
    _redSide = red;

    return true;
}

void SceneGame::back(CCObject *)
{
    if(_steps->count() == 0) return;

    Step* step = (Step*)_steps->lastObject();
    _steps->removeLastObject();
    _s[step->_moveid]->setX(step->_xFrom);
    _s[step->_moveid]->setY(step->_yFrom);
    _s[step->_moveid]->setPosition(getStonePos(step->_xFrom, step->_yFrom));

    if(step->_killid != -1)
    {
        _s[step->_killid]->setVisible(true);
        _s[step->_killid]->setDead(false);
    }

    _redTurn = !_redTurn;
}

void SceneGame::setSelectID(int id)
{
    if(id == -1 || _s[id]->getRed() != _redTurn) return;
    _selectid = id;

    _selectSprite->setVisible(true);
    _selectSprite->setPosition(_s[_selectid]->getPosition());
}

void SceneGame::moveStone(int moveid, int killid, int x, int y)
{
    //if(killid == -1) return;//这么写是有bug的 只有当killid有子时才能move过去 显然不对
    if(killid != -1 && _s[moveid]->getRed() == _s[killid]->getRed())
    {
        setSelectID(killid);
        return;
    }

    Step* step = Step::create(moveid,killid,_s[moveid]->getX(),_s[moveid]->getY(),x,y);
    _steps->addObject(step);

    if(killid != -1)
    {
        _s[killid]->setDead(true);
        _s[killid]->setVisible(false);

        if(_s[killid]->getType() == Stone::JIANG)
        {
            CCDirector::sharedDirector()->replaceScene(SceneGame::scene(!_redSide));
        }
    }

    //moveid不受killid影响 (x,y)应该在上一个判断中判断过为合法坐标 moveid可以移动
    _s[moveid]->setX(x);
    _s[moveid]->setY(y);
    _s[moveid]->setPosition(getStonePos(x,y));

    _selectid = -1;
    _selectSprite->setVisible(false);
    _redTurn = !_redTurn;
}

CCPoint SceneGame::getStonePos(int x, int y)
{
    return ccp(x*_d, y*_d) + this->_stoneOffset;
}

bool SceneGame::ccTouchBegan(CCTouch *pTouch, CCEvent *)
{
    CCPoint ptInWin = pTouch->getLocation();
    int x,y;
    //找到点击对应的棋盘坐标
    if(!getClickPos(ptInWin, x, y))
        return false;

    int clickid = getStone(x,y);
    if(_selectid == -1)
        setSelectID(clickid);
    else
        moveStone(_selectid, clickid, x, y);
    return true;
}

int SceneGame::getStone(int x, int y)
{
    Stone* s;
    for(int i=0;i<32;i++)
    {
        s = _s[i];
        if(s->getX() == x && s->getY() == y && !s->getDead())
            return i;
    }
    return -1;
}

bool SceneGame::getClickPos(CCPoint ptInWin, int &x, int &y)
{
    for(x=0;x<=8;x++)
        for(y=0;y<=9;y++)
        {
            CCPoint ptInPlate = ccp(x*_d,y*_d) + _stoneOffset;
            if(ptInWin.getDistance(ptInPlate) < _d/2)
                return true;
        }
    return false;
}
