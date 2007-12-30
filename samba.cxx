/*
 *  Samba support for GeeXboX FLTK Generator
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
#include "samba.h"
#include "system.h"
#include "utils.h"

#include <FL/fl_ask.H> /* fl_alert */

#include <string>
#include <vector>

typedef struct {
    std::string servername;
    std::string ip;
    std::string username;
    std::string password;
    std::vector<std::string> shares;
} Smbshare;

int g_nbshare;

static void init_tree(Flu_Tree_Browser *tree)
{
    tree->get_root()->always_open(true);
    tree->show_root(true);
    tree->label("Shares");
    tree->animate(false);
    tree->selection_mode(FLU_SINGLE_SELECT);
    tree->insertion_mode(FLU_INSERT_BACK);
}

void update_smb_tab(GeneratorUI *ui)
{
    Smbshare *s;
    std::vector<std::string>::const_iterator it;
    Flu_Tree_Browser *tree = ui->smb_shares;

    s = (Smbshare*)ui->smb_servers->mvalue()->user_data();

    if (s && ui->smb_servers->size() > 2) {
        ui->smb_servername->value(s->servername.c_str());
        ui->smb_ip->value(s->ip.c_str());
        ui->smb_username->value(s->username.c_str());
        ui->smb_password->value(s->password.c_str());

        tree->clear();
        init_tree(tree);
        for (it = s->shares.begin(); it != s->shares.end(); it++)
            tree->add(it->c_str());
    }
    else {
        ui->smb_servername->value("");
        ui->smb_ip->value("");
        ui->smb_username->value("");
        ui->smb_password->value("");

        tree->clear();
        init_tree(tree);
    }

    Fl::redraw();
}

static std::string get_str(std::string buf, std::string d1, std::string d2)
{
    std::string::size_type i_s, i_e;

    i_s = buf.find(d1);
    i_e = buf.find(d2);

    if (i_s != std::string::npos &&
        i_e != std::string::npos &&
        (i_e - i_s - d1.length()) > 0)
    {
        return buf.substr(i_s + d1.length(), i_e - i_s - d1.length());
    }
    return "";
}

static std::string get_share(std::string buf, int n)
{
    static std::string share, tmp;
    std::string res;

    if (n) {
        share = get_str(buf, "<#>", "");
        tmp = get_str(share, "", "<#>");
        return !tmp.empty() ? tmp : share;
    }

    tmp = tmp + "<#>";
    share = get_str(share, tmp, "");
    tmp = get_str(share, "", "<#>");

    return !tmp.empty() ? tmp : share;
}

static inline std::string get_servername(std::string buf)
{
    return get_str(buf, "<&>", "<#>");
}

static inline std::string get_ip(std::string buf)
{
    return get_str(buf, "<@>", "<&>");
}

static inline std::string get_password(std::string buf)
{
    return get_str(buf, "<%>", "<@>");
}

static inline std::string get_username(std::string buf)
{
    return get_str(buf, "", "<%>");
}

void add_smb(GeneratorUI *ui, int newadd)
{
    Smbshare *s = new Smbshare;
    Flu_Tree_Browser *tree = ui->smb_shares;
    Flu_Tree_Browser::Node *n;
    std::string server;

    s->servername = get_str_nospace((char*)ui->smb_servername->value(), 1);
    s->ip = get_str_nospace((char*)ui->smb_ip->value(), 1);
    s->username = get_str_nospace((char*)ui->smb_username->value(), 1);
    s->password = get_str_nospace((char*)ui->smb_password->value(), 1);

    if (!s->servername.empty() && !s->ip.empty() && (n = tree->first_leaf()))
    {
        while (n) {
            s->shares.push_back(n->label());
            n = n->next();
        }

        /* insert the new static samba entry in the listbox */
        server = s->servername + "\\/" + s->ip + " : " + s->username;
        ui->smb_servers->add(server.c_str(), 0, 0, (Smbshare*)s, 0);

        if (newadd)
            ui->smb_servers->value(0);

        update_smb_tab(ui);
    }
    else {
        delete s;

        if (newadd)
            fl_alert("At least 'Server name', 'Server IP' and one share are needed!\n");
    }
}

void add_smbshare(GeneratorUI *ui)
{
    std::string share;
    Flu_Tree_Browser *tree = ui->smb_shares;

    share = get_str_nospace((char*)ui->smb_name->value(), 1);
    if (!share.empty() && !tree->find(share.c_str()))
        tree->add(share.c_str());

    ui->smb_name->value("");

    if (strcmp(ui->smb_servers->mvalue()->label(), "<new>"))
        add_smb(ui, 0);
    else
        Fl::redraw();
}

void remove_smb(GeneratorUI *ui)
{
    if (ui->smb_servers->size() > 2) {
        if (strcmp(ui->smb_servers->mvalue()->label(), "<new>")) {
            /* remove the static samba entry */
            ui->smb_servers->remove(ui->smb_servers->value());
            ui->smb_servers->value(0);

            update_smb_tab(ui);
        }
    }
}

void remove_smbshare(GeneratorUI *ui)
{
    Flu_Tree_Browser *tree = ui->smb_shares;

    tree->remove(tree->get_hilighted());

    if (strcmp(ui->smb_servers->mvalue()->label(), "<new>"))
        add_smb(ui, 0);
    else
        Fl::redraw();
}

int init_samba_tab(GeneratorUI *ui)
{
    int i = 1;
    char buf[256];
    config_t *config;
    std::string server, share;
    Flu_Tree_Browser *tree = ui->smb_shares;

    config = config_open(PATH_BASEISO "/etc/network", 1);
    if (!config) {
        fl_alert("Missing samba configuration files.\n");
        return 0;
    }

    init_tree(tree);

    ui->smb_servers->add("<new>");

    while (config_getvar_location(config, "STATIC_SMB", i++, buf, sizeof(buf)))
    {
        Smbshare *s = new Smbshare;

        s->servername = get_servername(buf);
        s->ip = get_ip(buf);
        s->username = get_username(buf);
        s->password = get_password(buf);

        /* parse all shares */
        share = get_share(buf, 1);
        while (!share.empty()) {
            s->shares.push_back(share);
            share = get_share(buf, 0);
        }

        /* insert the static samba entry in the listbox */
        server = s->servername + "\\/" + s->ip + " : " + s->username;
        ui->smb_servers->add(server.c_str(), 0, 0, (Smbshare*)s, 0);

        g_nbshare++;
    }
    ui->smb_servers->value(0);

    config_destroy(config);

    return 1;
}

int write_samba_settings(GeneratorUI *ui)
{
    int i, new_nbshare = 0, obs_nbshare = 0;
    config_t *config;
    std::string item;
    std::vector<std::string>::const_iterator it;
    Smbshare *s;

    config = config_open(PATH_BASEISO "/etc/network", 1);
    if (!config) {
        fl_alert("Failed to write samba configuration.\n");
        return 0;
    }

    for (i = 1; i < ui->smb_servers->size() - 1; i++) {
        ui->smb_servers->value(i);
        s = (Smbshare*)ui->smb_servers->mvalue()->user_data();

        if (s) {
            item = s->username + "<%>" +
                   s->password + "<@>" +
                   s->ip + "<&>" +
                   s->servername;

            for (it = s->shares.begin(); it != s->shares.end(); it++)
                item += "<#>" + *it;

            config_setvar_location(config, "STATIC_SMB", i, item.c_str());
            if (i > g_nbshare)
                new_nbshare++;
        }
    }
    i--;

    /* if there is less of shares than before, then others must be removed */
    while (i < g_nbshare) {
        config_setvar_location(config, "STATIC_SMB", ++i, NULL);
        obs_nbshare++;
    }

    g_nbshare -= obs_nbshare;
    g_nbshare += new_nbshare;

    config_write(config, PATH_BASEISO "/etc/network");
    config_destroy(config);

    ui->smb_servers->value(0);
    update_smb_tab(ui);

    return 1;
}
