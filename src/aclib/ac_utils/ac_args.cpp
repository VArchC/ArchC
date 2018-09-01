#include "ac_args.H"
#include "ac_utils.H"

using namespace std;

// Read model options before application
void ac_init_opts(int ac, char *av[]) {

    extern const char *project_name;
    extern const char *project_file;
    extern const char *archc_version;
    extern const char *archc_options;

    if (ac > 1) {

        if (!strncmp(av[1], "--help", 6)) {
            cerr << "This is a " << project_name
                 << " architecture simulator generated by ArchC.\n";
            cerr << "For more information visit http://www.archc.org\n\n";
            cerr << "Usage: " << av[0] << " [options]\n\n";
            cerr << "Options:\n";
            cerr << "  --help                  Display this help message\n";
            cerr << "  --version               Display ArchC version and "
                    "options used when built\n";
            cerr << "  --load=<prog_path>      Load target application\n";
            cerr << "  -- <prog_path>          Load target application\n";
            cerr << "  --trace-cache=<cache>,<file> Trace cache access\n";
#ifdef USE_GDB
            cerr << "  --port=<port>           Set the GDB port\n";
#endif /* USE_GDB */
            exit(1);
        }

        if (!strncmp(av[1], "--version", 9)) {
            cout << project_name << " simulator generated by ArchC "
                 << archc_version << " from " << project_file
                 << " with options (" << archc_options << ")" << endl;
            exit(0);
        }
    }
}

// Initialize arguments for application
args_t ac_init_args(int ac, char *av[]) {

    int size;
    char *appname = 0;
    int ac_argc;
    char **ac_argv;

    ac_argc = ac - 1; // Skiping program's name
    ac_argv = av;

    // struct for return
    args_t args;

    // Check if "help" or "version" was called
    ac_init_opts(ac, av);

    while (ac > 1) {
        size = strlen(av[1]);

        // Checking if an application needs to be loaded
        if ((size > 11) &&
            (!strncmp(av[1], "--load_obj=",
                      11))) { // Loading hex file in the ArchC Format
            appname = (char *)malloc(size - 10);
            strcpy(appname, av[1] + 11);
            ac_argv[1] = appname;
            appfilename = (char *)malloc(sizeof(char) * (size - 6)); // ?
            strcpy(appfilename, appname);
            // ac_load_obj( appname );
        } else if ((size > 7) && (!strncmp(av[1], "--load=",
                                           7))) { // Loading a binary app with
                                                  // former way: --load=<bin>
            appname = (char *)malloc(size - 6);
            strcpy(appname, av[1] + 7);
            ac_argv[1] = appname;
            appfilename = (char *)malloc(sizeof(char) * (size - 6));
            strcpy(appfilename, appname);

        } else if ((size == 2) &&
                   (!strncmp(av[1], "--",
                             2))) { // Loading a binary with new way: -- <bin>
            ac_argc--;
            ac--;

            // Remove -- from ac_argv
            for (int i = 1; i < ac; i++) {
                ac_argv[i] = ac_argv[i + 1];
            }

            size = strlen(av[1]);
            appname = (char *)malloc(sizeof(char) * (size + 1)); // Allowing enough room for \0
            strcpy(appname, av[1]);
            cerr << appname << endl;
            // ac_argv[1] = appname;
            appfilename = (char *)malloc(sizeof(char) * (size + 1)); // Allowing enough room for \0
            strcpy(appfilename, appname);

            av++;
        }
#ifdef USE_GDB
        else if (size >= 7 &&
                 (!strncmp(av[1], "--port=", 7))) { // Enable GDB support
            unsigned port = atoi(av[1] + 7);
            if (port > 1024)
                args.gdb_port = port;
        }
#endif /* USE_GDB */

        else if ((size > 14) && (!strncmp(av[1], "--trace-cache=", 14))) {
            char *comma = strchr(av[1], ',');
            if (comma == NULL) {
                std::cerr << "Error: invalid argument syntax.\n";
                exit(EXIT_FAILURE);
            }
            std::string cache_name(av[1] + 14, comma);
            std::string file_name(comma + 1, av[1] + size);
            ac_cache_traces[cache_name] = new std::ofstream(file_name.c_str());
            if (!ac_cache_traces[cache_name]) {
                std::cerr << "Error opening file: " << file_name << "\n";
                exit(EXIT_FAILURE);
            }
            // Remove this parameter from the list and reset the loop
            for (int i = 1; i <= ac; i++) {
                av[i] = av[i + 1];
            }

            ac_argc--;
            ac--;
            continue;
        }

        ac--;
        av++;
    }

    if (!appname) {
        AC_ERROR("No application provided.");
        AC_ERROR("Use --load=<prog_path> or -- <prog_path> to load a target "
                 "application.");
        AC_ERROR("Try --help for more options.");
        exit(1);
    }
    ac_argv++;

    args.size = ac_argc;
    args.app_args = ac_argv;
    args.app_filename = appfilename;
    return args;
}

