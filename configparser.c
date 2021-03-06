/*
 *  config parser for GeeXboX FLTK Generator
 *  Copyright (C) 2006  Amir Shalem
 *  Copyright (C) 2007-2008  Mathieu Schroeter
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "configparser.h"

typedef struct item item_t;

struct item {
  item_t *next;
  char *value;
  char *name;
};

struct config {
  item_t *item_list;
  item_t **item_list_end;
  item_t *item_list_tail;
  int shell_escape;
};

void config_destroy(config_t *config)
{
  item_t *item, *item_next;
  for (item = config->item_list; item; item = item_next)
  {
    item_next = item->next;
    free(item->value);
    free(item->name);
    free(item);
  }
  free(config);
}

int config_write(config_t *config, const char *filename)
{
  FILE *f;
  item_t *item;
  char *it;

  f = fopen(filename, "wb");
  if (f == NULL)
    return 0;

  for (item = config->item_list; item; item = item->next)
  {
    if (item->name == NULL)
      {
        fputs(item->value, f);
      }
    else
      {
        if (item->value)
          {
            fputs(item->name, f);
            fputc('=', f);
            if (config->shell_escape)
              {
                fputc('\"', f);
                for (it = item->value; *it; it++)
                  {
                    if (*it == '"')
                      fputc('\\', f);
                    fputc(*it, f);
                  }
                fputc('\"', f);
              }
            else
              {
                fputs(item->value, f);
              }
          }
      }
  }

  fclose(f);
  return 1;
}

static char *config_dup_string(const char *str, size_t str_len, int shell_escape)
{
  char *result =  malloc(str_len + 1);
  char *it;
  const char *str_it;
  int len = 0;

  if (!result)
    return NULL;

  for (it = result, str_it = str; str_it - str < str_len; str_it++, it++)
    {
      if (shell_escape && *str_it == '\\' && *(str_it + 1) == '"')
        {
          *it = *(++str_it);
          len++;
        }
      else
        *it = *str_it;
    }

  if (len)
    result = realloc(result, str_len - len + 1);
  result[str_len - len] = '\0';

  return result;
}

static int config_add_item(config_t *config, const char *value, size_t value_len, const char *name, size_t name_len)
{
  item_t *item;

  if (value_len == 0 && name_len == 0)
    return 1;

  item = malloc(sizeof(item_t));
  if (!item)
    return 0;

  item->next = NULL;
  item->value = config_dup_string(value, value_len, config->shell_escape);
  if (!item->value)
    {
      free(item);
      return 0;
    }

  if (name_len == 0)
    {
      item->name = NULL;
    }
  else
    {
      item->name = config_dup_string(name, name_len, config->shell_escape);
      if (!item->name)
        {
          free(item->value);
          free(item);
          return 0;
        }
    }

  *config->item_list_end = item;
  config->item_list_end = &item->next;
  config->item_list_tail = item;

  return 1;
}

static int config_read_line(config_t *config, char *line)
{
  char *next_line = NULL;

  char *equal;

  char *name, *name_end;

  char *value, *value_end;

  char *comment;

  while (*line)
  {
    /* look for comment */
    for (comment = line; isspace(*comment); comment++)
      ;
    if (*comment == '#')
      break;

    equal = strchr(line, '=');
    if (equal)
      {
        name_end = name = equal-1;
        while (name >= line && !isspace(*name))
          name--;
        name++;

        if (name <= name_end && (name == line || isspace(name[-1])))
        {
          value = equal + 1;
          if (config->shell_escape && (*value == '\"' || *value == '\''))
            {
              char strtype = *value++;
              value_end = value;
              do
                {
                  value_end = strchr(value_end, strtype);
                }
              while (*(value_end - 1) == '\\' && value_end++);

              if (value_end)
                {
                  next_line = value_end-- + 1;
                }
            }
          else
            {
              for (value_end = value; !isspace(*value_end); value_end++)
                ;
              next_line = value_end--;
            }

          if (value_end)
            {
              if (!config_add_item(config, line, name - line, NULL, 0) ||
                  !config_add_item(config, value, value_end - value + 1, name, name_end - name + 1))
                {
                  return 0;
                }

              line = next_line;
              continue;
            }
        }
      }

    break;
  }

  return config_add_item(config, line, strlen(line), NULL, 0);
}

static int config_read(config_t *config, const char *filename)
{
  FILE *f;
  char buf[1024], *line;
  char *marker;

  if (config->item_list)
    return 0;

  f = fopen(filename, "rb");
  if (!f)
    return 0;

  while ((line = fgets(buf, sizeof(buf), f)))
  {
    marker = strstr(line, "\r\n");
    if (marker)
      {
        *marker++ = '\n';
        *marker++ = '\0';
      }

    if (!config_read_line(config, line))
      {
        return 0;
      }
  }

  fclose(f);
  return 1;
}

static config_t *config_create(int shell_escape)
{
  config_t *config = malloc(sizeof(config_t));
  if (config)
    {
      config->shell_escape = shell_escape;
      config->item_list = NULL;
      config->item_list_end = &config->item_list;
      config->item_list_tail = NULL;
    }
  return config;
}

static int config_fix_tail(config_t *config)
{
  item_t *item;

  item = config->item_list_tail;

  if (item && (item->name || !strchr(item->value, '\n')))
    return config_add_item(config, "\n", 1, NULL, 0);

  return 1;
}

config_t *config_open(const char *filename, int shell_escape)
{
  config_t *config;

  config = config_create(shell_escape);
  if (!config)
    return NULL;

  if (!config_read(config, filename) || !config_fix_tail(config))
    {
      config_destroy(config);
      return NULL;
    }

  return config;
}

int config_getvar_location (config_t *config, const char *name, int location, char *dst, size_t dstlen)
{
  item_t *item;
  int current_location = 0;

  for (item = config->item_list; item; item = item->next)
    {
      if (item->name && !strcmp(item->name, name) && (++current_location == location))
        {
          strncpy(dst, item->value, dstlen);
          dst[dstlen-1] = '\0';
          return 1;
        }
    }

  *dst = '\0';
  return 0;
}

int config_getvar_int_location (config_t *config, const char *name, int location, int *dst)
{
  char value[20];
  *dst = 0;
  if (!config_getvar_location(config, name, location, value, sizeof(value)))
    return 0;

  *dst = atoi(value);
  return 1;
}

int config_getvar (config_t *config, const char *name, char *dst, size_t dstlen)
{
  return config_getvar_location(config, name, 1, dst, dstlen);
}

int config_getvar_int (config_t *config, const char *name, int *dst)
{
  return config_getvar_int_location(config, name, 1, dst);
}

int config_setvar_location (config_t *config, const char *name, int location, const char *value)
{
  item_t *item;
  int found = 0;
  int current_location = 0;

  for (item = config->item_list; item; item = item->next)
    {
      if (item->name && !strcmp(item->name, name) && (location == 0 || location == ++current_location))
        {
          found = 1;
          free(item->value);
          if (value)
            {
              item->value = strdup(value);
              if (!item->value)
                return 0;
            }
          else
            item->value = NULL;
        }
    }

  if (found)
    return 1;
  else if (!value)
    return 0;
  else
    return config_add_item(config, value, strlen(value), name, strlen(name)) &&
           config_add_item(config, "\n", 1, NULL, 0);
}

int config_setvar_int_location (config_t *config, const char *name, int location, int value)
{
  char value_string[20];
  sprintf(value_string, "%d", value);

  return config_setvar_location(config, name, location, value_string);
}

int config_setvar (config_t *config, const char *name, const char *value)
{
  return config_setvar_location(config, name, 0, value);
}

int config_setvar_int (config_t *config, const char *name, int value)
{
  return config_setvar_int_location(config, name, 0, value);
}
