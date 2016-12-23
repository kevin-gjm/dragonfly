#ifndef DRAGONFLY_DRAGONFLY_H_
#define DRAGONFLY_DRAGONFLY_H_

#include "config.h"

#ifdef HAVE_GLOG_H
#include <glog/logging.h>
#endif

#include <stdio.h>

#define PACKAGE "DragonFly"
#define VERSION "v0.0.1"

typedef enum
{
    conn_listening,
    conn_read,
    conn_write,
    conn_closing,
    conn_closed,
    conn_max,
} conn_states;


typedef enum
{
    DRAGONFLY_SUCCESS,
    DRAGONFLY_ERROR,
    DRAGONFLY_NOT_FOUND,
    DRAGONFLY_IO_WAIT,
    DRAGONFLY_MAX_RETURN,
} return_t;

struct stats
{
    uint64_t total_conns;
};

struct settings
{
    int maxconns;
    int port;
    int verbose;
    int num_threads;
    bool do_daemonize;
    int maxcore;
    char *inter;
    int reqs_per_event;

};
#endif // DRAGONFLY_DRAGONFLY_H_
