#ifndef DRAGONFLY_NET_EVENTLOOP_H_
#define DRAGONFLY_NET_EVENTLOOP_H_
#include <time.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
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
                void setIp(std::string& ip)
                {
                    ip_ = ip;
                }

            private:
                void setupThread(LibeventThread* thread);
                static void* threadProcess(void *arg);
                static void notifyHandler(int fd,short which,void* arg);


                static void acceptCb(evconnlistener* listener,evutil_socket_t fd,sockaddr* sa,int socklen,void *user_data);

                static void bufferReadCb(struct bufferevent* bev,void* data);
                static void bufferWriteCb(struct bufferevent* bev,void* data);
                static void bufferEventCb(struct bufferevent* bev,short events,void* data);

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
