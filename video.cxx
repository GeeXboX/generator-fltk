/*
 *  Video support for GeeXboX FLTK Generator
 *  Copyright (C) 2006-2008 Mathieu Schroeter
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
#include "compile.h"
#include "configparser.h"
#include "video.h"
#include "system.h"
#include "fs.h"
#include "utils.h"

#include <FL/fl_ask.H> /* fl_alert */

#include <sys/stat.h> /* stat */
#include <string>

#define NB_BOOTLABEL  3

typedef struct isolinux_s {
  std::string name;
  std::string value;
  struct isolinux_s *prev;
  struct isolinux_s *next;
  struct isolinux_s *parent;
  struct isolinux_s *child;
} isolinux_t;

const char *get_target_resolution(GeneratorUI *ui)
{
    char *res;

    switch (ui->vesa_res->value()) {
    case GeneratorUI::VESA_RES_640:
        res = "640x480";
        break;
    case GeneratorUI::VESA_RES_800:
        res = "800x600";
        break;
    case GeneratorUI::VESA_RES_1024:
        res = "1024x768";
        break;
    case GeneratorUI::VESA_RES_1280:
        res = "1280x1024";
        break;
    case GeneratorUI::VESA_RES_1600:
        res = "1600x1200";
        break;
    default:
        res = "custom";
    }

    return res;
}

static isolinux_t *isolinux_load(const char *path)
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

static void isolinux_unload(isolinux_t *isolinux)
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

static std::string isolinux_get_default(isolinux_t *isolinux)
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

static void isolinux_set_default(isolinux_t *isolinux, std::string label)
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

static void isolinux_write(isolinux_t *isolinux, const char *path)
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

int init_video_tab(GeneratorUI *ui)
{
    char buf[256];
    int vgamode;
    int res, depth;

    if (target_arch == TARGET_ARCH_I386) {
        config_t *config, *config2;
        isolinux_t *isolinux;
        FILE *xorg_drivers;
        const Fl_Menu_Item *m;
        char xorg_w[8], xorg_h[8];

        /* part for read the default boot label */
        isolinux = isolinux_load(PATH_BASEISO "/boot/isolinux.cfg");
        if (!isolinux) {
            fl_alert("Missing isolinux configuration files.\n");
            return 0;
        }
        ui->hdtv->value(0);
        if (isolinux_get_default(isolinux) == "hdtv")
            ui->hdtv->value(1);
        isolinux_unload(isolinux);

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        if (!config) {
            fl_alert("Missing isolinux configuration files.\n");
            return 0;
        }

        if (file_exists(PATH_BASEISO "/etc/X11/X.cfg")) {
            ui->xorg_auto->value(0);
            config2 = config_open(PATH_BASEISO "/etc/X11/X.cfg", 1);
        }
        else {
            ui->xorg_auto->value(1);
            config2 = config_open(PATH_BASEISO "/etc/X11/X.cfg.sample", 1);
        }

        if (config2) {
            xorg_drivers = fopen(PATH_BASEISO "/etc/X11/drivers", "rb");
            ui->xorg_drivers->add("auto", 0, NULL, (char *)"auto");
            if (xorg_drivers) {
                while (fgets(buf, sizeof(buf), xorg_drivers)) {
                    buf[strlen(buf) - 1] = '\0';
                    ui->xorg_drivers->add(buf, 0, NULL, buf);
                }
                fclose(xorg_drivers);
            }

            config_getvar(config2, "XORG_DRIVER", buf, sizeof(buf));
            if ((m = ui->xorg_drivers->find_item(buf)))
                ui->xorg_drivers->value(m);
            else if ((m = ui->xorg_drivers->find_item("auto")))
                ui->xorg_drivers->value(m);
            else
                ui->xorg_drivers->value(0);

            config_getvar(config2, "XORG_RATE", buf, sizeof(buf));
            ui->xorg_rate->value(buf);
            config_getvar(config2, "XORG_HORIZSYNC", buf, sizeof(buf));
            ui->xorg_horizsync->value(buf);
            config_getvar(config2, "XORG_VERTREFRESH", buf, sizeof(buf));
            ui->xorg_vertrefresh->value(buf);

            config_getvar(config2, "XORG_RESX", xorg_w, sizeof(xorg_w));
            config_getvar(config2, "XORG_RESY", xorg_h, sizeof(xorg_h));
            if (!my_strcasecmp(xorg_w, "720") &&
                !my_strcasecmp(xorg_h, "480"))
                res = GeneratorUI::XORG_RES_720;
            else if (!my_strcasecmp(xorg_w, "1280") &&
                     !my_strcasecmp(xorg_h, "720"))
                res = GeneratorUI::XORG_RES_1280;
            else if (!my_strcasecmp(xorg_w, "1360") &&
                     !my_strcasecmp(xorg_h, "768"))
                res = GeneratorUI::XORG_RES_1360;
            else if (!my_strcasecmp(xorg_w, "1368") &&
                     !my_strcasecmp(xorg_h, "768"))
                res = GeneratorUI::XORG_RES_1368;
            else if (!my_strcasecmp(xorg_w, "1920") &&
                     !my_strcasecmp(xorg_h, "1080"))
                res = GeneratorUI::XORG_RES_1920;
            else if (!my_strcasecmp(xorg_w, "auto") ||
                     !my_strcasecmp(xorg_h, "auto"))
                res = GeneratorUI::XORG_AUTO;
            else {
                res = GeneratorUI::XORG_CUSTOM;
                ui->xorg_custom_w->value(xorg_w);
                ui->xorg_custom_h->value(xorg_h);
            }
            ui->xorg_res->value(res);

            config_destroy(config2);
        }
        else {
            ui->hdtv->value(0);
            ui->hdtv->deactivate();
        }

        config_getvar_int_location(config, "vga", 1, &vgamode);
        if (vgamode >= 784 && vgamode <= 786) {
            res = GeneratorUI::VESA_RES_640;
            depth = vgamode - 784;
        }
        else if (vgamode >= 787 && vgamode <= 789) {
            res = GeneratorUI::VESA_RES_800;
            depth = vgamode - 787;
        }
        else if (vgamode >= 790 && vgamode <= 792) {
            res = GeneratorUI::VESA_RES_1024;
            depth = vgamode - 790;
        }
        else if (vgamode >= 793 && vgamode <= 795) {
            res = GeneratorUI::VESA_RES_1280;
            depth = vgamode - 793;
        }
        else if (vgamode >= 797 && vgamode <= 799) {
            res = GeneratorUI::VESA_RES_1600;
            depth = vgamode - 797;
        }
        else {
            res = GeneratorUI::VESA_CUSTOM;
            depth = GeneratorUI::VESA_DEPTH_24;
            sprintf(buf, "%i", vgamode);
            ui->vesa_custom->value(buf);
        }
        ui->vesa_res->value(res);
        ui->vesa_depth->value(depth);

        config_getvar_location(config, "splash", 1, buf, sizeof(buf));
        ui->video_splash->value(!my_strcasecmp(buf, "silent"));

        config_destroy(config);
    }
    else if (target_arch == TARGET_ARCH_POWERPC)
    {
        config_t *config;

        config = config_open(PATH_BASEISO "/boot/yaboot.conf", 0);
        if (!config) {
            fl_alert("Missing yaboot configuration files.\n");
            return 0;
        }

        /* no X.Org with PowerPC */
        ui->hdtv->value(0);
        ui->hdtv->deactivate();

        config_getvar_location(config, "splash", 1, buf, sizeof(buf));
        ui->video_splash->value(!my_strcasecmp(buf, "silent"));

        ui->vesa_res->deactivate();
        ui->vesa_res->value(GeneratorUI::VESA_RES_800);
        ui->vesa_depth->deactivate();
    }

    return 1;
}

int write_video_settings(GeneratorUI *ui)
{
    int res, depth;
    int vgamode;

    if (target_arch == TARGET_ARCH_I386) {
        int i;
        config_t *config, *config2, *config3;
        isolinux_t *isolinux, *pxelinux;
        char xorg_w[8], xorg_h[8];

        /* part for write the default boot label */
        isolinux = isolinux_load(PATH_BASEISO "/boot/isolinux.cfg");
        pxelinux = isolinux_load(PATH_BASEISO "/boot/pxelinux.cfg/default");
        if (!isolinux || !pxelinux) {
            fl_alert("Failed to open for write isolinux configuration.\n");
            return 0;
        }
        if (ui->hdtv->value()) {
            isolinux_set_default(isolinux, "hdtv");
            isolinux_set_default(pxelinux, "hdtv");
        }
        else {
            isolinux_set_default(isolinux, "geexbox");
            isolinux_set_default(pxelinux, "geexbox");
        }
        isolinux_write(isolinux, PATH_BASEISO "/boot/isolinux.cfg");
        isolinux_write(pxelinux, PATH_BASEISO "/boot/pxelinux.cfg/default");
        isolinux_unload(isolinux);
        isolinux_unload(pxelinux);

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        config2 = config_open(PATH_BASEISO "/boot/pxelinux.cfg/default", 0);
        if (!config || !config2) {
            fl_alert("Failed to open for write isolinux configuration.\n");
            return 0;
        }

        config3 = config_open(PATH_BASEISO "/etc/X11/X.cfg", 1);
        if (!config3)
            config3 = config_open(PATH_BASEISO "/etc/X11/X.cfg.sample", 1);

        if (config3) {
            config_setvar(config3, "XORG_DRIVER",
                          ui->xorg_drivers->mvalue()->label());

            config_setvar(config3, "XORG_RATE",
                          *ui->xorg_rate->value() == '\0' ?
                          "auto" : ui->xorg_rate->value());
            config_setvar(config3, "XORG_HORIZSYNC",
                          *ui->xorg_horizsync->value() == '\0' ?
                          "auto" : ui->xorg_horizsync->value());
            config_setvar(config3, "XORG_VERTREFRESH",
                          *ui->xorg_vertrefresh->value() == '\0' ?
                          "auto" : ui->xorg_vertrefresh->value());

            switch (ui->xorg_res->value()) {
            case GeneratorUI::XORG_AUTO:
                strcpy(xorg_w, "auto");
                strcpy(xorg_h, "auto");
                break;
            case GeneratorUI::XORG_RES_720:
                strcpy(xorg_w, "720");
                strcpy(xorg_h, "480");
                break;
            case GeneratorUI::XORG_RES_1280:
                strcpy(xorg_w, "1280");
                strcpy(xorg_h, "720");
                break;
            case GeneratorUI::XORG_RES_1360:
                strcpy(xorg_w, "1360");
                strcpy(xorg_h, "768");
                break;
            case GeneratorUI::XORG_RES_1368:
                strcpy(xorg_w, "1368");
                strcpy(xorg_h, "768");
                break;
            case GeneratorUI::XORG_RES_1920:
                strcpy(xorg_w, "1920");
                strcpy(xorg_h, "1080");
                break;
            default:
                strncpy(xorg_w, ui->xorg_custom_w->value(), sizeof(xorg_w));
                strncpy(xorg_h, ui->xorg_custom_h->value(), sizeof(xorg_h));
                xorg_w[sizeof(xorg_w) - 1] = '\0';
                xorg_h[sizeof(xorg_h) - 1] = '\0';
            }

            config_setvar(config3, "XORG_RESX", *xorg_w == '\0' ?
                                                "auto" : xorg_w);
            config_setvar(config3, "XORG_RESY", *xorg_h == '\0' ?
                                                "auto" : xorg_h);

            config_write(config3, PATH_BASEISO "/etc/X11/X.cfg.sample");
            config_destroy(config3);

            if (!ui->xorg_auto->value()) {
                copy_file(PATH_BASEISO "/etc/X11/X.cfg.sample",
                          PATH_BASEISO "/etc/X11/X.cfg");
                unlink(PATH_BASEISO "/etc/X11/X.cfg.sample");
            }
            else
                unlink(PATH_BASEISO "/etc/X11/X.cfg");

            /* 800x600 bootsplash with HDTV */
            if (ui->hdtv->value() && ui->video_splash->value())
                ui->vesa_res->value(GeneratorUI::VESA_RES_800);
        }
        else {
            ui->hdtv->value(0);
            ui->hdtv->deactivate();
        }

        for (i = 1; i <= NB_BOOTLABEL; i++) {
            config_setvar_location(config, "splash", i,
                ui->video_splash->value() &&
                ui->vesa_res->value() != GeneratorUI::VESA_CUSTOM ? "silent" : "0");
            config_setvar_location(config2, "splash", i,
                ui->video_splash->value() &&
                ui->vesa_res->value() != GeneratorUI::VESA_CUSTOM ? "silent" : "0");
        }

        depth = ui->vesa_depth->value();
        switch (ui->vesa_res->value()) {
        case GeneratorUI::VESA_RES_640:
            res = 784;
            break;
        case GeneratorUI::VESA_RES_800:
            res = 787;
            break;
        case GeneratorUI::VESA_RES_1024:
            res = 790;
            break;
        case GeneratorUI::VESA_RES_1280:
            res = 793;
            break;
        case GeneratorUI::VESA_RES_1600:
            res = 797;
            break;
        default:
            res = atoi(ui->vesa_custom->value());
            depth = 0;
        }
        vgamode = res + depth;

        config_setvar_int(config, "vga", vgamode);
        config_setvar_int(config2, "vga", vgamode);

        config_write(config, PATH_BASEISO "/boot/isolinux.cfg");
        config_write(config2, PATH_BASEISO "/boot/pxelinux.cfg/default");
        config_destroy(config);
        config_destroy(config2);
    }
    else if (target_arch == TARGET_ARCH_POWERPC)
    {
        config_t *config, *config2;

        config = config_open(PATH_BASEISO "/boot/yaboot.conf", 0);
        config2 = config_open(PATH_BASEISO "/boot/netboot/yaboot.conf", 0);
        if (!config || !config2) {
            fl_alert("Failed to open for write yaboot configuration.\n");
            return 0;
        }

        config_setvar_location(config, "splash", 1,
                               ui->video_splash->value() ? "silent" : "0");
        config_setvar_location(config2, "splash", 1,
                               ui->video_splash->value() ? "silent" : "0");

        config_write(config, PATH_BASEISO "/boot/yaboot.conf");
        config_write(config2, PATH_BASEISO "/boot/netboot/yaboot.conf");
        config_destroy(config);
        config_destroy(config2);
    }

    return 1;
}
