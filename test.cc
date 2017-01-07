#include <functional>
#include <glog/logging.h>
#include "TcpServer.h"
using namespace dragonfly::net;

class EchoServer
{
    public:
        EchoServer(int count,std::string& ip,int port)
			:server_(new TcpServer(count, ip, port))
        {
			server_->setReadCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1));
			server_->setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        }
        ~EchoServer()
        {
            delete server_;
        }
        void start()
        {
            server_->startRun();
        }
    private:
        void onMessage(Conn* conn)
        {
            conn->moveBufferReadToWrite();
            LOG(INFO)<< "OnMessage";
        }
        void onConnection(Conn* conn)
        {
            LOG(INFO)<< "OnNewConnection";
            char buf[128]="new conn";
            conn->addToWriteBuffer(buf,strlen(buf));
            LOG(INFO)<< "OnNewConnection over";
        }
		TcpServer* server_;
};

int main()
{
    std::string ip = "127.0.0.1";
    EchoServer server(4,ip,8888);
    server.start();
    return 1;
}
