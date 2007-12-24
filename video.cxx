/*
 *  Video support for GeeXboX FLTK Generator
 *  Copyright (C) 2006-2007 Mathieu Schroeter
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

#include <FL/fl_ask.H> /* fl_alert */

#include <sys/stat.h> /* stat */

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


int init_video_tab(GeneratorUI *ui)
{
    char buf[256];
    int vgamode;
    int res, depth;

    if (target_arch == TARGET_ARCH_I386) {
        config_t *config, *config2;
        FILE *isolinux, *xorg_drivers;
        const Fl_Menu_Item *m;
        char xorg_w[8], xorg_h[8];

        /* part for read the default boot label */
        isolinux = fopen(PATH_BASEISO "/boot/isolinux.cfg", "rb");
        if (!isolinux) {
            fl_alert("Missing isolinux configuration files.\n");
            return 0;
        }
        ui->hdtv->value(0);
        while (fgets(buf, sizeof(buf), isolinux)) {
            if (strstr(buf, "DEFAULT") && strstr(buf, "hdtv")) {
                ui->hdtv->value(1);
                break;
            }
        }
        fclose(isolinux);

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        if (!config) {
            fl_alert("Missing isolinux configuration files.\n");
            return 0;
        }

        config2 = config_open(PATH_BASEISO "/etc/X11/X.cfg", 1);
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

static int write_boot_default(GeneratorUI *ui, const char *path)
{
    int res = 0;
    char buf[256];
    char *buf2;
    struct stat st;
    FILE *f;

    if (!path)
        return res;

    f = fopen(path, "rb");
    if (f) {
        stat(path, &st);

        while (fgets(buf, sizeof(buf), f)) {
            /* search the 'DEFAULT' line */
            if (strstr(buf, "DEFAULT")) {
                buf2 = (char *)malloc(st.st_size - strlen(buf));
                if (buf2)
                    fread(buf2, 1, st.st_size - strlen(buf), f);
                else
                    break;
                fclose(f);

                f = fopen(path, "wb");
                if (f) {
                    if (ui->hdtv->value())
                        fprintf(f, "DEFAULT hdtv\n");
                    else
                        fprintf(f, "DEFAULT geexbox\n");

                    fwrite(buf2, 1, st.st_size - strlen(buf), f);
                    res = 1;
                }
                free(buf2);
                break;
            }
        }
        if (f)
            fclose(f);
    }

    return res;
}

int write_video_settings(GeneratorUI *ui)
{
    int res, depth;
    int vgamode;

    if (target_arch == TARGET_ARCH_I386) {
        int i;
        config_t *config, *config2, *config3;
        char xorg_w[8], xorg_h[8];

        if (!write_boot_default(ui, PATH_BASEISO "/boot/isolinux.cfg") ||
            !write_boot_default(ui, PATH_BASEISO "/boot/pxelinux.cfg/default"))
        {
            fl_alert("Failed to open for write isolinux configuration.\n");
            return 0;
        }

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        config2 = config_open(PATH_BASEISO "/boot/pxelinux.cfg/default", 0);
        if (!config || !config2) {
            fl_alert("Failed to open for write isolinux configuration.\n");
            return 0;
        }

        config3 = config_open(PATH_BASEISO "/etc/X11/X.cfg", 1);
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
                strcpy(xorg_h, "640");
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

            config_write(config3, PATH_BASEISO "/etc/X11/X.cfg");
            config_destroy(config3);
        }
        else {
            ui->hdtv->value(0);
            ui->hdtv->deactivate();
        }

        for (i = 1; i <= 2; i++) {
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
