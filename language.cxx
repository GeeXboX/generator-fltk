/*
 *  Language support for GeeXboX FLTK Generator
 *  Copyright (C) 2005  Amir Shalem
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "generatorUI.h"

#include "config.h"
#include "language.h"
#include "theme.h"
#include "utils.h"

#include <FL/fl_ask.H> /* fl_alert */
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
    char buf2[50], buf3[256];
    FILE *f;
    struct lang_info *l;
    struct charset_info *c;
    const Fl_Menu_Item *m;

    f = fopen(PATH_LANGCONF, "r");
    if (!f) {
	fl_alert("Missing language configuration files.\n");
	return 0;
    }

    get_shvar_value(f, "CHARSETS", buf);
    for (word = strtok(buf, " \t"); word; word = strtok(NULL, " \t"))
    {
	c = (struct charset_info*)malloc(sizeof(struct charset_info));
	if (c)
	{
	    ui->sub_charset->add(word, 0, NULL, c);

	    c->menu_font = c->sub_font = NULL;
	    c->name = my_strdup(word);

	    replace_char(word, '-', '_');

	    c->codename = my_strdup(word);

	    sprintf(buf2, "%s_menufont", word);
	    if (get_shvar_value(f, buf2, buf3) && buf3[0])
		c->menu_font = my_strdup(buf3);

	    sprintf(buf2, "%s_subfont", word);
	    if (get_shvar_value(f, buf2, buf3) && buf3[0])
		c->sub_font = my_strdup(buf3);

	    sprintf(buf2, "%s_font", word);
	    if (!get_shvar_value(f, buf2, buf3) || !buf3[0])
	        get_shvar_value(f, "DEFAULT_FONT", buf3);
	    if (!c->menu_font)
	        c->menu_font = my_strdup(buf3);
	    if (!c->sub_font)
	        c->sub_font = my_strdup(buf3);
	}
    }

    get_shvar_value(f, "LANGUAGES", buf);
    for (word = strtok(buf, " \t"); word; word = strtok(NULL, " \t"))
    {
	sprintf(buf2, "%s_name", word);
	get_shvar_value(f, buf2, buf3);

	l = (struct lang_info*)malloc(sizeof(struct lang_info));
	if (l)
	{
	    l->c = NULL;
	    ui->menu_lang->add(buf3, 0, NULL, l);

	    l->shortname = my_strdup(word);

	    sprintf(buf2, "%s_charset", word);
	    get_shvar_value(f, buf2, buf3);
	    m = ui->sub_charset->find_item(buf3);
	    if (!m) {
		fl_alert("Missing language character set %s.\n", buf3);
		fclose(f);
		return 0;
	    }
	    l->c = (struct charset_info*)m->user_data();
	}
    }

    if (ui->sub_charset->size() < 1 || ui->menu_lang->size() < 1) {
	fl_alert("No languages or character set found.\n");
	return 0;
    }

    get_shvar_value (f, "DEFAULT_LANGUAGE", buf);

    sprintf(buf2, "%s_charset", buf);
    get_shvar_value (f, buf2, buf3);
    if ((m = ui->sub_charset->find_item(buf3)))
	ui->sub_charset->value(m);
    else
	ui->sub_charset->value(0);

    sprintf(buf2, "%s_name", buf);
    get_shvar_value (f, buf2, buf3);
    if ((m = ui->menu_lang->find_item(buf3)))
	ui->menu_lang->value(m);
    else
	ui->menu_lang->value(0);

    fclose(f);

    return 1;
}

static int copy_charset_iconv(const char *charset)
{
    char buf[256], buf2[256], buf3[256];
    int charset_len;
    FILE *f;

    f = fopen(PATH_ICONV "/charset.db", "r");
    if (!f) {
	fl_alert("Failed to read iconv character set database.\n");
	return 0;
    }

    charset_len = strlen(charset);

    while (!feof(f))
	if (fscanf(f, "%s %s", buf, buf3) == 2 && !strcmp(charset, buf))
	{
	    sprintf(buf2, PATH_ICONV "/%s", buf3);
	    sprintf(buf, PATH_BASEISO "/usr/share/iconv/%s", buf3);
	    copy_file(buf2, buf);
	}

    fclose(f);

    return 1;
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

    sprintf(buf2, "i18n/texts/help_%s.txt", l->shortname);
    sprintf(buf, PATH_BASEISO "/usr/share/mplayer/help_%s.txt", l->shortname);
    copy_file(buf2, buf);

    sprintf(buf2, "i18n/texts/menu_%s.conf", l->shortname);
    sprintf(buf, PATH_BASEISO "/etc/mplayer/menu_%s.conf", l->shortname);
    copy_file(buf2, buf);

    sprintf(buf2, "i18n/lang.conf");
    sprintf(buf, PATH_BASEISO "/etc/lang.conf");
    copy_file(buf2, buf);

    fp = fopen(PATH_BASEISO "/etc/lang", "wb");
    if (!fp) {
	fl_alert("Failed to write language configuration (%s).\n", "/etc/lang");
	return 0;
    }
    fprintf(fp, "%s", l->shortname);
    fclose(fp);

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
	filename = find_basename(ui->sub_font->value());

	sprintf(buf, PATH_BASEISO "/usr/share/fonts/%s", filename);
	copy_file(ui->sub_font->value(), buf);

	fprintf(fp, "%s_subfont=\"%s\"\n", c->codename, filename);
    } else {
	sprintf(buf2, PATH_FONTS "/%s", c->sub_font);
	sprintf(buf, PATH_BASEISO "/usr/share/fonts/%s", c->sub_font);

	if (!file_exists(buf2)) 
	{
	    fl_alert("Font '%s' is missing.\nPlease visit README - EXTRA SUBTITLE FONT section.\n", c->sub_font);
	    return 0;
	}

	copy_file(buf2, buf);
    }

    /* Menu Font */
    if (ui->override_menu_font->value()) {
	filename = find_basename(ui->menu_font->value());

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
	    fl_alert("Font '%s' is missing.\nPlease visit README - EXTRA SUBTITLE FONT section.\n", l->c->menu_font);
	    return 0;
	}

	copy_file(buf2, buf);
    }

    fclose(fp); /* close /etc/lang.conf */

    if (!copy_charset_iconv(c->name) || !copy_charset_iconv(l->c->name))
	return 0;

    return 1;
}
