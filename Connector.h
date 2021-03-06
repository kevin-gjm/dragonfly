#ifndef DRAGONFLY_NET_CONNECTOR_H_
#define DRAGONFLY_NET_CONNECTOR_H_

#include <pthread.h>
#include <event.h>
#include <glog/logging.h>
#include "uncopyable.h"
namespace dragonfly
{
    namespace net
    {

        class TcpServer;
        class ConnQueue;
        struct LibeventThread;

        class Conn: dragonfly::uncopyable
        {
            friend class ConnQueue;
            friend class TcpServer;
            public:
                Conn(int fd=0)
                    :fd_(fd),
                    thread_(NULL),
                    read_buf_(NULL),
                    write_buf_(NULL),
                    pre_(NULL),
                    next_(NULL)
            {}

                LibeventThread* getThread() {return thread_;}

                int getFd(){return fd_;}

                int getReadBufferLen()
                {
                    return evbuffer_get_length(read_buf_);
                }
                int getReadBuffer(char* buffer,int len)
                {
                    return evbuffer_remove(read_buf_,buffer,len);
                }
                int copyReadBuffer(char *buffer, int len)
                {
                    return evbuffer_copyout(read_buf_, buffer, len);
                }
                int getWriteBufferLen()
                {
                    return evbuffer_get_length(write_buf_);
                }
                int addToWriteBuffer(char *buffer, int len)
                {
                    LOG(INFO)<<"add write buffer";
                    return evbuffer_add(write_buf_, buffer, len);
                }
                void moveBufferReadToWrite()
                {
                    evbuffer_add_buffer(write_buf_, read_buf_);
                }
            private:
                int fd_;
                LibeventThread* thread_;
                evbuffer *read_buf_;
                evbuffer *write_buf_;

                Conn *pre_;
                Conn *next_;
        };

        class ConnQueue
        {
            public:
                ConnQueue()
                    :head_(new Conn(0)),
                    tail_(new Conn(0))
            {
                head_->pre_ = tail_->next_ = NULL;
                head_->next_ = tail_;
                tail_->pre_ = head_;
            }
                ~ConnQueue()
                {
                    Conn *tcur,*tnext;
                    tcur = head_;
                    while(tcur != NULL)
                    {
                        tnext = tcur->next_;
                        delete tcur;
                        tcur = tnext;
                    }
                }
                Conn* insertConn(int fd,LibeventThread* t)
                {
                    Conn *c = new Conn(fd);
                    c->thread_ = t;
                    Conn *next = head_->next_;

                    c->pre_ = head_;
                    c->next_ = head_->next_;
                    head_->next_ = c;
                    next->pre_ = c;
                    return c;
                }
                void deleteConn(Conn* conn)
                {
                    conn->pre_->next_ = conn->next_;
                    conn->next_->pre_ = conn->pre_;
                    delete conn;
                }
            private:
                Conn* head_;
                Conn* tail_;
        };
        struct LibeventThread
        {
            pthread_t thread_id;
            struct event_base *base;
            struct event notify_event;
            int notify_send_fd;
            int notify_recv_fd;
			///TODO:
			/// not use this.
			/// every loop have one queue,but not use

            ConnQueue connect_queue;

            TcpServer *tcp_connect;
        };

    }
}




#endif //DRAGONFLY_NET_CONNECTOR_H_
