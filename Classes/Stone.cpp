#include "Stone.h"

Stone::Stone()
{
}

//初始化的位置需要做表来实现
//当然这个表可以进一步简化
Stone::InitPos Stone::_initPos[16] =
{
    {0,0,Stone::CHE},
    {1,0,Stone::MA},
    {2,0,Stone::XIANG},
    {3,0,Stone::SHI},
    {4,0,Stone::JIANG},
    {5,0,Stone::SHI},
    {6,0,Stone::XIANG},
    {7,0,Stone::MA},
    {8,0,Stone::CHE},

    {1,2,Stone::PAO},
    {7,2,Stone::PAO},

    {0,3,Stone::BING},
    {2,3,Stone::BING},
    {4,3,Stone::BING},
    {6,3,Stone::BING},
    {8,3,Stone::BING},
};

Stone* Stone::create(int id, bool red)
{
    Stone* s = new Stone();
    s->init(id, red);
    s->autorelease();
    return s;
}

bool Stone::init(int id, bool red)
{
    const char* stonePic[14] =
    {
        "rshuai.png",
        "rshi.png",
        "rxiang.png",
        "rche.png",
        "rma.png",
        "rpao.png",
        "rbing.png",

        "bjiang.png",
        "bshi.png",
        "bxiang.png",
        "bche.png",
        "bma.png",
        "bpao.png",
        "bzu.png",
    };

    _id = id;
    _red = _id < 16;
    _type = _initPos[_id%16]._type;
    int idx = (_red?0:1)*7+_type;
    CCSprite::initWithFile(stonePic[idx]);

    setScale(.8f);

    reset(red);

    return true;
}

void Stone::reset(bool red)
{
    this->setDead(false);
    if(red)
    {
        if(_id < 16)
        {
            this->setX(_initPos[_id]._x);
            this->setY(_initPos[_id]._y);
        }
        else
        {
            this->setX(8 - _initPos[_id-16]._x);
            this->setY(9 - _initPos[_id-16]._y);
        }
    }
    else
    {
        if(_id >= 16)
        {
            this->setX(_initPos[_id - 16]._x);
            this->setY(_initPos[_id - 16]._y);
        }
        else
        {
            this->setX(8 - _initPos[_id]._x);
            this->setY(9 - _initPos[_id]._y);
        }
    }
}
