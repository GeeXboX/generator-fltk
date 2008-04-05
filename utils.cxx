/*
 *  Utilities for GeeXboX FLTK Generator
 *  Copyright (C) 2005-2007  Amir Shalem
 *                           Mathieu Schroeter
 *
 *   This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *   This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "utils.h"

#include <stdio.h>
#include <ctype.h> /* isspace */

void replace_char (char *str, const char o, const char n)
{
    for (; *str; str++)
	if (*str == o)
	    *str = n;
}

std::string get_str_nospace(char *buf, int loc)
{
    int i, len;
    char *start, *end, *str;
    char buf2[256];
    std::string res;

    str = buf;
    for (i = 1; i <= loc; i++) {
        while (isspace(*str) && *str != '\n' && *str != '\0')
            str++;

        start = str;
        while (!isspace(*str) && *str != '\n' && *str != '\0')
            str++;
        end = str;

        if (i == loc) {
            if (end - start + 1 <= (signed)sizeof(buf2))
                len = end - start;
            else
                len = sizeof(buf2) - 1;

            snprintf(buf2, len + 1, "%s", start);
            buf2[len] = '\0';
            res = buf2;
            break;
        }
    }
    return res;
}
