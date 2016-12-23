#ifndef DRAGONFLY_SETUP_H_
#define DRAGONFLY_SETUP_H_

#include <sys/resource.h>

namespace dragonfly{
    namespace setup{
        bool daemonize(int nochdir, int noclose);
        bool set_coredump(void);
        bool set_fdlimit(rlim_t fds);
        bool switch_user(const char* user);
        bool set_signals(void);
        bool pid_write(const char *pid_file);
        void pid_delete(const char *pid_file);
    }
}
#endif //DRAGONFLY_SETUP_H_
