#include "dragonfly.h"
#include "setup.h"

using namespace dragonfly;

struct settings settings;
struct stats stats;

static void settings_init(void)
{
    settings.port = 8888;
    settings.maxconns = 1024;
    settings.verbose = 0;
    settings.num_threads = 4;
    settings.reqs_per_event = 20;
    settings.do_daemonize = false;
    settings.maxcore = 1;
    settings.inter = NULL;
}
static void log_init(const char* log_name)
{
    google::InitGoogleLogging(log_name);

    FLAGS_log_dir = "./";
    //    FLAGS_minloglevel=google::INFO;
    //    google::SetLogDestination(google::INFO,"./log/INFO");
    //    google::SetLogDestination(google::WARNING,"./log/WARNING");
    //    google::SetLogDestination(google::ERROR,"./log/ERROR");

    //pass 60s will write log buffer to file
//    FLAGS_logbufsecs = 60;
    FLAGS_logbufsecs = 1;

    ////max single log file size /M
    //FLAGS_max_log_size = 10;

    //FLAGS_logtostderr = true;
    //FLAGS_logtostderr = false;

}
static void usage(void)
{
    printf("%s-%s\n",PACKAGE,VERSION);
    printf(
        "           -p <num>     TCP port number to listen on(default: 8888)\n"
        "           -r           maximize core file limit\n"
        "           -v           verbose (log level)\n "
        "           -d           run as a daemon\n"
        "           -l <addr>    interface to listen on (default: INADDR_ANY, all addresses)\n"
        "           -P <file>    save PID in <file>,only used with -d option\n"
        "           -t <nnum>    number of threads to use(default:4)\n"
        "           -R           Maximum number of requests per event, limits the number of\n"
        "                        requests process for a given connection to prevent \n"
        "                        starvation (default: 20)\n"
        "           -L <dir>     save log in <dir> directory\n"
        "           -h           show help information\n"
        );
}
int main(int argc,char** argv)
{
    int c;
    char *username = NULL;
    char *pid_file = NULL;
    char *log_dir = NULL;
    struct passwd *pw;
    struct rlimit rlim;

    //setup::set_signals();

    settings_init();
    setbuf(stderr,NULL);

    while(-1 != (c = getopt(argc,argv,
                    "p:"  /*TCP port number to listen on*/
                    "r"   /*maximize core file limit*/
                    "v"   /*verbose*/
                    "d"   /*daemon mode*/
                    "l:"  /*interface to listen on*/
                    "P:"  /*save PID in file*/
                    "t:"  /*threads*/
                    "R:"  /*max requests per event*/
                    "L:"  /*log file dir*/
                    "h"   /*get help*/
                    )))
    {
        switch(c)
        {
            case 'p':
                settings.port = atoi(optarg);
                break;
            case 'r':
                settings.maxcore = 1;
                break;
            case 'v':
                settings.verbose++;
                break;
            case 'd':
                settings.do_daemonize = true;
                break;
            case 'l':
                if (settings.inter != NULL) {
                    if (strstr(settings.inter, optarg) != NULL) {
                        break;
                    }
                    size_t len = strlen(settings.inter) + strlen(optarg) + 2;
                    char *p = (char *)malloc(len);
                    if (p == NULL) {
                        fprintf(stderr, "Failed to allocate memory\n");
                        return 1;
                    }
                    snprintf(p, len, "%s,%s", settings.inter, optarg);
                    free(settings.inter);
                    settings.inter = p;
                } else {
                    settings.inter= strdup(optarg);
                }
                break;
            case 'P':
                pid_file = optarg;
                printf("pid_file:%s\n",pid_file);
                break;
            case 'L':
                log_dir = optarg;
                printf("log_dir:%s\n",log_dir);
                break;
            case 't':
                settings.num_threads = atoi(optarg);
                if(settings.num_threads <= 0)
                {
                    fprintf(stderr,"Number of threads must be greater than 0\n");
                    return 1;
                }
                if(settings.num_threads > 64)
                {
                    fprintf(stderr, "WARNING: Setting a high number of worker"
                            "threads is not recommended.\n"
                            " Set this value to the number of cores in"
                            " your machine or less.\n");
                }
                break;
            case 'R':
                settings.reqs_per_event = atoi(optarg);
                if(settings.reqs_per_event == 0)
                {
                    fprintf(stderr, "Number of requests per event must be greater than 0\n");
                    return 1;
                }
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr,"Illegal argument \"%c\"\n",c);
                return 1;
        }
    }

    if(settings.do_daemonize)
    {
        if(!setup::daemonize(1,0))
            return 1;
    }
    log_init(argv[0]);
    if(0 == settings.verbose || 1 == settings.verbose)
    {
        FLAGS_minloglevel = google::ERROR;
    }else if(2 == settings.verbose)
    {
        FLAGS_minloglevel = google::WARNING;
    }else
    {
        FLAGS_minloglevel = google::INFO;
    }
    if(log_dir != NULL)
    {
        FLAGS_log_dir = log_dir;
    }
    if(!setup::set_fdlimit(settings.maxconns))
    {
        return 1;
    }
    if(settings.maxcore != 0)
    {
        setup::set_coredump();
    }
    if(pid_file != NULL)
    {
        setup::pid_write(pid_file);
    }
    google::ShutdownGoogleLogging();
    return 0;
}

