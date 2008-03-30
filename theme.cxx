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
#include "system.h"
#include "theme.h"
#include "utils.h"
#include "video.h"

#include <sys/types.h>
#include <string.h> /* strcmp strncmp strlen strcpy strcat strstr */
#include <stdlib.h> /* free */
#include <stdio.h>

#include <FL/fl_ask.H> /* fl_alert */
#include <FL/filename.H> /* fl_filename_list */

static const char *path_lzma, *path_cpio;

int init_theme_tab(GeneratorUI *ui)
{
    char buf[50];
    FILE *f;
    const Fl_Menu_Item *m;

    update_theme_tab(ui);

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

    path_lzma = find_program("lzma");
    path_cpio = find_program("cpio");

    return 1;
}

void update_theme_tab(GeneratorUI *ui)
{
    int num_files, i;
    struct dirent **files;
    char *fname;

    if ((num_files = fl_filename_list("themes/", &files, NULL)) > 0)
    {
	for (i = 0; i < num_files; i++)
	{
	    fname = files[i]->d_name;
	    if (fname[strlen(fname)-1] == '/')
		fname[strlen(fname)-1] = '\0';
	    if (!strncmp("theme-", fname, 6) && !ui->theme->find_item(fname))
		ui->theme->add(&fname[6]);
	    free((void*)files[i]);
	}
	free((void*)files);
    }
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

    sprintf(buf, "themes/theme-%s/background-wide.avi", theme);
    sprintf(buf2, PATH_BASEISO "/usr/share/mplayer/background-wide.avi");
    if (file_exists(buf))
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

static int unlzma(const char *src, const char *dst)
{
    char buf[2048];

    snprintf(buf, sizeof(buf), "%s d \"%s\" \"%s\"", path_lzma, src, dst);
    return execute_bg_program(buf) == 0;
}

static int lzma(const char *src, const char *dst)
{
    char buf[2048];

    snprintf(buf, sizeof(buf), "%s e \"%s\" \"%s\"", path_lzma, src, dst);
    return execute_bg_program(buf) == 0;
}

static int cpio_append(const char *archive, const char *list)
{
    char buf[2048];

    snprintf(buf, sizeof(buf), "%s -o -A -H newc -F \"%s\" < \"%s\"", path_cpio, archive, list);
    return execute_bg_program(buf) == 0;
}

int copy_theme_boot_files(GeneratorUI *ui)
{
    char buf[256], buf2[256], bootsplash[256];
    const char *theme = ui->theme->mvalue()->label();
    FILE *f;

    if (ui->video_splash->value() && ui->vesa_res->value() != GeneratorUI::VESA_CUSTOM)
    {
	sprintf(buf, "themes/theme-%s/bootsplash-%s.dat", theme, get_target_resolution(ui));
	sprintf(bootsplash, "./bootsplash");

	if (!file_exists(buf)) {
	    fl_alert("Theme %s doesn't have bootsplash data for resolution: %s.\nPlease disable bootsplash screen, or switch to a different resolution.", theme, get_target_resolution(ui));
	    return 0;
	}

	copy_file(buf, bootsplash);

	sprintf(buf, PATH_BASEISO "/boot/initrd.gz");
	sprintf(buf2, "ziso/GEEXBOX/boot/initrd");

	if (!unlzma(buf, buf2)) {
	    fl_alert("Failed to uncompress initrd.\n");
	    return 0;
	}

	/* List of files appended to cpio archive, only ./bootsplash here */
	f = fopen("./cpio_list", "wb");
	if (f) {
	    fprintf(f, bootsplash);
	    fclose(f);
	}

	if (!cpio_append(buf2, "./cpio_list")) {
	    fl_alert("Failed to append bootsplash to initrd.\n");
	    return 0;
	}

	sprintf(buf, "ziso/GEEXBOX/boot/initrd.gz");

	if (!lzma(buf2, buf)) {
	    fl_alert("Failed to compress initrd.\n");
	    return 0;
	}

	unlink(buf2);
    }

    if (target_arch == TARGET_ARCH_I386) {
	sprintf(buf, "themes/theme-%s/splash-isolinux.rle", theme);
	sprintf(buf2, "ziso/GEEXBOX/boot/splash.rle");
	if (file_exists(buf))
	    copy_file(buf, buf2);
    }

    return 1;
}
