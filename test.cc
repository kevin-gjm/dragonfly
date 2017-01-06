#include <functional>
#include <glog/logging.h>
#include "EventLoop.h"
#include "Connector.h"
using namespace dragonfly::net;
using namespace dragonfly;

class EchoServer
{
    public:
        EchoServer()
        {
            std::string ip ="127.0.0.1";
            server_ = new EventLoop(5);
            server_->setPort(8888);
            server_->setIp(ip);
            server_->setReadCallback(std::bind(&EchoServer::onMessage,this,std::placeholders::_1));
            server_->setConnectionCallback(std::bind(&EchoServer::onConnection,this,std::placeholders::_1));
        }
        ~EchoServer()
        {
            delete server_;
        }
        void start()
        {
            server_->loop();
        }
    private:
        void onMessage(Conn* conn)
        {
            conn->MoveBufferReadToWrite();
            LOG(INFO)<< "OnMessage";
        }
        void onConnection(Conn* conn)
        {
            LOG(INFO)<< "OnNewConnection";
            char buf[128]="new conn";
            conn->AddToWriteBuffer(buf,strlen(buf));
            LOG(INFO)<< "OnNewConnection over";
        }
        EventLoop* server_;

};

int main()
{
    EchoServer server;
    server.start();
    return 1;
}
