
#ifndef __AC_ARGS__
#define __AC_ARGS__

typedef struct {
    int size;
    char **app_args;
    char *app_filename;
#ifdef USE_GDB
    unsigned gdb_port = 5000;
#endif
} args_t;

// Read model options before application
void ac_init_opts(int ac, char *av[]);

// Initialize arguments for application
args_t ac_init_args(int ac, char *av[]);

#endif // __AC_ARGS__
