/*
 *  Recorder support for GeeXboX FLTK Generator
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
#include "recorder.h"
#include "system.h"
#include "utils.h"

#include <FL/fl_ask.H> /* fl_alert */

int init_recorder_tab(GeneratorUI *ui)
{
    char buf[256], buf_tmp[256];
    unsigned int i, j;
    unsigned int is_bl;
    FILE *f2;
    config_t *config;
    const Fl_Menu_Item *m;

    char *bl[] = {
        "[common]",
        "[dump]"
    };

    config = config_open(PATH_BASEISO "/etc/recorder", 1);
    if (!config) {
        ui->recorder->deactivate();
        return 1;
    }

    config_getvar(config, "SAVE_PATH", buf, sizeof(buf));
    ui->recorder_path->value(buf);

    f2 = fopen(PATH_BASEISO "/etc/mplayer/mencoder.conf", "r");
    if (!f2) {
        fl_alert("Missing mencoder profile files.\n");
        return 0;
    }
    while ((fgets (buf_tmp, sizeof(buf_tmp), f2))) {
        if (buf_tmp[0] == '[') {
            // search for a 'bl' word
            is_bl = 0;
            for (i = 0; i < sizeof(bl)/sizeof(bl[0]) && !is_bl; i++) {
                j = 0;
                while (buf_tmp[j] == bl[i][j] && j < strlen(bl[i]))
                    j++;
                if (j == strlen(bl[i]))
                    is_bl = 1;
            }
            if (!is_bl) {
                // if not then add item in the combobox
                j = 0;
                for (i = 1; i < strlen(buf_tmp); i++) {
                    if (buf_tmp[i] != ']' && buf_tmp[i] != '\0') {
                        buf[j] = buf_tmp[i];
                        j++;
                    }
                }
                buf[j-1] = '\0';
                ui->recorder_profile->add(buf);
            }
        }
    }

    config_getvar(config, "RECORD_PROFILE", buf, sizeof(buf));
    if ((m = ui->recorder_profile->find_item(buf)))
        ui->recorder_profile->value(m);
    else if ((m = ui->recorder_profile->find_item("mpeg1")))
        ui->recorder_profile->value(m);
    else
        ui->recorder_profile->value(0);

    config_destroy(config);
    fclose(f2);

    return 1;
}

int write_recorder_settings(GeneratorUI *ui)
{
    config_t *config;

    if (ui->recorder->active()) {
        config = config_open(PATH_BASEISO "/etc/recorder", 1);
        if (!config) {
            fl_alert("Failed to write recorder configuration.\n");
            return 0;
        }

        config_setvar(config, "SAVE_PATH", ui->recorder_path->value());
        config_setvar(config, "RECORD_PROFILE", ui->recorder_profile->mvalue()->label());
        config_write(config, PATH_BASEISO "/etc/recorder");
        config_destroy(config);
    }

    return 1;
}
