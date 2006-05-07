/*
 *  Theme support for GeeXboX FLTK Generator
 *  Copyright (C) 2005-2006  Amir Shalem
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
#include "language.h"
#include "theme.h"
#include "utils.h"
#include "video.h"

#include <sys/types.h>
#include <string.h> /* strcmp strncmp strlen strcpy strcat strstr */
#include <stdlib.h> /* free */

#include <FL/fl_ask.H> /* fl_alert */
#include <FL/filename.H> /* fl_filename_list */

int init_theme_tab(GeneratorUI *ui)
{
    char buf[50];
    int num_files, i;
    struct dirent **files;
    char *fname;
    FILE *f;
    const Fl_Menu_Item *m;

    if ((num_files = fl_filename_list("themes/", &files, NULL)) > 0)
    {
	for (i = 0; i < num_files; i++)
	{
	    fname = files[i]->d_name;
	    if (!strncmp("theme-", fname, 6))
	    {
		if (fname[strlen(fname)-1] == '/')
		    fname[strlen(fname)-1] = '\0';
		ui->theme->add(&fname[6]);
	    }
	    free((void*)files[i]);
	}
	free((void*)files);
    }

    if (ui->theme->size() < 1) {
	fl_alert("Missing theme configuration files.\n");
	return 0;
    }

    ui->theme->value(0);
    if ((f = fopen(PATH_DEFAULTTHEME, "r")))
    {
	if (fscanf(f, "theme-%s", buf) == 1 && (m = ui->theme->find_item(buf)))
	    ui->theme->value(m);
	fclose(f);
    }

    return 1;
}

char *valid_theme_font(const char *theme_name, struct charset_info *c)
{
    char buf[256], buf2[256];
    int num_files, i;
    struct dirent **files;
    const char *fname;
    char *font;
    config_t *config;

    sprintf(buf, "themes/theme-%s/config", theme_name);
    config = config_open(buf, 1);
    if (!config)
	return NULL;

    config_getvar(config, "FONT_CHARSETS", buf, sizeof(buf));
    if (!buf[0])
	strcpy(buf, "iso-8859-1 ");
    else
	strcat(buf, " ");

    config_destroy(config);

    sprintf(buf2, "%s ", c->name);
    if (!strstr(buf, buf2))
	return NULL;

    sprintf(buf, "themes/theme-%s/", theme_name);
    if ((num_files = fl_filename_list(buf, &files, NULL)) <= 0)
	return NULL;

    font = NULL;
    i = num_files;
    while (i--)
    {
	fname = files[i]->d_name;
	if (!strcmp(".ttf", &fname[strlen(fname)-4]))
	{
	    sprintf(buf2, "%s/%s", buf, fname);
	    font = strdup(buf2);
	    break;
	}
	free((void*)files[i]);
    }
    while (i--)
	free((void*)files[i]);
    free((void*)files);

    return font;
}

int copy_theme_files(GeneratorUI *ui)
{
    char buf[256], buf2[256];
    const char *theme;

    if (!ui->theme->mvalue()) {
	fl_alert("Please pick theme.\n");
	return 0;
    }

    theme = ui->theme->mvalue()->label();

    sprintf(buf, "themes/theme-%s/background.avi", theme);
    sprintf(buf2, PATH_BASEISO "/usr/share/mplayer/background.avi");
    if (!file_exists(buf)) {
	fl_alert("Theme %s is missing theme %s file.\n", theme, "background.avi");
	return 0;
    }
    copy_file(buf, buf2);

    sprintf(buf, "themes/theme-%s/background-audio.avi", theme);
    sprintf(buf2, PATH_BASEISO "/usr/share/mplayer/background-audio.avi");
    if (file_exists(buf))
	copy_file(buf, buf2);

    if (target_arch == TARGET_ARCH_I386) {
	sprintf(buf, "themes/theme-%s/grub-splash.xpm.gz", theme);
	sprintf(buf2, PATH_BASEISO "/usr/share/grub-splash.xpm.gz");
	if (file_exists(buf))
	    copy_file(buf, buf2);
    }

    sprintf(buf, "themes/theme-%s/config", theme);
    sprintf(buf2, PATH_BASEISO "/etc/theme.conf");
    if (!file_exists(buf)) {
	fl_alert("Theme %s is missing theme %s file.\n", theme, "config");
	return 0;
    }
    copy_file(buf, buf2);

    return 1;
}

int copy_theme_boot_files(GeneratorUI *ui)
{
    char buf[256], buf2[256];
    const char *theme = ui->theme->mvalue()->label();

    if (ui->video_splash->value())
    {
	sprintf(buf, "themes/theme-%s/bootsplash-%s.dat", theme, get_target_resolution(ui));
	sprintf(buf2, "ziso/GEEXBOX/boot/initrd.gz");

	if (!file_exists(buf)) {
	    fl_alert("Theme %s doesn't have bootsplash data for resolution: %s\nPlease disable bootsplash screen, or switch to a different resolution.", theme, get_target_resolution(ui));
	    return 0;
	}

	if (_copy_file(buf, buf2, 1)) {
	    fl_alert("Failed to append bootplash data to initrd image.\n");
	    return 0;
	}
    }

    if (target_arch == TARGET_ARCH_I386) {
	sprintf(buf, "themes/theme-%s/splash-isolinux.rle", theme);
	sprintf(buf2, "ziso/GEEXBOX/boot/splash.rle");
	if (file_exists(buf))
	    copy_file(buf, buf2);
    }

    return 1;
}
