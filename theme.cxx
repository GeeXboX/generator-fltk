/*
 *  Theme support for GeeXboX FLTK Generator
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

#include <sys/types.h>
#include <dirent.h> /* opendir */
#include <string.h> /* strcmp strncmp strlen strcpy strcat strstr */

#include <FL/fl_ask.H> /* fl_alert */

int init_theme_tab(GeneratorUI *ui)
{
    char buf[50];
    DIR *dirp;
    struct dirent *dp;
    FILE *f;
    const Fl_Menu_Item *m;

    dirp = opendir("themes");
    if (dirp)
    {
	while ((dp = readdir(dirp)) != NULL)
	    if (!strncmp("theme-", dp->d_name, 6))
		ui->theme->add(&dp->d_name[6]);
	closedir(dirp);
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
    DIR *dirp;
    struct dirent *dp;
    FILE *fp;

    sprintf(buf, "themes/theme-%s/config", theme_name);
    fp = fopen(buf, "r");
    if (!fp)
	return NULL;

    get_shvar_value(fp, "FONT_CHARSETS", buf);
    if (!buf[0])
	strcpy(buf, "iso-8859-1 ");
    else
	strcat(buf, " ");

    fclose(fp);

    sprintf(buf2, "%s ", c->name);
    if (!strstr(buf, buf2))
	return NULL;

    sprintf(buf, "themes/theme-%s", theme_name);
    dirp = opendir(buf);
    if (!dirp)
	return NULL;

    while ((dp = readdir(dirp)) != NULL)
    {
	if (!strcmp(".ttf", &dp->d_name[strlen(dp->d_name)-4]))
	{
	    sprintf(buf2, "%s/%s", buf, dp->d_name);
	    closedir(dirp);
	    return my_strdup(buf2);
	}
    }
    closedir(dirp);

    return NULL;
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

    sprintf(buf, "themes/theme-%s/grub-splash.xpm.gz", theme);
    sprintf(buf2, PATH_BASEISO "/usr/share/grub-splash.xpm.gz");
    if (file_exists(buf))
	copy_file(buf, buf2);

    sprintf(buf, "themes/theme-%s/splash-isolinux.rle", theme);
    sprintf(buf2, PATH_BASEISO "/boot/splash.rle");
    if (!file_exists(buf)) {
	fl_alert("Theme %s is missing theme %s file.\n", theme, "splash-isolinux.rle");
	return 0;
    }
    copy_file(buf, buf2);

    sprintf(buf, "themes/theme-%s/config", theme);
    sprintf(buf2, PATH_BASEISO "/etc/theme.conf");
    if (!file_exists(buf)) {
	fl_alert("Theme %s is missing theme %s file.\n", theme, "config");
	return 0;
    }
    copy_file(buf, buf2);

    return 1;
}

int append_bootplash_to_initrd(GeneratorUI *ui)
{
    char buf[256], buf2[256];

    sprintf(buf, "themes/theme-%s/bootsplash.dat", ui->theme->mvalue()->label());
    sprintf(buf2, PATH_BASEISO "/boot/initrd.gz");

    if (file_exists(buf) && _copy_file(buf, buf2, 1)) {
	fl_alert("Failed to append bootplash data to initrd image.\n");
	return 0;
    }

    return 1;
}
