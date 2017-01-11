#include "EventLoop.h"

//TODO:
//if user not use glog then use fprintf(stderr/stdout....)
#ifdef HAVE_GLOG_H
#include <glog/logging.h>
#endif

using namespace dragonfly::net;

EventLoop::EventLoop(int count)
    :read_cb_(NULL),
    write_cb_(NULL),
    connect_cb_(NULL),
    event_cb_(NULL),
    thread_count_(count),
    port_(0),
    ip_(),
    main_base_(new LibeventThread);
threads_(new LibeventThread[thread_count_])
{
    main_base_->thread_id = pthread_self();
    main_base_->base = event_base_new();

    for(int i=0;i< thread_count_;i++)
    {
        setupThread(&threads_[i]);
    }
    LOG(INFO) << "EventLoop ctor";
}

EventLoop::~EventLoop()
{
    quit(NULL);
    event_base_free(main_base_->base);
    for(int i =0;i< thread_count_;i++)
    {
        event_base_free(threads_[i].base);
    }
    delete main_base_;
    delete[] threads_;
    LOG(INFO)<< "EventLoop dtor";
}
void EventLoop::setupThread(LibeventThread* thread)
{
    thread->base = event_base_new();
    if(NULL == thread->base)
        LOG(FATAL)<<"event base new error";
    int fds[2];
    if(pipe(fds))
        LOG(FATAL)<<"create pipe error";
    thread->notify_send_fd = fds[1];
    thread->notify_recv_fd = fds[0];

    event_set(&thread->notify_event,thread->notify_recv_fd,EV_READ|EV_PERSIST,notifyHandler,thread);
    event_base_set(thread->base,&thread->notify_event);
    if(event_add(&thread->notify_event,0) == -1)
        LOG(FATAL)<<"Can not monitor notify pipe";
}

void EventLoop::notifyHandler(int fd,short which,void* arg)
{
    LibeventThread * thread = (LibeventThread*)arg;

    int pipefd = thread->notify_recv_fd;
    evutil_socket_t confd;
    if(-1 == read(pipefd,&confd,sizeof(evutil_socket_t)))
    {
        LOG(ERROR)<<"pipe read error";
        return ;
    }
    if(EXIT_CODE == confd)
    {
        event_base_loopbreak(thread->base);
        LOG(INFO) << "notify pipe recv EXIT_CODE base loopbreak";
    }

    struct bufferevent *bev;
    bev = bufferevent_socket_new(thread->base,confd,BEV_OPT_CLOSE_ON_FREE);
    if(!bev)
    {
        LOG(ERROR)<<"bufferevent create with evutil_socket_t  error";
        event_base_loopbreak(thread->base);
        return;
    }

    // insert new connect to the thread's connection queue
    Conn *conn = thread->connect_queue.InsertConn(confd,thread);

    bufferevent_setcb(bev,bufferReadCb,bufferWriteCb,bufferEventCb,conn);
    bufferevent_enable(bev,EV_READ);
    bufferevent_enable(bev,EV_WRITE);

    if(connect_cb_)
        connect_cb_(conn);
}

void* EventLoop::threadProcess(void *arg)
{
    LibeventThread *thread = (LibeventThread*)arg;
    LOG(INFO)<<"thread "<<thread->thread_id <<" started!";
    event_base_dispatch(thread->base);
}
void EventLoop::bufferReadCb(struct bufferevent* bev,void* data)
{
    Conn* conn = (Conn*)data;
    conn->read_buf_ = bufferevent_get_input(bev);
    conn->write_buf_ = bufferevent_get_output(bev);
    LOG(INFO)<<"have data to read";
    if(read_cb_)
        read_cb_(conn);
}
void EventLoop::bufferWriteCb(struct bufferevent* bev,void* data)
{
    Conn* conn = (Conn*)data;
    conn->read_buf_ = bufferevent_get_input(bev);
    conn->write_buf_ = bufferevent_get_output(bev);
    LOG(INFO)<<"have data to send";
    if(write_cb_)
        write_cb_(conn);
}
void EventLoop::bufferEventCb(struct bufferevent* bev,short events,void* data)
{
    LOG(ERROR) <<"some error happened."<<events;
    Conn *conn = (Conn*)data;
    if(event_cb_)
        event_cb_(conn,events);
    conn->getThread()->connect_queue_.deleteConn(conn);
    bufferevent_free(bev);
}
void EventLoop::loop()
{
    evconnlistener *listener;

    sockaddr_in sin;
    memset(&sin,0,sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port_);
    if(ip_ != std::string())
    {
        if(inet_pton(AF_INET,ip,&sin->sin_addr) <= 0)
            LOG(FATAL) << "socket ip error";
    }

    listener = evconnlistener_new_bind(main_base_->base,acceptCb,(void*)this,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,-1,(sockaddr*)&sin,sizeof(sockaddr_in));
    if(NULL == listener)
        LOG(FATAL) << "create listener error ";

    for(int i=0;i<thread_count_;i++)
    {
        pthread_create(&threads_[i].thread_id,NULL,threadProcess,(void*)&threads_[i]);
    }

    event_base_dispatch(main_base_->base);

    evconnlistener_free(listener);
}

void EventLoop::quit(timeval *tv)
{
    int contant = EXIT_CODE;
    for(int i=0;i<thread_count_;i++)
    {
        write(threads_[i].notify_send_fd,&contant,sizeof(int));
    }

    event_base_loopexit(main_base_->base,tv);
}
void EventLoop::acceptCb(evconnlistener* listener,evutil_socket_t fd,sockaddr* sa,int socklen,void *user_data)
{
    EventLoop *server = (EventLoop*) user_data;
    int num = rand() % thread_count_;
    write(threads_[num].notify_send_fd,&fd,sizeof(evutil_socket_t));
}