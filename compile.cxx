/*
 *  Compliation code for GeeXboX FLTK Generator
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

#include "audio.h"
#include "compile.h"
#include "config.h"
#include "fs.h"
#include "language.h"
#include "network.h"
#include "remote.h"
#include "theme.h"
#include "system.h"

#include <stdlib.h> /* system */
#include <stdio.h> /* FILE */

#include <FL/fl_ask.H> /* fl_alert */

void update_progress(GeneratorUI *ui, const char *msg)
{
    ui->progress->value(ui->progress->value() + 1);
    ui->progress->label(msg);
    Fl::check();

    fprintf(stderr, "%d = %s\n", (int)ui->progress->value(), msg);

    my_msleep(55);
}

static int write_geexbox_files(GeneratorUI *ui)
{
    const struct {
	const char *name;
	int (*func) (GeneratorUI *ui);
    } funcs[] = {
      { "Copying theme files...", copy_theme_files },
      { "Copying language files...", copy_language_files },
      { "Copying remote files...", copy_remote_files },
      { "Writing audio settings...", write_audio_settings },
      { "Writing network settings...", write_network_settings },
    };
    unsigned int i;
    for (i = 0; i < sizeof(funcs)/sizeof(funcs[0]); i++)
    {
	update_progress(ui, funcs[i].name);
	if (!(funcs[i].func)(ui) || copy_errors > 0)
	    return 0;
    }
    return 1;
}

static void cleanup_isotree(void)
{
    multi_delete(PATH_BASEISO "/usr/share/iconv/", NULL, NULL, 0);
    multi_delete(PATH_BASEISO "/usr/share/fonts/", NULL, ".ttf", 0);
    multi_delete(PATH_BASEISO "/usr/share/mplayer/", "help_", ".txt", 0);
    multi_delete(PATH_BASEISO "/etc/mplayer/", "menu_", ".conf", 0);
    multi_delete(PATH_BASEISO "/etc/", "lirc", NULL, 0);

    unlink(PATH_BASEISO "/usr/share/mplayer/background.avi");
    unlink(PATH_BASEISO "/usr/share/mplayer/background-audio.avi");
    unlink(PATH_BASEISO "/usr/share/grub-splash.xpm.gz");
    unlink(PATH_BASEISO "/etc/theme.conf");
    unlink(PATH_BASEISO "/etc/lang.conf");
    unlink(PATH_BASEISO "/etc/lang");
    unlink(PATH_BASEISO "/etc/subfont");
}

static void cleanup_zisotree(void)
{
    multi_delete("ziso/", NULL, NULL, 1);
    rmdir("ziso");
}

static int compile_zisotree(void)
{
    char buf[256];

    sprintf(buf, PATH_MKZFTREE " " PATH_BASEISO " ziso/GEEXBOX");
    return system(buf) == 0;
}

static int compile_isoimage(GeneratorUI *ui)
{
    char buf[2048];
    char iso_image[256];

    sprintf(iso_image, "geexbox-custom-%s.iso", ((struct lang_info*)ui->menu_lang->mvalue()->user_data())->shortname);

    sprintf(buf, PATH_MKISOFS " -o \"%s\" -quiet -no-pad -V GEEXBOX -volset GEEXBOX -P \"The GeeXboX team (www.geexbox.org)\" -p \"The GeeXboX team (www.geexbox.org)\" -A \"MKISOFS ISO 9660/HFS FILESYSTEM BUILDER\" -z -f -D -r -J -b GEEXBOX/boot/isolinux.bin -c GEEXBOX/boot/boot.catalog -sort sort -no-emul-boot -boot-load-size 4 -boot-info-table ziso", iso_image);
    return system(buf) == 0;
}

static int real_compile_iso(GeneratorUI *ui)
{
    ui->progress->minimum(0);
    ui->progress->maximum(12);
    ui->progress->value(0);

    copy_errors = 0;

    update_progress(ui, "Cleaning work tree...");
    cleanup_isotree();

    if (!write_geexbox_files(ui))
	return 0;

    update_progress(ui, "Creating ziso tree...");
    cleanup_zisotree();
    if (my_mkdir("ziso", 0700) < 0) {
	fl_alert("Failed to create temporery ziso directory.\n");
	return 0;
    }

    if (!compile_zisotree()) {
	fl_alert("Failed to create ziso tree.\n");
	return 0;
    }

    update_progress(ui, "Cleaning temporery files...");
    cleanup_isotree();

    update_progress(ui, "Copying boot files...");
    my_mkdir("ziso/GEEXBOX/boot", 0700);
    multi_copy("iso/GEEXBOX/boot/", "ziso/GEEXBOX/boot/", "");
    multi_copy("iso/", "ziso/", "GEEXBOX");

    if (copy_errors > 0)
	return 0;

    if (!append_bootplash_to_initrd(ui))
	return 0;

    update_progress(ui, "Compiling iso file...");
    if (!compile_isoimage(ui)) {
	fl_alert("Failed to create ISO image.\n");
	return 0;
    }

    update_progress(ui, "Cleaning ziso tree...");

    cleanup_zisotree();

    update_progress(ui, "DONE");

    return 1;
}

int compile_iso(GeneratorUI *ui)
{
    int rc;

    ui->setting_tabs->deactivate();
    ui->compile_button->deactivate();
    Fl::check();

    rc = real_compile_iso(ui);

    ui->setting_tabs->activate();
    ui->compile_button->activate();

    return rc;
}

void cleanup_compile(void)
{
    cleanup_isotree();
    cleanup_zisotree();
}
