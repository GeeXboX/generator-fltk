/*
 *  LCD Display support for GeeXboX FLTK Generator
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

#include "compile.h"
#include "config.h"
#include "configparser.h"
#include "lcd.h"
#include "system.h"

#include <FL/fl_ask.H> /* fl_alert */

#define yes_no(x) ((x) ? "yes" : "no") 

int init_lcd_tab(GeneratorUI *ui)
{
    char buf[256], buf_tmp[256];
    unsigned int i, j;
    FILE *f;
    config_t *config;
    const Fl_Menu_Item *m;

    const char *model = "Display ";

    /* only for i386 */
    config = config_open(PATH_BASEISO "/etc/lcddisplay", 1);
    if (target_arch != TARGET_ARCH_I386 || !config) {
        ui->setting_tabs->remove(ui->lcd_tab);
        return 1;
    }

    config_getvar(config, "LCD_ENABLED", buf, sizeof(buf));
    ui->lcd_enabled->value(!my_strcasecmp(buf, "yes"));

    config_getvar(config, "LCD_WIDTH", buf, sizeof(buf));
    ui->lcd_width->value(buf);

    config_getvar(config, "LCD_HEIGHT", buf, sizeof(buf));
    ui->lcd_height->value(buf);

    f = fopen(PATH_BASEISO "/etc/lcd4linux.conf", "r");
    if (!f) {
        fl_alert("Missing lcd4linux profile files.\n");
        return 0;
    }
    while ((fgets (buf_tmp, sizeof(buf_tmp), f))) {
        if (strncmp(model, buf_tmp, strlen(model)) == 0) {
            j = 0;
            for (i = strlen(model); i < strlen(buf_tmp) && buf_tmp[i] != ' '; i++) {
                buf[j] = buf_tmp[i];
                j++;
            }
            buf[j] = '\0';
            ui->lcd_model->add(buf);
        }
    }

    config_getvar(config, "LCD_MODEL", buf, sizeof(buf));
    if ((m = ui->lcd_model->find_item(buf)))
        ui->lcd_model->value(m);
    else if ((m = ui->lcd_model->find_item("HD44780-winamp")))
        ui->lcd_model->value(m);
    else
        ui->lcd_model->value(0);

    config_destroy(config);
    fclose(f);

    return 1;
}

int write_lcd_settings(GeneratorUI *ui)
{
    config_t *config;

    /* only for i386 */
    config = config_open(PATH_BASEISO "/etc/lcddisplay", 1);
    if (target_arch != TARGET_ARCH_I386 || !config) {
        return 1;
    }

    config_setvar(config, "LCD_ENABLED", yes_no(ui->lcd_enabled->value()));
    config_setvar(config, "LCD_MODEL", ui->lcd_model->mvalue()->label());
    config_setvar(config, "LCD_WIDTH", ui->lcd_width->value());
    config_setvar(config, "LCD_HEIGHT", ui->lcd_height->value());
    config_write(config, PATH_BASEISO "/etc/lcddisplay");
    config_destroy(config);

    return 1;
}
