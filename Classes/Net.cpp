#include "Net.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "cocos2d.h"

Net* Net::getInstance()
{
    static Net* ret = NULL;
    if(ret == NULL) return ret = new Net();
    return ret;
}

Net::Net()
{
    _srv = -1;
    _client = -1;
    _recvlen = 0;
}

int Net::setNonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

bool Net::listen(short port)
{
    if(_srv != -1) return false;

    _srv = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int ret = bind(_srv, (struct sockaddr*)&addr, sizeof(addr));
    if(ret < 0)
    {
        close(_srv);
        _srv = -1;
        return false;
    }

    setNonblock(_srv);
    ::listen(_srv, 10);
    return true;
}

bool Net::accept()
{
    if(_srv == -1) return false;

    _client = ::accept(_srv, NULL, NULL);
    setNonblock(_client);
    return _client != -1;
}

int Net::send(const char *buf, int size)
{
    if(_client == -1) return -1;
    return ::send(_client, buf, size, 0);
}

void Net::recv()
{
    if(_client == -1) return;
    int ret = ::recv(_client, _packet + _recvlen, 1, 0);
    if(ret == 1) _recvlen++;
}

bool Net::isPacketReady()
{
    if(_recvlen == 0) return false;
    switch(_packet[0])
    {
    case 0:
    case 1:
    case 4:
        return _recvlen == 2;
    case 2:
        return _recvlen == 4;
    case 3:
        return _recvlen == 1;
    }
    CCAssert(0, "packer error");
    return false;
}

bool Net::connect(short port, const char *ip)
{
    if(_client != -1) return false;
    _client = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    int ret = ::connect(_client, (struct sockaddr*)&addr, sizeof(addr));
    if(ret < 0)
    {
        close(_client);
        _client = -1;
        CCLog("connect error, errno=%d", errnos);
        return false;
    }
    setNonblock(_client);
    return true;
}
