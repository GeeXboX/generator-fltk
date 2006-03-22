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
#include "recorder.h"
#include "system.h"
#include "utils.h"

#include <FL/fl_ask.H> /* fl_alert */

int init_recorder_tab(GeneratorUI *ui)
{
    char buf[256], buf_tmp[256];
    unsigned int i, j;
    FILE *f, *f2;
    const Fl_Menu_Item *m;
    const char *bl = "[common]";

    f = fopen(PATH_BASEISO "/etc/recorder", "r");
    if (!f) {
        ui->recorder->deactivate();
        return 1;
    }

    get_shvar_value(f, "SAVE_PATH", buf);
    ui->recorder_path->value(buf);

    f2 = fopen(PATH_BASEISO "/etc/mplayer/mencoder.conf", "r");
    if (!f2) {
        fl_alert("Missing mencoder profile files.\n");
        return 0;
    }
    while ((fgets (buf_tmp, sizeof(buf_tmp), f2))) {
        if (buf_tmp[0] == '[') {
            // test if it's "[common]"
            i = 0;
            while (buf_tmp[i] == bl[i] && i < strlen(bl) + 1)
                i++;
            if (i != strlen(bl)) {
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

    get_shvar_value(f, "RECORD_PROFILE", buf);
    if ((m = ui->recorder_profile->find_item(buf)))
        ui->recorder_profile->value(m);
    else if ((m = ui->recorder_profile->find_item("mpeg1")))
        ui->recorder_profile->value(m);
    else
        ui->recorder_profile->value(0);

    fclose(f);
    fclose(f2);

    return 1;
}

int write_recorder_settings(GeneratorUI *ui)
{
    FILE *fp;

    if (ui->recorder->active()) {
        fp = fopen(PATH_BASEISO "/etc/recorder", "wb");
        if (!fp) {
            fl_alert("Failed to write recorder configuration.\n");
            return 0;
        }

        fprintf(fp, "SAVE_PATH=\"%s\"\n", ui->recorder_path->value());
        fprintf(fp, "RECORD_PROFILE=%s\n", ui->recorder_profile->mvalue()->label());

        fclose(fp);
    }

    return 1;
}
