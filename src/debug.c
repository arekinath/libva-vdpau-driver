/*
 *  debug.c - Debugging utilities
 *
 *  libva-vdpau-driver (C) 2009-2011 Splitted-Desktop Systems
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "sysdeps.h"
#include "debug.h"
#include "utils.h"
#include <stdarg.h>
#include <unistd.h>

static void do_vfprintf(FILE *fp, const char *msg, va_list args)
{
    /* XXX: use another printf() function, e.g. a valgrind one to
       maintain correct control flow */
    vfprintf(fp, msg, args);
}

static void do_fprintf(FILE *fp, const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    do_vfprintf(fp, msg, args);
    va_end(args);
}

void vdpau_error_message(const char *msg, ...)
{
    va_list args;

    do_fprintf(stdout, "%s: error: ", PACKAGE_NAME);
    va_start(args, msg);
    do_vfprintf(stdout, msg, args);
    va_end(args);
}

void vdpau_information_message(const char *msg, ...)
{
    va_list args;

    do_fprintf(stdout, "%s: ", PACKAGE_NAME);
    va_start(args, msg);
    do_vfprintf(stdout, msg, args);
    va_end(args);
}

static int debug_enabled(void)
{
    static int g_debug_enabled = -1;
    if (g_debug_enabled < 0) {
        if (getenv_yesno("VDPAU_VIDEO_DEBUG", &g_debug_enabled) < 0)
            g_debug_enabled = 0;
    }
    return g_debug_enabled;
}

static FILE *debug_file(void)
{
    static FILE *g_debug_file = NULL;
    if (g_debug_file == NULL) {
        g_debug_file = fopen("/tmp/libva-vdpau-debug.log", "a");
    }
    return g_debug_file;
}

void debug_message(const char *msg, ...)
{
    va_list args;
    FILE *f = debug_file();

    if (!debug_enabled())
        return;

    do_fprintf(f, "%s(%d): ", PACKAGE_NAME, getpid());
    va_start(args, msg);
    do_vfprintf(f, msg, args);
    va_end(args);
    fflush(f);
}

static int g_trace_is_new_line  = 1;
static int g_trace_indent       = 0;

int trace_enabled(void)
{
    static int g_trace_enabled = -1;
    if (g_trace_enabled < 0) {
        if (getenv_yesno("VDPAU_VIDEO_TRACE", &g_trace_enabled) < 0)
            g_trace_enabled = 0;
    }
    return g_trace_enabled;
}

static int trace_indent_width(void)
{
    static int g_indent_width = -1;
    if (g_indent_width < 0) {
        if (getenv_int("VDPAU_VIDEO_TRACE_INDENT_WIDTH", &g_indent_width) < 0)
            g_indent_width = 4;
    }
    return g_indent_width;
}

void trace_indent(int inc)
{
    g_trace_indent += inc;
}

void trace_print(const char *format, ...)
{
    va_list args;
    FILE *f = debug_file();

    if (g_trace_is_new_line) {
        int i, j, n;
        fprintf(f, "%s: ", PACKAGE_NAME);
        n = trace_indent_width();
        for (i = 0; i < g_trace_indent; i++) {
            for (j = 0; j < n / 4; j++)
                fprintf(f, "    ");
            for (j = 0; j < n % 4; j++)
                fprintf(f, " ");
        }
    }

    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);

    g_trace_is_new_line = (strchr(format, '\n') != NULL);

    if (g_trace_is_new_line)
        fflush(f);
}
