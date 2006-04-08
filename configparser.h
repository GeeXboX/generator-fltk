/*
 *  config parser for GeeXboX FLTK Generator
 *  Copyright (C) 2006  Amir Shalem
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

#ifndef configparser_h
#define configparser_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct config config_t;

config_t *config_open(const char *filename, int shell_escape);
int config_write(config_t *config, const char *filename);
void config_destroy(config_t *config);

int config_getvar (config_t *config, const char *var, char *dst, size_t dstlen);
int config_getvar_int (config_t *config, const char *var, int *dst);
int config_setvar (config_t *config, const char *var, const char *value);
int config_setvar_int (config_t *config, const char *var, int value);

#ifdef __cplusplus
}
#endif

#endif
