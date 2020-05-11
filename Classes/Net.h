#ifndef NET_H
#define NET_H

class Net
{
    // private ctor
    // 禁止调用显式调用ctor
    Net();
public:
    int _srv;
    int _client;

    static Net* getInstance();

    char _packet[4];
    int _recvlen;

    //server-special
    bool listen(short port);
    bool accept();

    //client-special
    bool connect(short port, const char* ip);

    //APIs in common
    int send(const char* buf, int size);
    bool recv();
    bool isPacketReady();
    int setNonblock(int fd);
};

#endif // NET_H
