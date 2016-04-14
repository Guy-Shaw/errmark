/*
 * Filename: lookup-color-name.c
 * Library: libcscript
 * Brief: color to ANSI escape; lookup a color name; return a color table entry.
 *
 * Copyright (C) 2016 Guy Shaw
 * Written by Guy Shaw <gshaw@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cscript.h>
#include <string.h>     // Import strcmp(), strncmp()

static color_esc_t normal_colors_table[] = {
    { "black",   "\e[0;30m", "\e[m" },
    { "red",     "\e[0;31m", "\e[m" },
    { "green",   "\e[0;32m", "\e[m" },
    { "yellow",  "\e[0;33m", "\e[m" },
    { "blue",    "\e[0;34m", "\e[m" },
    { "magenta", "\e[0;35m", "\e[m" },
    { "cyan",    "\e[0;36m", "\e[m" },
    { "white",   "\e[0;37m", "\e[m" },
    { 0, 0, 0 }
};

static color_esc_t bright_colors_table[] = {
    { "black",   "\e[1;30m", "\e[m" },
    { "red",     "\e[1;31m", "\e[m" },
    { "green",   "\e[1;32m", "\e[m" },
    { "yellow",  "\e[1;33m", "\e[m" },
    { "blue",    "\e[1;34m", "\e[m" },
    { "magenta", "\e[1;35m", "\e[m" },
    { "cyan",    "\e[1;36m", "\e[m" },
    { "white",   "\e[1;37m", "\e[m" },
    { 0, 0, 0 }
};

color_esc_t *normal_colors = &normal_colors_table[0];
color_esc_t *bright_colors = &bright_colors_table[0];

color_esc_t *
lookup_color(char const *name)
{
    color_esc_t *color_table;
    color_esc_t * ent;
    int i;

    if (name == NULL) {
        return (NULL);
    }

    if (strncmp(name, "bright-", 7) == 0) {
        name += 7;
        color_table = bright_colors;
    }
    else {
        color_table = normal_colors;
    }

    for (i = 0; ent = &color_table[i], ent->name != NULL; ++i) {
        if (strcmp(name, ent->name) == 0) {
            return (ent);
        }
    }

    return (NULL);
}
