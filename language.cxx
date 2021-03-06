/*
 *  Language support for GeeXboX FLTK Generator
 *  Copyright (C) 2005-2006  Amir Shalem
 *  Copyright (C) 2007-2008  Mathieu Schroeter
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
#include "compile.h"
#include "fs.h"
#include "language.h"
#include "system.h"
#include "theme.h"
#include "utils.h"

#include <FL/fl_ask.H> /* fl_alert */
#include <FL/filename.H> /* fl_filename_name */
#include <FL/Fl_File_Chooser.H> /* fl_file_chooser */

void change_font(Fl_Output *o, Fl_Check_Button *b)
{
    const char *new_file;
    const char *old_file = o->value();

    if (!old_file || !*old_file)
        old_file = PATH_FONTS;

    new_file = fl_file_chooser("Choose font?", "*.ttf", old_file);
    if (new_file)
    {
        o->value(new_file);
        o->activate();
        b->value(1);
    }
}

int init_language_tab(GeneratorUI *ui)
{
    char buf[256], *word;
    char buf2[50], buf3[256], buf4[256], buf5[256];
    char lang[256];
    struct lang_info *l;
    struct charset_info *c;
    const Fl_Menu_Item *m;
    config_t *config;

    if (target_arch == TARGET_ARCH_I386 || target_arch == TARGET_ARCH_X86_64) {
        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        if (!config) {
            fl_alert("Missing isolinux configuration files.\n");
            return 0;
        }

        config_getvar(config, "lang", lang, sizeof(lang));

        config_destroy(config);
    }
    else if (target_arch == TARGET_ARCH_POWERPC) {
        config = config_open(PATH_BASEISO "/boot/yaboot.conf", 0);
        if (!config) {
            fl_alert("Missing yaboot configuration files.\n");
            return 0;
        }

        config_getvar(config, "lang", lang, sizeof(lang));

        config_destroy(config);
    }

    config = config_open(PATH_LANGCONF, 1);
    if (!config) {
        fl_alert("Missing language configuration files.\n");
        return 0;
    }

    config_getvar(config, "CHARSETS", buf, sizeof(buf));
    for (word = strtok(buf, " \t"); word; word = strtok(NULL, " \t"))
    {
        c = (struct charset_info*)malloc(sizeof(struct charset_info));
        if (c)
        {
            ui->sub_charset->add(word, 0, NULL, c);

            c->menu_font = c->sub_font = NULL;
            c->name = strdup(word);

            replace_char(word, '-', '_');

            c->codename = strdup(word);

            sprintf(buf2, "%s_menufont", word);
            config_getvar(config, buf2, buf3, sizeof(buf3));
            if (buf3[0])
                c->menu_font = strdup(buf3);

            sprintf(buf2, "%s_subfont", word);
            config_getvar(config, buf2, buf3, sizeof(buf3));
            if (buf3[0])
                c->sub_font = strdup(buf3);

            sprintf(buf2, "%s_font", word);
            config_getvar(config, buf2, buf3, sizeof(buf3));
            if (!buf3[0])
                config_getvar(config, "DEFAULT_FONT", buf3, sizeof(buf3));
            if (!c->menu_font)
                c->menu_font = strdup(buf3);
            if (!c->sub_font)
                c->sub_font = strdup(buf3);
        }
    }

    config_getvar(config, "LANGUAGES", buf, sizeof(buf));
    for (word = strtok(buf, " \t"); word; word = strtok(NULL, " \t"))
    {
        sprintf(buf2, "%s_name", word);
        config_getvar(config, buf2, buf3, sizeof(buf3));

        l = (struct lang_info*)malloc(sizeof(struct lang_info));
        if (l)
        {
            l->c = NULL;
            ui->menu_lang->add(buf3, 0, NULL, l);

            l->shortname = strdup(word);

            sprintf(buf2, "%s_charset", word);
            config_getvar(config, buf2, buf3, sizeof(buf3));
            m = ui->sub_charset->find_item(buf3);
            if (!m) {
                fl_alert("Missing language character set %s.\n", buf3);
                config_destroy(config);
                return 0;
            }
            l->c = (struct charset_info*)m->user_data();
        }
    }

    if (ui->sub_charset->size() < 1 || ui->menu_lang->size() < 1) {
        fl_alert("No languages or character set found.\n");
        config_destroy(config);
        return 0;
    }

    config_getvar(config, "DEFAULT_LANGUAGE", buf, sizeof(buf));

    sprintf(buf2, "%s_charset", buf);
    sprintf(buf4, "%s_charset", lang);
    config_getvar(config, buf2, buf3, sizeof(buf3));
    config_getvar(config, buf4, buf5, sizeof(buf5));
    if ((m = ui->sub_charset->find_item(buf5)))
        ui->sub_charset->value(m);
    else if ((m = ui->sub_charset->find_item(buf3)))
        ui->sub_charset->value(m);
    else
        ui->sub_charset->value(0);

    sprintf(buf2, "%s_name", buf);
    sprintf(buf4, "%s_name", lang);
    config_getvar(config, buf2, buf3, sizeof(buf3));
    config_getvar(config, buf4, buf5, sizeof(buf5));
    if ((m = ui->menu_lang->find_item(buf5)))
        ui->menu_lang->value(m);
    else if ((m = ui->menu_lang->find_item(buf3)))
        ui->menu_lang->value(m);
    else
        ui->menu_lang->value(0);

    config_destroy(config);

    return 1;
}

int write_language_settings(GeneratorUI *ui)
{
    struct lang_info *l;

    if (!ui->menu_lang->mvalue()) {
        fl_alert("Please pick menu language.\n");
        return 0;
    }
    l = (struct lang_info*)ui->menu_lang->mvalue()->user_data();

    if (target_arch == TARGET_ARCH_I386 || target_arch == TARGET_ARCH_X86_64) {
        config_t *config, *config2;

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        config2 = config_open(PATH_BASEISO "/boot/pxelinux.cfg/default", 0);
        if (!config || !config2) {
            fl_alert("Failed to open for write isolinux configuration.\n");
            return 0;
        }

        config_setvar(config, "lang", l->shortname);
        config_setvar(config2, "lang", l->shortname);

        config_write(config, PATH_BASEISO "/boot/isolinux.cfg");
        config_write(config2, PATH_BASEISO "/boot/pxelinux.cfg/default");
        config_destroy(config);
        config_destroy(config2);
    }
    else if (target_arch == TARGET_ARCH_POWERPC) {
        config_t *config, *config2;

        config = config_open(PATH_BASEISO "/boot/yaboot.conf", 0);
        config2 = config_open(PATH_BASEISO "/boot/netboot/yaboot.conf", 0);
        if (!config || !config2) {
            fl_alert("Failed to open for write yaboot configuration.\n");
            return 0;
        }

        config_setvar(config, "lang", l->shortname);
        config_setvar(config2, "lang", l->shortname);

        config_write(config, PATH_BASEISO "/boot/yaboot.conf");
        config_write(config2, PATH_BASEISO "/boot/netboot/yaboot.conf");
        config_destroy(config);
        config_destroy(config2);
    }

    return 1;
}

static void show_font_warning(const char *font)
{
    fl_alert("Font '%s' is missing.\nBut exists as an external package, please download it using the Packages tab.\n", font);
}

int copy_language_files(GeneratorUI *ui)
{
    char buf[256], buf2[256], *tmp;
    const char *filename;
    struct charset_info *c;
    struct lang_info *l;
    FILE *fp;

    if (!ui->menu_lang->mvalue()) {
        fl_alert("Please pick menu language.\n");
        return 0;
    }
    if (!ui->sub_charset->mvalue()) {
        fl_alert("Please pick subtitle character set.\n");
        return 0;
    }

    l = (struct lang_info*)ui->menu_lang->mvalue()->user_data();
    c = (struct charset_info*)ui->sub_charset->mvalue()->user_data();

    if (!fl_filename_isdir(PATH_BASEISO "/etc/installator"))
        my_mkdir(PATH_BASEISO "/etc/installator");

    if (!fl_filename_isdir(PATH_BASEISO "/etc/configurator"))
        my_mkdir(PATH_BASEISO "/etc/configurator");

    sprintf(buf2, "i18n/texts/help_en.txt");
    sprintf(buf, PATH_BASEISO "/usr/share/mplayer/help_en.txt");
    copy_file(buf2, buf);

    strcpy(buf2, "i18n/texts/en.lang");
    strcpy(buf, PATH_BASEISO "/etc/mplayer/en.lang");
    copy_file(buf2, buf);

    strcpy(buf2, "i18n/texts/en.install");
    strcpy(buf, PATH_BASEISO "/etc/installator/en.install");
    copy_file(buf2, buf);

    strcpy(buf2, "i18n/texts/en.config");
    strcpy(buf, PATH_BASEISO "/etc/configurator/en.config");
    copy_file(buf2, buf);

    if (strcmp(l->shortname, "en")) {
        sprintf(buf2, "i18n/texts/help_%s.txt", l->shortname);
        sprintf(buf, PATH_BASEISO "/usr/share/mplayer/help_%s.txt", l->shortname);
        if (file_exists(buf2))
            copy_file(buf2, buf);

        sprintf(buf2, "i18n/texts/%s.lang", l->shortname);
        sprintf(buf, PATH_BASEISO "/etc/mplayer/%s.lang", l->shortname);
        if (file_exists(buf2))
            copy_file(buf2, buf);

        sprintf(buf2, "i18n/texts/%s.install", l->shortname);
        sprintf(buf, PATH_BASEISO "/etc/installator/%s.install", l->shortname);
        if (file_exists(buf2))
            copy_file(buf2, buf);

        sprintf(buf2, "i18n/texts/%s.config", l->shortname);
        sprintf(buf, PATH_BASEISO "/etc/configurator/%s.config", l->shortname);
        if (file_exists(buf2))
            copy_file(buf2, buf);
    }

    sprintf(buf2, "i18n/lang.conf");
    sprintf(buf, PATH_BASEISO "/etc/lang.conf");
    copy_file(buf2, buf);

    fp = fopen(PATH_BASEISO "/etc/subfont", "wb");
    if (!fp) {
        fl_alert("Failed to write language configuration (%s).\n", "/etc/subfont");
        return 0;
    }
    fprintf(fp, "%s", c->name);
    fclose(fp);

    fp = fopen(PATH_BASEISO "/etc/lang.conf", "ab");
    if (!fp) {
        fl_alert("Failed to write language configuration (%s).\n", "/etc/lang.conf");
        return 0;
    }

    if (ui->override_sub_font->value()) {
        filename = fl_filename_name(ui->sub_font->value());

        sprintf(buf, PATH_BASEISO "/usr/share/fonts/%s", filename);
        copy_file(ui->sub_font->value(), buf);

        fprintf(fp, "%s_subfont=\"%s\"\n", c->codename, filename);
    } else {
        sprintf(buf2, PATH_FONTS "/%s", c->sub_font);
        sprintf(buf, PATH_BASEISO "/usr/share/fonts/%s", c->sub_font);

        if (!file_exists(buf2))
        {
            fclose(fp);
            show_font_warning(c->sub_font);
            return 0;
        }

        copy_file(buf2, buf);
    }

    /* Menu Font */
    if (ui->override_menu_font->value()) {
        filename = fl_filename_name(ui->menu_font->value());

        sprintf(buf, PATH_BASEISO "/usr/share/fonts/%s", filename);
        copy_file(ui->menu_font->value(), buf);

        fprintf(fp, "%s_menufont=\"%s\"\n", l->c->codename, filename);
    } else if ((tmp = valid_theme_font(ui->theme->mvalue()->label(), l->c))) {
        strcpy(buf, PATH_BASEISO "/usr/share/fonts/themefont.ttf");
        copy_file(tmp, buf);

        free(tmp);
    } else {
        sprintf(buf2, PATH_FONTS "/%s", l->c->menu_font);
        sprintf(buf, PATH_BASEISO "/usr/share/fonts/%s", l->c->menu_font);

        if (!file_exists(buf2))
        {
            fclose(fp);
            show_font_warning(l->c->menu_font);
            return 0;
        }

        copy_file(buf2, buf);
    }

    fclose(fp); /* close /etc/lang.conf */

    return 1;
}
