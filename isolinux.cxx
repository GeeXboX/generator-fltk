/*
 *  Isolinux support for GeeXboX FLTK Generator
 *  Copyright (C) 2008 Mathieu Schroeter
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

#include "config.h"
#include "isolinux.h"
#include "utils.h"

struct isolinux_s {
    std::string name;
    std::string value;
    struct isolinux_s *prev;
    struct isolinux_s *next;
    struct isolinux_s *parent;
    struct isolinux_s *child;
};

isolinux_t *isolinux_load(const char *path)
{
    FILE *f;
    char buf[256];
    isolinux_t *first_line = NULL;
    isolinux_t *line = NULL;

    f = fopen(path, "rb");
    if (!f)
        return NULL;

    while (fgets(buf, sizeof(buf), f)) {
        if (!line) {
            line = new isolinux_t;
            line->prev = NULL;
            line->next = NULL;
            line->parent = NULL;
            line->child = NULL;
            first_line = line;
        }
        else {
            line->next = new isolinux_t;
            line->next->prev = line;
            line = line->next;
            line->next = NULL;
            line->parent = NULL;
            line->child = NULL;
        }

        /* sub-property */
        if (strlen(buf) > 2 && *buf == ' ' && *(buf + 1) == ' ') {
            line->name = get_str_nospace(buf, 1);
            line->value = std::string(buf + line->name.length() + 3);
            if (line->value.length())
                line->value.resize(line->value.length() - 1);

            /* first child ? */
            if (!line->prev->child && !line->prev->parent) {
                line->parent = line->prev;
                line->prev->child = line;
                line->prev = NULL;
            }
            else
                line->parent = line->prev->parent;

            continue;
        }

        /* return to root if the prev is a sub-property */
        if (line->prev && line->prev->parent) {
            line->prev->parent->next = line;
            line->prev->next = NULL;
            line->prev = line->prev->parent;
        }

        /* property */
        if (*buf != ' ') {
            line->name = get_str_nospace(buf, 1);
            line->value = std::string(buf + line->name.length() + 1);
            if (line->value.length())
                line->value.resize(line->value.length() - 1);
        }
        /* unknown (empty line ?) */
        else {
            line->name = std::string(buf);
            line->value = "";
        }
    }

    fclose(f);
    return first_line;
}

void isolinux_unload(isolinux_t *isolinux)
{
    isolinux_t *next;

    if (!isolinux);
        return;

    while (isolinux) {
        if (isolinux->child)
            isolinux_unload(isolinux->child);

        next = isolinux->next;
        delete isolinux;
        isolinux = next;
    }
}

std::string isolinux_get_default(isolinux_t *isolinux)
{
    std::string res = "";

    if (!isolinux)
        return res;

    while (isolinux && res.empty()) {
        if (isolinux->child && !isolinux->parent)
            res = isolinux_get_default(isolinux->child);
        else if (isolinux->parent
                 && isolinux->name == "MENU"
                 && isolinux->value == "DEFAULT")
        {
            res = isolinux->parent->value;
            break;
        }

        isolinux = isolinux->next;
    }

    return res;
}

void isolinux_set_default(isolinux_t *isolinux, std::string label)
{
    std::string current_def;
    isolinux_t *tmp, *new_prop;

    current_def = isolinux_get_default(isolinux);

    if (!isolinux || label.empty() || (label == current_def))
        return;

    tmp = isolinux;

    /* remove the current default label */
    if (!current_def.empty()) {
        /* search the label */
        while (tmp) {
            if (tmp->name == "LABEL" && tmp->value == current_def) {
                tmp = tmp->child;
                break;
            }
            tmp = tmp->next;
        }

        /* remove the property */
        while (tmp) {
            if (tmp->name == "MENU" && tmp->value == "DEFAULT") {
                if (tmp->prev)
                    tmp->prev->next = tmp->next;
                else
                    tmp->parent->child = tmp->next;
                delete tmp;
                break;
            }
            tmp = tmp->next;
        }
    }

    tmp = isolinux;

    /* set the new default label */
    while (tmp) {
        if (tmp->name == "LABEL" && tmp->value == label) {
            new_prop = new isolinux_t;
            new_prop->name = "MENU";
            new_prop->value = "DEFAULT";
            new_prop->prev = NULL;
            new_prop->child = NULL;

            if (tmp->child) {
                tmp = tmp->child;
                tmp->parent->child = new_prop;
                new_prop->next = tmp;
                new_prop->parent = tmp->parent;
            }
            else {
                tmp->child = new_prop;
                new_prop->next = NULL;
                new_prop->parent = tmp;
            }
            break;
        }
        tmp = tmp->next;
    }
}

int isolinux_bootlabel_nb(isolinux_t *isolinux, int debug)
{
  int nb = 0;

  for (; isolinux; isolinux = isolinux->next)
      if (isolinux->name == "LABEL") {
          if (!debug && isolinux->value.find("debug") != std::string::npos)
              continue;
          nb++;
      }

  return nb;
}

void isolinux_write(isolinux_t *isolinux, const char *path)
{
    FILE *f;
    isolinux_t *child;

    if (!isolinux)
        return;

    f = fopen(path, "wb");
    if (!f)
        return;

    while (isolinux) {
        fprintf(f, "%s", isolinux->name.c_str());
        if (!isolinux->value.empty())
            fprintf(f, " %s", isolinux->value.c_str());
        fprintf(f, "\n");

        child = isolinux->child;
        while (child) {
            fprintf(f, "  %s", child->name.c_str());
            if (!child->value.empty())
                fprintf(f, " %s", child->value.c_str());
            fprintf(f, "\n");
            child = child->next;
        }
        isolinux = isolinux->next;
    }

    fclose(f);
}
