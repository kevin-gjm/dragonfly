#ifndef DRAGONFLY_NET_CONNECTOR_H_
#define DRAGONFLY_NET_CONNECTOR_H_

#include <pthread>
#include <event.h>
#include "uncopyable"
namespace dragonfly
{
    namespace net
    {

        class ConnQueue;

        struct LibeventThread
        {
            pthread_t thread_id;
            struct event_base *base;
            struct event notify_event;
            int notify_send_fd;
            int notify_recv_fd;
            ConnQueue connect_queue;
        };

        class Conn::dragonfly::uncopyable
        {
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

                int GetReadBufferLen()
                {
                    return evbuffer_get_length(read_buf_);
                }
                int GetReadBuffer(char* buffer,int len)
                {
                    return evbuffer_remove(read_buf_,buffer,len);
                }
                int CopyReadBuffer(char *buffer, int len)
                {
                    return evbuffer_copyout(m_ReadBuf, buffer, len);
                }
                int GetWriteBufferLen()
                {
                    return evbuffer_get_length(m_WriteBuf);
                }
                int AddToWriteBuffer(char *buffer, int len)
                {
                    return evbuffer_add(m_WriteBuf, buffer, len);
                }
                void MoveBufferData()
                {
                    evbuffer_add_buffer(m_WriteBuf, m_ReadBuf);
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
                Conn* InsertConn(int fd,LibeventThread* t)
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
                void DeleteConn(Conn* conn)
                {
                    conn->pre_->next_ = conn->next_;
                    conn->next_->pre_ = conn->pre_;
                    delete c;
                }
            private:
                Conn* head_;
                Conn* tail_;
        };
    }
}




#endif //DRAGONFLY_NET_CONNECTOR_H_
