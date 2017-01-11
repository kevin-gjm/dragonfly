#ifndef DRAGONFLY_NET_EVENTLOOP_H_
#define DRAGONFLY_NET_EVENTLOOP_H_
#include <time.h>
#include <string>
#include <event.h>
#include "Connector.h"
#include "Callbacks.h"

namespace dragonfly{
    namespace net{
        class EventLoop:dragonfly::uncopyable
        {
            public:

                static const int EXIT_CODE = -1;

                EventLoop(int count);
                ~EventLoop();

                void loop();

                void quit(timeval *tv);

                // read enough data then call ReadCallback
                void setReadCallback(const DataCallback& cb)
                {
                    read_cb_ =cb;
                }
                // write enough data the call WriteCallback
                void setWriteCallback(const DataCallback& cb)
                {
                    write_cb_ = cb;
                }
                // add a new conn then call ConnectCallback
                void setConnectionCallback(const DataCallback& cb)
                {
                    connect_cb_ = cb;
                }
                // error then call EventCallback
                void setEventCallback(const EventCallback& cb)
                {
                    event_cb_ = cb;
                }
                void setPort(int port)
                {
                    port_ = port;
                }
                void setPort(std::string& ip)
                {
                    ip_ = ip;
                }

            private:
                void setupThread(LibeventThread* thread);
                void* threadProcess(void *arg);
                void notifyHandler(int fd,short which,void* arg);


                void acceptCb(evconnlistener* listener,evutil_socket_t fd,sockaddr* sa,int socklen,void *user_data);

                void bufferReadCb(struct bufferevent* bev,void* data);
                void bufferWriteCb(struct bufferevent* bev,void* data);
                void bufferEventCb(struct bufferevent* bev,short events,void* data);

                DataCallback read_cb_;
                DataCallback write_cb_;
                DataCallback connect_cb_;
                EventCallback event_cb_;

                int thread_count_;
                int port_;
                std::string ip_;
                LibeventThread* main_base_;
                LibeventThread* threads_;


        };
    }
}

#endif //DRAGONFLY_NET_EVENTLOOP_H_
