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
        config_t *config;
        FILE *isolinux;

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

        config_getvar_location(config, "splash", 1, buf, sizeof(buf));
        ui->video_splash->value(!my_strcasecmp(buf, "silent"));

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
        config_t *config, *config2;

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

        config_setvar_location(config, "splash", 1,
                               ui->video_splash->value() ? "silent" : "0");
        config_setvar_location(config2, "splash", 1,
                               ui->video_splash->value() ? "silent" : "0");

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
