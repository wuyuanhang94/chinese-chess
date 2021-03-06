#include "SceneGame.h"
#include <algorithm>

using namespace std;

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

        _s[i]->setPosition(ccp(CCRANDOM_0_1()*winSize.width, CCRANDOM_0_1()*winSize.height));
        CCMoveTo* move = CCMoveTo::create(1,getStonePos(_s[i]->getX(), _s[i]->getY()));
        _s[i]->runAction(move);
    }

    //游戏状态
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

    //server or client
    CCMenuItemFont* itemS = CCMenuItemFont::create("MakeServer",
                                                  this,menu_selector(SceneGame::makeServer));
    CCMenuItemFont* itemC = CCMenuItemFont::create("MakeClient",
                                                  this,menu_selector(SceneGame::makeClient));
    menu->addChild(itemS);
    menu->addChild(itemC);
    itemS->setPosition(item->getPosition() + ccp(0,50));
    itemC->setPosition(item->getPosition() + ccp(0,100));

    // 两个按钮互斥 选择一个就不能选另一个 同时选择本身好了不能重复选择 禁用
    itemS->setUserObject(itemC);
    itemC->setUserObject(itemS);

    CCMenuItem* itemSurrender = CCMenuItemFont::create("Surrender", this, \
                                              menu_selector(SceneGame::surrender));
    itemSurrender->setPosition(item->getPosition() - ccp(0, 50));
    menu->addChild(itemSurrender);

    return true;
}

void SceneGame::surrender(CCObject *)
{
    char buf[2];
    buf[0] = 4;
    buf[1] = _redSide;

    Net::getInstance()->send(buf, 2);
    resetGame();
}

//阻塞式socket太卡了 还是需要非阻塞的
void SceneGame::accept(float dt)
{
    bool b = Net::getInstance()->accept();
    if(b)
    {
        CCLog("Connect successfully");
        unschedule(schedule_selector(SceneGame::accept));

        char buf[2];
        buf[0] = 0;
        buf[1] = _redSide ? 0 : 1;
        Net::getInstance()->send(buf,2);
        //server收消息
        schedule(schedule_selector(SceneGame::recv), 1.0f);
    }
}

void SceneGame::makeServer(CCObject* item)
{
    if(Net::getInstance()->listen(9999))
    {
        CCMenuItem* menuItem = (CCMenuItem*)item;
        menuItem->setEnabled(false);
        menuItem = (CCMenuItem*)menuItem->getUserObject();
        menuItem->setEnabled(false);

        schedule(schedule_selector(SceneGame::accept), 1.0f);
    }
}

void SceneGame::makeClient(CCObject* item)
{
    if(Net::getInstance()->connect(9999, "127.0.0.1"))
    {
        CCMenuItem* menuItem = (CCMenuItem*)item;
        menuItem->setEnabled(false);
        menuItem = (CCMenuItem*)menuItem->getUserObject();
        menuItem->setEnabled(false);

        schedule(schedule_selector(SceneGame::recv), 1.0f);
    }
}

void SceneGame::resetGame()
{
    _redSide = !_redSide;
    _redTurn = true;
    _selectid = -1;
    _selectSprite->setVisible(false);
    //32个棋子reset
    for(int i=0;i<32;i++)
    {
        Stone* s = _s[i];
        s->reset(_redSide);
        setRealPos(s);
    }
    _steps->removeAllObjects();
}

void SceneGame::recv(float)
{
    bool bSuccess = Net::getInstance()->recv();
    if(bSuccess)
    {
        char* buf = Net::getInstance()->_packet;
        Net::getInstance()->_recvlen = 0;
        switch(buf[0])
        {
        case 0:
            // 这里是重大bug replaceScene 重新开始
            // recv不再侦听
            //CCDirector::sharedDirector()->replaceScene(SceneGame::scene(buf[1]));
        {
            for(int i=0;i<32;i++)
            {
                Stone* s = _s[i];
                s->reset(buf[1]);
                setRealPos(s);
                _redSide = buf[1];
            }
        }
            break;
        case 1:
            setSelectID(buf[1]);
            break;
        case 2:
        {
            int killid = getStone(buf[2], buf[3]);
            moveStone(buf[1], killid, buf[2], buf[3]);
            //如果老将干掉了 reset 游戏
            if(_redSide)
            {
                for(int i=0;i<16;i++)
                {
                    Stone* s = _s[i];
                    if(s->getType() == Stone::JIANG && s->getDead())
                        surrender(NULL);
                }
            }
            else
            {
                for(int i=16;i<32;i++)
                {
                    Stone* s = _s[i];
                    if(s->getType() == Stone::JIANG && s->getDead())
                        surrender(NULL);
                }
            }
        }
            break;
        case 3:
        {
            doRealBack();
            doRealBack();
        }
            break;
        case 4:
        {
            resetGame();
        }
            break;
        }
    }
}

void SceneGame::setRealPos(Stone *s)
{
    s->setPosition(getStonePos(s->getX(), s->getY()));
}

void SceneGame::doRealBack()
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

void SceneGame::back(CCObject *)
{
    doRealBack();
    doRealBack();
    char buf = 3;
    Net::getInstance()->send(&buf, 1);
}

void SceneGame::setSelectID(int id)
{
    if(id == -1 || _s[id]->getRed() != _redTurn) return;
    _selectid = id;

    _selectSprite->setVisible(true);
    _selectSprite->setPosition(_s[_selectid]->getPosition());
}

bool SceneGame::canMoveJiang(int moveid, int killid, int x, int y)
{
    //老将照面 特殊处理
    if(killid != -1)
    {
        Stone* killStone = _s[killid];
        if(killStone->getType() == Stone::JIANG)
            return canMoveChe(moveid,x,y);
    }

    Stone* s = _s[moveid];
    //1.走一步
    //2.不能出九宫
    int xo = s->getX();
    int yo = s->getY();
    int xoff = abs(xo - x);
    int yoff = abs(yo - y);
    if(xoff > 1 || yoff > 1 || xoff+yoff != 1) return false;
    //不能出九宫 有两种情况
    // 黑or红
    //......
    //......
    // 红 黑
    //ture false // _redSide
    if(x < 3 || x > 5) return false;
    if(_redSide == s->getRed())
    {
        if(y < 0 || y > 2) return false;
    }
    else
    {
        if(y < 7 || y > 9) return false;
    }
    return true;
}

bool SceneGame::canMoveShi(int moveid, int x, int y)
{
    Stone* s = _s[moveid];
    //1.走一步 走对角线
    //2.不能出九宫
    int xo = s->getX();
    int yo = s->getY();
    int xoff = abs(xo - x);
    int yoff = abs(yo - y);
    if(xoff !=1 || yoff != 1) return false;

    if(x < 3 || x > 5) return false;
    if(_redSide == s->getRed())
    {
        if(y < 0 || y > 2) return false;
    }
    else
    {
        if(y < 7 || y > 9) return false;
    }
    return true;
}

bool SceneGame::canMoveXiang(int moveid, int x, int y)
{
    Stone* s = _s[moveid];
    //1.走一步 走对角线
    //2.xoff yoff 都为2
    //3.象眼
    int xo = s->getX();
    int yo = s->getY();
    int xoff = abs(xo - x);
    int yoff = abs(yo - y);
    if(xoff != 2 || yoff != 2) return false;

    if(_redSide == s->getRed())
    {
        if(y < 0 || y > 4) return false;
    }
    else
    {
        if(y < 5 || y > 9) return false;
    }

    int xmid = (x+xo)/2;
    int ymid = (y+yo)/2;
    if(getStone(xmid, ymid) != -1) return false;
    return true;
}

int SceneGame::getStoneCount(int xo, int yo, int x, int y)
{
    //to calculate how many stones lie from (xo,yo) to (x,y) in the same line
    if(xo != x && yo != y) return -1;
    int cnt = 0;
    if(xo == x)
    {
        int ymin = min(y,yo);
        int ymax = max(y,yo);
        for(int j=ymin+1;j<ymax;j++)
            if(getStone(x,j) != -1) cnt++;
    }
    else
    {
        int xmin = min(x,xo);
        int xmax = max(x,xo);
        for(int i=xmin+1;i<xmax;i++)
            if(getStone(i,y) != -1) cnt++;
    }
    return cnt;
}

bool SceneGame::canMoveChe(int moveid, int x, int y)
{
    //1.走直线
    //2.中间不能有阻拦
    Stone* s = _s[moveid];

    int xo = s->getX();
    int yo = s->getY();
    if(getStoneCount(xo,yo,x,y) != 0) return false;
    return true;
}

bool SceneGame::canMoveMa(int moveid, int x, int y)
{
    Stone* s = _s[moveid];
    //1.走日 xoff yoff
    //2.马眼
    int xo = s->getX();
    int yo = s->getY();
    int xoff = abs(xo - x);
    int yoff = abs(yo - y);
    int d = xoff*10 + yoff;
    if(d != 21 && d != 12) return false;

    if(d == 21 && getStone((x+xo)/2, yo) != -1) return false;
    if(d == 12 && getStone(xo, (y+yo)/2) != -1) return false;
    return true;
}

bool SceneGame::canMovePao(int moveid, int killid, int x, int y)
{
    Stone* s = _s[moveid];

    int xo = s->getX();
    int yo = s->getY();
    if(killid == -1) return canMoveChe(moveid, x, y);
    if(getStoneCount(xo,yo,x,y) != 1) return false;
    return true;
}

bool SceneGame::canMoveBing(int moveid, int x, int y)
{
    Stone* s = _s[moveid];
    //1.走一步
    //2.不能回头
    int xo = s->getX();
    int yo = s->getY();
    int xoff = abs(xo - x);
    int yoff = abs(yo - y);
    int d = 10*xoff + yoff;
    if(d != 1 && d != 10) return false;

    if(_redSide == s->getRed())
    {
        if(y < yo) return false;//不能回头
        if(yo <= 4 && y == yo) return false;
    }
    else
    {
        if(y > yo) return false;//不能回头
        if(yo >= 5 && y == yo) return false;
    }
    return true;
}

bool SceneGame::canMove(int moveid, int killid, int x, int y)
{
    Stone* s = _s[moveid];
    switch (s->getType()) {
    case Stone::JIANG:
        return canMoveJiang(moveid, killid, x, y);
        break;
    case Stone::SHI:
        return canMoveShi(moveid, x, y);
        break;
    case Stone::XIANG:
        return canMoveXiang(moveid, x, y);
        break;
    case Stone::CHE:
        return canMoveChe(moveid, x, y);
        break;
    case Stone::MA:
        return canMoveMa(moveid, x, y);
        break;
    case Stone::PAO:
        return canMovePao(moveid, killid, x, y);
        break;
    case Stone::BING:
        return canMoveBing(moveid, x, y);
        break;
    }
    return false;
}

void SceneGame::moveStone(int moveid, int killid, int x, int y)
{
    //moveStone分两步
    //1.执行走棋
    //2.处理死棋的结果
    //if(killid == -1) return;//这么写是有bug的 只有当killid有子时才能move过去 显然不对
    if(killid != -1 && _s[moveid]->getRed() == _s[killid]->getRed())
    {
        //更换选择
        setSelectID(killid);
        return;
    }

    bool bCanMove = canMove(moveid, killid, x, y);
    if(!bCanMove) return;

    Step* step = Step::create(moveid,killid,_s[moveid]->getX(),_s[moveid]->getY(),x,y);
    _steps->addObject(step);

    //moveid不受killid影响 (x,y)应该在上一个判断中判断过为合法坐标 moveid可以移动
    _s[moveid]->setX(x);
    _s[moveid]->setY(y);
    CCMoveTo* move = CCMoveTo::create(.75f, getStonePos(x,y));
    CCCallFuncND* call = CCCallFuncND::create(this,
                                     callfuncND_selector(SceneGame::moveComplete),
                                     (void*)(intptr_t)killid);
    CCSequence* seq = CCSequence::create(move, call, NULL);
    _s[moveid]->setZOrder(_s[moveid]->getZOrder()+1);
    _s[moveid]->runAction(seq);
    _selectSprite->setVisible(false);
    // setRealPos(_s[moveid]);
}

void SceneGame::moveComplete(CCNode* moveStone, void* _killid)
{
    moveStone->setZOrder(moveStone->getZOrder()-1);
    int killid = (int)(intptr_t)_killid;
    if(killid != -1)
    {
        _s[killid]->setDead(true);
        _s[killid]->setVisible(false);
#if 0
        if(_s[killid]->getType() == Stone::JIANG)
        {
            CCDirector::sharedDirector()->replaceScene(SceneGame::scene(!_redSide));
        }
#endif
    }

    _selectid = -1;
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
    {
        if(clickid == -1) return false;
        if(_s[clickid]->getRed() != _redSide) return false;
        //发送选棋报文
        char buf[2];
        buf[0] = 1;
        buf[1] = clickid;
        Net::getInstance()->send(buf,2);

        setSelectID(clickid);
    }
    else
    {
        //发送走棋报文
        char buf[4];
        buf[0] = 2;
        buf[1] = _selectid;
        buf[2] = 8-x;
        buf[3] = 9-y;
        Net::getInstance()->send(buf,4);

        moveStone(_selectid, clickid, x, y);
    }
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
