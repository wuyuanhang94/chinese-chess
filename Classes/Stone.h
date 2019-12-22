#ifndef STONE_H
#define STONE_H

#include "cocos2d.h"

USING_NS_CC;

//Stone有两张表
//1.初始位置摆放
//2.图片加载

class Stone : public CCSprite
{
public:
    Stone();

    //棋子类型
    enum TYPE {JIANG,SHI,XIANG,CHE,MA,PAO,BING};

    static Stone* create(int id, bool red);

    //init时候需要填充每个图片
    //为简化switch流程 最好也搞个表
    //其实不需要type 和 red 仅仅一个id就应该把这些cover掉
    bool init(int id, bool red);
    //reset去做一些可能需要重复做的事情
    void reset(bool red);

    //棋子初始摆放位置
    //不是每个棋子的属性 更是整个棋局的属性 用static
    static struct InitPos
    {
        int _x;
        int _y;
        Stone::TYPE _type;
    } _initPos[16];

    CC_SYNTHESIZE(TYPE, _type, Type)
    CC_SYNTHESIZE(int, _x, X)
    CC_SYNTHESIZE(int, _y, Y)
    CC_SYNTHESIZE(bool, _dead, Dead)
    CC_SYNTHESIZE(bool, _red, Red)
    CC_SYNTHESIZE(int, _id, ID)

#if 0
    Type _type;

    //相比与棋盘的棋子坐标
    int _x;
    int _y;

    //id
    int _id;//0-31
    bool _dead;

    bool _red;
#endif
};

#endif // STONE_H
