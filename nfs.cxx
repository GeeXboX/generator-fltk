/*
 *  NFS support for GeeXboX FLTK Generator
 *  Copyright (C) 2007 Mathieu Schroeter
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

#include "generatorUI.h"

#include "config.h"
#include "configparser.h"
#include "nfs.h"
#include "system.h"

#include <FL/fl_ask.H> /* fl_alert */

#include <string>

typedef struct {
    std::string server;
    std::string dir;
} Nfsshare;

static std::string get_str_nospace(char *buf, int loc)
{
    int i, len;
    char *start, *end, *str;
    char buf2[256];
    std::string res;

    str = buf;
    for (i = 1; i <= loc; i++) {
        while (isspace(*str) && *str != '\n' && *str != '\0')
            str++;

        if (*str == '#')
            break;

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

void update_nfs_tab(GeneratorUI *ui)
{
    Nfsshare *n;

    n = (Nfsshare*)ui->nfs_shares->mvalue()->user_data();

    if (n && ui->nfs_shares->size() > 2) {
        ui->nfs_server->value(n->server.c_str());
        ui->nfs_dir->value(n->dir.c_str());
        ui->nfs_mountpoint->value(ui->nfs_shares->mvalue()->label());
    }
    else {
        ui->nfs_server->value("");
        ui->nfs_dir->value("");
        ui->nfs_mountpoint->value("");
    }
}

void add_nfs(GeneratorUI *ui)
{
    Nfsshare *n = new Nfsshare;
    std::string mp;

    mp = get_str_nospace((char*)ui->nfs_mountpoint->value(), 1);
    n->server = get_str_nospace((char*)ui->nfs_server->value(), 1);
    n->dir = get_str_nospace((char*)ui->nfs_dir->value(), 1);

    if (!n->server.empty() && !n->dir.empty() && !mp.empty()) {
        ui->nfs_shares->add(mp.c_str(), 0, 0, (Nfsshare*)n, 0);
        ui->nfs_shares->value(0);
        update_nfs_tab(ui);
    }
    else
        delete n;
}

void remove_nfs(GeneratorUI *ui)
{
    if (ui->nfs_shares->size() > 2) {
        int i;
        const char *item;

        item = ui->nfs_shares->mvalue()->label();
        if (strcmp(item, "<new>")) {
            for (i = 1; i < ui->nfs_shares->size() - 1; i++) {
                ui->nfs_shares->value(i);
                if (!strcmp(ui->nfs_shares->mvalue()->label(), item)) {
                    ui->nfs_shares->value(0);
                    ui->nfs_shares->remove(i);
                    break;
                }
            }
            update_nfs_tab(ui);
        }
    }
}

int init_nfs_tab(GeneratorUI *ui)
{
    char buf[256];
    FILE *f;

    f = fopen(PATH_BASEISO "/etc/nfs", "rb");
    if (!f) {
        fl_alert("Missing nfs configuration files.\n");
        return 0;
    }

    /* Read all NFS mountpoints */
    ui->nfs_shares->add("<new>");

    while ((fgets(buf, sizeof(buf), f))) {
        std::string src, dst;

        src = get_str_nospace(buf, 1);
        dst = get_str_nospace(buf, 2);

        if (!src.empty() && !dst.empty()) {
            std::string::size_type index;
            std::string server, dir;

            index = src.find(':');
            if (index != std::string::npos) {
                Nfsshare *n = new Nfsshare;

                server = src.substr(0, index);
                dir = src.substr(index + 1);
                n->server = server;
                n->dir = dir;
                ui->nfs_shares->add(dst.c_str(), 0, 0, (Nfsshare*)n, 0);
            }
        }
    }
    ui->nfs_shares->value(0);
    fclose(f);

    return 1;
}

int write_nfs_settings(GeneratorUI *ui)
{
    int i;
    std::string share;
    FILE *f;
    Nfsshare *n;

    f = fopen(PATH_BASEISO "/etc/nfs", "wb");
    if (!f) {
        fl_alert("Failed to write nfs configuration.\n");
        return 0;
    }

    /* Write all NFS mountpoints */
    for (i = 1; i < ui->nfs_shares->size() - 1; i++) {
        ui->nfs_shares->value(i);
        n = (Nfsshare*)ui->nfs_shares->mvalue()->user_data();

        if (n) {
            share = n->server + ":" + n->dir + " ";
            share += ui->nfs_shares->mvalue()->label() + std::string("\n");
            fputs(share.c_str(), f);
        }
    }
    fclose(f);
    ui->nfs_shares->value(0);
    update_nfs_tab(ui);

    return 1;
}
