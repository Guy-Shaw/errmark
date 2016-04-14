#define _GNU_SOURCE 1

#include <stdlib.h>         // Import exit()
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <ctype.h>          // Import isprint()

#include <errmark.h>
#include <cscript.h>

FILE *errprint_fh;
FILE *dbgprint_fh;

static cmd_t cmdbuf;
static cmd_t *cmd = &cmdbuf;

const char *program_path;
const char *program_name;

bool verbose = false;
bool debug   = false;

enum opt {
    OPT_BASE = 0xf000,
    OPT_COLOR,
    OPT_MARK,
    OPT_COPY,
};

static struct option long_options[] = {
    {"help",     no_argument,       0,  'h'},
    {"version",  no_argument,       0,  'V'},
    {"verbose",  no_argument,       0,  'v'},
    {"debug",    no_argument,       0,  'd'},
    {"mark",     required_argument, 0,  OPT_MARK},
    {"color",    required_argument, 0,  OPT_COLOR},
    {"copy",     required_argument, 0,  OPT_COPY},
    {0, 0, 0, 0 }
};

static const char usage_text[] =
    "Options:\n"
    "  --help|-h|-?     Show this help message and exit\n"
    "  --version        Show ush version information and exit\n"
    "  --verbose|-v     verbose\n"
    "  --debug|-d       debug\n"
    "  --mark|-m        <mark-specification>\n"
    "      Where mark-specification = fd:start:end.\n"
    "  --color          <color-name>\n"
    "  -c|copy          <filename>\n";


static const char version_text[] =
    "0.1\n"
    ;

static const char copyright_text[] =
    "Copyright (C) 2016 Guy Shaw\n"
    "Written by Guy Shaw\n"
    ;

static const char license_text[] =
    "License GPLv3+: GNU GPL version 3 or later"
    " <http://gnu.org/licenses/gpl.html>.\n"
    "This is free software: you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n"
    ;

static void
fshow_program_version(FILE *f)
{
    fputs(version_text, f);
    fputc('\n', f);
    fputs(copyright_text, f);
    fputc('\n', f);
    fputs(license_text, f);
    fputc('\n', f);
}

static void
show_program_version(void)
{
    fshow_program_version(stdout);
}

static void
usage(void)
{
    eprintf("usage: %s [ <options> ]\n", program_name);
    eprint(usage_text);
}

static inline bool
is_long_option(const char *s)
{
    return (s[0] == '-' && s[1] == '-');
}

static inline char *
vischar_r(char *buf, size_t sz, int c)
{
    if (isprint(c)) {
        buf[0] = c;
        buf[1] = '\0';
    }
    else {
        snprintf(buf, sz, "\\x%02x", c);
    }
    return (buf);
}

void
opt_mark(char *mark_specs)
{
    bool mark_ok;

    mark_ok = parse_mark_specs(mark_specs);
    if (verbose || !mark_ok) {
        fputs("--mark='", stderr);
        fshow_str(stderr, mark_specs);
        fputs("'\n", stderr);
    }
    if (!mark_ok) {
        exit(2);
    }
}

void
fshow_color_table(FILE *f, color_esc_t *color_table)
{
    color_esc_t *ent;
    int i;

    for (i = 0; ent = &color_table[i], ent->name != NULL; ++i) {
        if (i >= 1) {
            fputs(" ", f);
        }
        fputs(ent->name, f);
    }
}

void
opt_color(char const *color_name)
{
    color_esc_t *color_ent = lookup_color(color_name);
    if (!color_ent) {
        extern color_esc_t *normal_colors;
        extern color_esc_t *bright_colors;

        fprintf(stderr, "Unknown color, '%s'.\n", color_name);
        fputs("Known color names are:\n", stderr);
        fputs("    ", stderr);
        fshow_color_table(stderr, normal_colors);
        fputs("\n", stderr);
        fputs("    ", stderr);
        fshow_color_table(stderr, bright_colors);
        fputs("\n", stderr);
        exit(2);
    }

    if (!setmark(2, color_ent->esc_start, color_ent->esc_end)) {
        exit(2);
    }
}

int
main(int argc, char * const *argv)
{
    extern char *optarg;
    extern int optind, opterr, optopt;
    int option_index;
    int err_count;
    int optc;
    int rv;

    set_print_fh();
    program_path = *argv;
    program_name = sname(program_path);
    option_index = 0;
    err_count = 0;
    opterr = 0;

    while (true) {
        int this_option_optind;

        if (err_count > 10) {
            eprintf("%s: Too many option errors.\n", program_name);
            break;
        }

        this_option_optind = optind ? optind : 1;
        optc = getopt_long(argc, argv, "+hVdv", long_options, &option_index);

        if (optc == -1) {
            break;
        }

        if (debug) {
            dbg_printf("optc=0x%x", optc);
            if (optc >= 0 && optc <= 127 && isprint(optc)) {
                dbg_printf("='%c'", optc);
            }
            eprintf("\n");
        }

        rv = 0;
        if (optc == '?' && optopt == '?') {
            optc = 'h';
        }

        switch (optc) {
        case 'V':
            show_program_version();
            exit(0);
            break;
        case 'h':
            fputs(usage_text, stdout);
            exit(0);
            break;
        case 'd':
            debug = true;
            break;
        case 'v':
            verbose = true;
            break;
        case OPT_MARK:
            opt_mark(optarg);
            break;
        case OPT_COLOR:
            opt_color(optarg);
            break;
        case OPT_COPY:
            cmd->copy_fname = optarg;
            break;
        case '?':
            eprint(program_name);
            eprint(": ");
            if (is_long_option(argv[this_option_optind])) {
                eprintf("unknown long option, '%s'\n",
                    argv[this_option_optind]);
            }
            else {
                char chrbuf[10];
                eprintf("unknown short option, '%s'\n",
                    vischar_r(chrbuf, sizeof (chrbuf), optopt));
            }
            ++err_count;
            break;
        default:
            eprintf("%s: INTERNAL ERROR: unknown option, '%c'\n",
                program_name, optopt);
            exit(2);
            break;
        }

        if (rv) {
            ++err_count;
        }
    }

    if (err_count != 0) {
        usage();
        exit(1);
    }

    verbose = verbose || debug;
    cmd->verbose = verbose;
    cmd->debug   = debug;

    if (argc == 0) {
        eprintf("%s: Must supply at least a command name.\n", program_name);
        usage();
        exit(2);
    }

    cmd->cmd_path = argv[optind];
    cmd->cmd_name = sname(cmd->cmd_path);
    cmd->argc = argc - optind;
    cmd->argv = argv + optind;
    cmd->nullify = true;
    cmd->slow    = true;

    if (verbose) {
        fshow_str_array(stderr, cmd->argc, cmd->argv);
    }

    cmd->copy_fh = NULL;
    if (cmd->copy_fname != NULL) {
        cmd->copy_fh = fopen(cmd->copy_fname, "w");
        if (cmd->copy_fh == NULL) {
            eprintf("open('%s', \"w\") failed.\n", cmd->copy_fname);
            exit(2);
        }
    }

    cmd->child_status = errmark_run_program(cmd);
    exit(cmd->child_status >> 8);
}
