/*
 *  Network support for GeeXboX FLTK Generator
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

#include "generatorUI.h"

#include "compile.h"
#include "config.h"
#include "configparser.h"
#include "fs.h"
#include "network.h"
#include "system.h"
#include "utils.h"

#include <FL/fl_ask.H> /* fl_alert */
#include <FL/Fl_File_Chooser.H> /* fl_file_chooser */

#include <string>

#define yes_no(x) ((x) ? "yes" : "no") 

const char *path_ndiswrapper;

static void load_driver_node(Flu_Tree_Browser *tree, std::string inf, int copy)
{
    char buf[512];
    std::string src, dst, dir;
    std::string drivername, filename;
    std::string dst_dir = PATH_BASEISO "/etc/ndiswrapper";
    int size;
    Flu_Tree_Browser::Node *n;

    size = inf.find_last_of("/", inf.length() - 1);
    drivername = inf.substr(size + 1, inf.length() - size - 5);
    dir = inf.substr(0, size + 1);

    if (tree->find(drivername.c_str()) == NULL) {
        if (!fl_filename_isdir(dst_dir.c_str()))
            my_mkdir(dst_dir.c_str());

        if (copy) {
            snprintf(buf, sizeof(buf), "%s -i \"%s\" -o \"%s\" -a", path_ndiswrapper, inf.c_str(), dst_dir.c_str());
            if (execute_bg_program(buf) != 0) {
                fl_alert("Error with the INF file processing!\n");
                return;
            }
        }
        // add node to the browser tree
        n = tree->add(drivername.c_str());
        n->auto_label_color(true);
        Fl::redraw();
    }
    else
        fl_alert("This driver is already loaded!\n");
}

static void unload_driver_node(Flu_Tree_Browser *tree, Flu_Tree_Browser::Node *n)
{
    multi_delete(PATH_BASEISO "/etc/ndiswrapper/", NULL, NULL, 1);
    tree->remove(n);
    Fl::redraw();
}

void load_drvwin32(Flu_Tree_Browser *tree)
{
    const char *new_driver;
    Flu_Tree_Browser::Node *n;

    n = tree->get_root();
    if (n->children() == 0) {
        new_driver = fl_file_chooser("Choose win32 (Windows XP) driver?", "*.inf", "");
        if (new_driver) {
            if (strstr(new_driver, ".inf") != NULL || strstr(new_driver, ".INF") != NULL)
                load_driver_node(tree, new_driver, 1);
            else
                fl_alert("You haven't chosen a .inf file!\n");
        }
    }
    else
        fl_alert("Only one driver can be loaded!\n");
}

void unload_drvwin32(Flu_Tree_Browser *tree)
{
    Flu_Tree_Browser::Node *n;

    n = tree->get_root();
    if (n->children() != 0) {
        n = tree->first_leaf();
        unload_driver_node(tree, n);
    }
}

static std::string search_driver(const char *path)
{
    const char *fname;
    int i, num_files;
    std::string ret = "";
    struct dirent **files;

    if (fl_filename_isdir(path)) {
        num_files = fl_filename_list(path, &files, NULL);
        for (i = 0; i < num_files; i++) {
            fname = files[i]->d_name;
            if (fl_filename_isdir(fname) &&
                strcmp(fname, ".") != 0 &&
                strcmp(fname, "./") != 0 &&
                strcmp(fname, "..") != 0 &&
                strcmp(fname, "../") != 0)
            {
                ret = fname;
                ret.resize(ret.length() - 1);
                break;
            }
        }
        for (i = num_files; i > 0;)
            free((void*)(files[--i]));
        free((void*)files);
    }

    return ret;
}

int init_network_tab(GeneratorUI *ui)
{
    char buf[256];
    std::string inf;
    std::string driver_name;
    config_t *config, *config2;
    const Fl_Menu_Item *m;

    Flu_Tree_Browser *tree = ui->drvwin32_tree;

    config = config_open(PATH_BASEISO "/etc/network", 1);
    if (!config) {
	fl_alert("Missing network configuration files.\n");
	return 0;
    }

    config2 = config_open(PATH_BASEISO "/etc/ftp", 1);
    if (!config2) {
	fl_alert("Missing ftp configuration files.\n");
	return 0;
    }

    if (target_arch == TARGET_ARCH_I386) {
        path_ndiswrapper = find_program("ndiswrapper");

        // Init browser tree for win32 drivers
        tree->get_root()->always_open(true);
        tree->when(FL_WHEN_NOT_CHANGED);
        tree->show_root(true);
        tree->label("Windows driver");
        tree->animate(true);
        tree->selection_mode(FLU_SINGLE_SELECT);
        tree->insertion_mode(FLU_INSERT_BACK);

        driver_name = search_driver(PATH_BASEISO "/etc/ndiswrapper");
        if (driver_name != "") {
            inf = PATH_BASEISO "/etc/ndiswrapper/" + driver_name + "/" + driver_name + ".inf";
            load_driver_node(tree, inf.c_str(), 0);
        }
    }
    else
        ui->ndiswrapper_subtab->deactivate();

    config_getvar(config, "PHY_TYPE", buf, sizeof(buf));
    ui->phy_iface->value(!my_strcasecmp(buf, "wifi") ? 
				GeneratorUI::NETWORK_PHY_IFACE_WIFI :
                         !my_strcasecmp(buf, "ethernet") ?
				GeneratorUI::NETWORK_PHY_IFACE_ETHER :
				GeneratorUI::NETWORK_PHY_IFACE_AUTO);

    config_getvar(config, "WIFI_MODE", buf, sizeof(buf));
    ui->wifi_mode->value(!my_strcasecmp(buf, "ad-hoc") ? 
				GeneratorUI::WIFI_MODE_ADHOC :
				GeneratorUI::WIFI_MODE_MANAGED);

    config_getvar(config, "WIFI_ENC", buf, sizeof(buf));
    ui->wifi_enc->value(!my_strcasecmp(buf, "WEP") ? 
				GeneratorUI::WIFI_ENC_WEP :
                !my_strcasecmp(buf, "WPA") ?
				GeneratorUI::WIFI_ENC_WPA :
				GeneratorUI::WIFI_ENC_NONE);

    config_getvar(config, "WIFI_KEY", buf, sizeof(buf));
    ui->wifi_key->value(buf);

    config_getvar(config, "WIFI_ESSID", buf, sizeof(buf));
    ui->wifi_ssid->value(buf);

    config_getvar(config, "WIFI_CHANNEL", buf, sizeof(buf));
    if (!strcmp(buf, ""))
        strcpy(buf, "AUTO");
    if ((m = ui->wifi_channel->find_item(buf)))
        ui->wifi_channel->value(m);
    else
        ui->wifi_channel->value(0);

    config_getvar(config, "WPA_DRV", buf, sizeof(buf));
    ui->wpa_drv->value(!my_strcasecmp(buf, "atmel") ? 
				GeneratorUI::WPA_DRV_ATMEL :
				GeneratorUI::WPA_DRV_WEXT );

    config_getvar(config, "GATEWAY", buf, sizeof(buf));
    ui->network_gateway->value(buf);

    config_getvar(config, "DNS_SERVER", buf, sizeof(buf));
    ui->network_dns->value(buf);

    config_getvar(config, "SUBNET", buf, sizeof(buf));
    ui->network_subnet->value(buf);

    config_getvar(config, "HOST", buf, sizeof(buf));
    ui->network_ip->value(buf);

    ui->network_conf->value(buf[0] ? GeneratorUI::NETWORK_CONF_MANUAL :
                                     GeneratorUI::NETWORK_CONF_DHCP);

    config_getvar(config, "SMB_USER", buf, sizeof(buf));
    ui->samba_user->value(buf);

    config_getvar(config, "SMB_PWD", buf, sizeof(buf));
    ui->samba_pass->value(buf);

    config_getvar(config, "TELNET_SERVER", buf, sizeof(buf));
    ui->server_telnet->value(!my_strcasecmp(buf, "yes"));

    config_getvar(config, "FTP_SERVER", buf, sizeof(buf));
    ui->server_ftp->value(!my_strcasecmp(buf, "yes"));

    config_getvar(config, "HTTP_SERVER", buf, sizeof(buf));
    ui->server_http->value(!my_strcasecmp(buf, "yes"));

    config_getvar(config, "UPNP", buf, sizeof(buf));
    ui->upnp_discovery->value(!my_strcasecmp(buf, "yes"));

    config_getvar(config, "SHOUTCAST", buf, sizeof(buf));
    ui->streaming_shoutcast->value(!my_strcasecmp(buf, "yes"));

    config_getvar(config, "SHOUTCASTTV", buf, sizeof(buf));
    ui->streaming_shoutcasttv->value(!my_strcasecmp(buf, "yes"));

    config_getvar(config, "ICECAST", buf, sizeof(buf));
    ui->streaming_icecast->value(!my_strcasecmp(buf, "yes"));

    config_getvar(config, "WHITELIST", buf, sizeof(buf));
    ui->streaming_whitelist->value(buf);

    config_getvar(config, "BLACKLIST", buf, sizeof(buf));
    ui->streaming_blacklist->value(buf);

    config_getvar(config, "TIMEOUT", buf, sizeof(buf));
    ui->streaming_timeout->value(buf);

    config_getvar(config, "TRIES", buf, sizeof(buf));
    ui->streaming_tries->value(buf);

    config_destroy(config);

    config_getvar(config2, "USERNAME", buf, sizeof(buf));
    ui->ftp_user->value(buf);

    config_getvar(config2, "PASSWORD", buf, sizeof(buf));
    ui->ftp_pass->value(buf);

    config_destroy(config2);

    return 1;
}

int write_network_settings(GeneratorUI *ui)
{
    int manual;
    config_t *config, *config2;
    const char *str = NULL;

    config = config_open(PATH_BASEISO "/etc/network", 1);
    if (!config) {
	fl_alert("Failed to write network configuration.\n");
	return 0;
    }

    config2 = config_open(PATH_BASEISO "/etc/ftp", 1);
    if (!config) {
	fl_alert("Failed to write ftp configuration.\n");
	return 0;
    }

    switch (ui->phy_iface->value())
    {
    case GeneratorUI::NETWORK_PHY_IFACE_AUTO:
        str = "auto";
        break;
    case GeneratorUI::NETWORK_PHY_IFACE_WIFI:
        str = "wifi";
        break;
    case GeneratorUI::NETWORK_PHY_IFACE_ETHER:
        str = "ethernet";
        break;
    }
    config_setvar(config, "PHY_TYPE", str);

    config_setvar(config, "WIFI_MODE",
		(ui->wifi_mode->value() == GeneratorUI::WIFI_MODE_ADHOC)
		    ? "ad-hoc" : "managed");
    config_setvar(config, "WIFI_ENC",
		(ui->wifi_enc->value() == GeneratorUI::WIFI_ENC_WEP)
		    ? "WEP" :
           (ui->wifi_enc->value() == GeneratorUI::WIFI_ENC_WPA)
            ? "WPA" : "none");
    config_setvar(config, "WIFI_KEY", ui->wifi_key->value());
    config_setvar(config, "WIFI_ESSID", ui->wifi_ssid->value());
    config_setvar(config, "WIFI_CHANNEL", strcmp(ui->wifi_channel->mvalue()->label(), "AUTO")
                                          ? ui->wifi_channel->mvalue()->label() : "");
    config_setvar(config, "WPA_DRV",
		(ui->wpa_drv->value() == GeneratorUI::WPA_DRV_ATMEL)
		    ? "atmel" : "wext" );

    manual = (ui->network_conf->value() == GeneratorUI::NETWORK_CONF_MANUAL);
    config_setvar(config, "HOST", manual ? ui->network_ip->value() : "");
    config_setvar(config, "SUBNET", manual ? ui->network_subnet->value() : "");
    config_setvar(config, "GATEWAY", manual ? ui->network_gateway->value() : "");
    config_setvar(config, "DNS_SERVER", manual ? ui->network_dns->value() : "");
    
    config_setvar(config, "SMB_USER", ui->samba_user->value());
    config_setvar(config, "SMB_PWD", ui->samba_pass->value());

    config_setvar(config, "TELNET_SERVER", yes_no(ui->server_telnet->value()));
    config_setvar(config, "FTP_SERVER", yes_no(ui->server_ftp->value()));
    config_setvar(config, "HTTP_SERVER", yes_no(ui->server_http->value()));

    config_setvar(config, "UPNP", yes_no(ui->upnp_discovery->value()));

    config_setvar(config, "SHOUTCAST", yes_no(ui->streaming_shoutcast->value()));
    config_setvar(config, "SHOUTCASTTV", yes_no(ui->streaming_shoutcasttv->value()));
    config_setvar(config, "ICECAST", yes_no(ui->streaming_icecast->value()));
    config_setvar(config, "WHITELIST", ui->streaming_whitelist->value());
    config_setvar(config, "BLACKLIST", ui->streaming_blacklist->value());
    config_setvar(config, "TIMEOUT", ui->streaming_timeout->value());
    config_setvar(config, "TRIES", ui->streaming_tries->value());

    config_write(config, PATH_BASEISO "/etc/network");
    config_destroy(config);

    config_setvar(config2, "USERNAME", ui->ftp_user->value());
    config_setvar(config2, "PASSWORD", ui->ftp_pass->value());

    config_write(config2, PATH_BASEISO "/etc/ftp");
    config_destroy(config2);

    return 1;
}
