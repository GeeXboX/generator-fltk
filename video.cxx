/*
 *  Video support for GeeXboX FLTK Generator
 *  Copyright (C) 2006 Mathieu Schroeter
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
#include "video.h"
#include "system.h"
#include "fs.h"

#include <FL/fl_ask.H> /* fl_alert */

int init_video_tab(GeneratorUI *ui)
{
    char buf[256];
    int vgamode;
    int res, depth;
    int location;

    // get only the first found value
    location = 1;

    // only for i386 arch
    if (file_exists(PATH_BASEISO "/boot/isolinux.bin")) {
        config_t *config;

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        if (!config) {
            fl_alert("Missing isolinux configuration files.\n");
            return 0;
        }

        config_getvar_location(config, "splash", location, buf, sizeof(buf));
        ui->video_splash->value(my_strcasecmp(buf, "silent"));

        config_getvar_int_location(config, "vga", location, &vgamode);
        switch (vgamode) {
        case 784:
        case 785:
        case 786:
            res = GeneratorUI::VESA_RES_640;
            depth = vgamode - 784;
            break;
        case 787:
        case 788:
        case 789:
            res = GeneratorUI::VESA_RES_800;
            depth = vgamode - 787;
            break;
        case 790:
        case 791:
        case 792:
            res = GeneratorUI::VESA_RES_1024;
            depth = vgamode - 790;
            break;
        case 793:
        case 794:
        case 795:
            res = GeneratorUI::VESA_RES_1280;
            depth = vgamode - 793;
            break;
        case 797:
        case 798:
        case 799:
            res = GeneratorUI::VESA_RES_1600;
            depth = vgamode - 797;
            break;
        // custom value for advanced users
        default:
            res = GeneratorUI::VESA_CUSTOM;
            depth = GeneratorUI::VESA_DEPTH_24;
            sprintf(buf, "%i", vgamode);
            ui->vesa_custom->value(buf);
        }
        ui->vesa_res->value(res);
        ui->vesa_depth->value(depth);

        config_destroy(config);
    }
    else
        ui->vesa_res->deactivate();

    return 1;
}

int write_video_settings(GeneratorUI *ui)
{
    int res, depth;
    int vgamode;
    int location;
    const char *splashmode = NULL;

    // set only the first found value
    location = 1;

    // only for i386 arch
    if (file_exists(PATH_BASEISO "/boot/isolinux.bin")) {
        config_t *config;

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        if (!config) {
            fl_alert("Failed to open for write isolinux configuration.\n");
            return 0;
        }

        if (!ui->video_splash->value())
            splashmode = "silent";
        else
            splashmode = "0";
        config_setvar_location(config, "splash", location, splashmode);

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

        config_setvar_int_location(config, "vga", location, vgamode);

        config_write(config, PATH_BASEISO "/boot/isolinux.cfg");
        config_destroy(config);
    }

    return 1;
}
