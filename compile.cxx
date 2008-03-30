/*
 *  Compliation code for GeeXboX FLTK Generator
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

#include "audio.h"
#include "autoplay.h"
#include "compile.h"
#include "config.h"
#include "dvdnav.h"
#include "extrafiles.h"
#include "fs.h"
#include "keymap.h"
#include "language.h"
#include "lcd.h"
#include "mplayer.h"
#include "network.h"
#include "nfs.h"
#include "packages.h"
#include "remote.h"
#include "samba.h"
#include "theme.h"
#include "system.h"
#include "video.h"

#include <stdio.h> /* FILE */
#include <stdlib.h> /* free */

#include <FL/filename.H> /* fl_filename_isdir */
#include <FL/fl_ask.H> /* fl_alert */
#include <FL/Fl.H> /* Fl::check */

static char *geexbox_version;
target_arch_t target_arch;

static const char *path_mkzftree, *path_mkisofs;

const char *get_target_arch_string(void)
{
    switch (target_arch)
    {
    case TARGET_ARCH_I386:
	return "i386";
    case TARGET_ARCH_POWERPC:
	return "powerpc";
    default:
	return NULL;
    }
}

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
      { "Writing language settings...", write_language_settings },
      { "Writing remote settings...", write_remote_settings },
      { "Writing DVD navigation settings...", write_dvdnav_settings },
      { "Writing autoplay settings...", write_autoplay_settings },
      { "Writing keymap settings...", write_keymap_settings },
      { "Writing audio settings...", write_audio_settings },
      { "Writing video settings...", write_video_settings },
      { "Writing MPlayer settings...", write_mplayer_settings },
      { "Writing network settings...", write_network_settings },
      { "Writing nfs settings...", write_nfs_settings },
      { "Writing samba settings...", write_samba_settings },
      { "Writing lcd display settings...", write_lcd_settings },
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

static void cleanup_basetree(void)
{
    unlink("./bootsplash");
    unlink("./cpio_list");
}

static void cleanup_isotree(void)
{
    multi_delete(PATH_BASEISO "/usr/share/iconv/", NULL, NULL, 0);
    multi_delete(PATH_BASEISO "/usr/share/fonts/", NULL, ".ttf", 0);
    multi_delete(PATH_BASEISO "/usr/share/mplayer/", "help_", ".txt", 0);
    multi_delete(PATH_BASEISO "/etc/mplayer/", NULL, ".lang", 0);

    multi_delete(PATH_BASEISO "/etc/installator/", NULL, NULL, 1);
    multi_delete(PATH_BASEISO "/etc/configurator/", NULL, NULL, 1);
    rmdir(PATH_BASEISO "/etc/installator");
    rmdir(PATH_BASEISO "/etc/configurator");

    unlink(PATH_BASEISO "/usr/share/mplayer/background.avi");
    unlink(PATH_BASEISO "/usr/share/mplayer/background-wide.avi");
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

    sprintf(buf, "%s " PATH_BASEISO " ziso/GEEXBOX", path_mkzftree);
    return execute_bg_program(buf) == 0;
}

static int compile_isoimage(GeneratorUI *ui)
{
    char buf[2048];
    char iso_image[256];
    char *mkisofs_arch = NULL;
    switch (target_arch)
    {
    case TARGET_ARCH_I386:
	mkisofs_arch = "-no-emul-boot -boot-info-table -boot-load-size 4 -b GEEXBOX/boot/isolinux.bin -c GEEXBOX/boot/boot.catalog";
	break;
    case TARGET_ARCH_POWERPC:
	mkisofs_arch = "-hfs -part -no-desktop -map maps -hfs-volid GEEXBOX -hfs-bless ziso/GEEXBOX/boot";
	break;
    }

    sprintf(iso_image, "geexbox-%s-%s.%s.iso", geexbox_version, ((struct lang_info*)ui->menu_lang->mvalue()->user_data())->shortname, get_target_arch_string());

    sprintf(buf, "%s -o \"%s\" -quiet -no-pad -V GEEXBOX -volset GEEXBOX -publisher \"The GeeXboX team (www.geexbox.org)\" -p \"The GeeXboX team (www.geexbox.org)\" -A \"MKISOFS ISO 9660/HFS FILESYSTEM BUILDER\" -z -D -r -J -sort sort -f %s ziso", path_mkisofs, iso_image, mkisofs_arch);
    return execute_bg_program(buf) == 0;
}

static void compile_extrafiles(GeneratorUI *ui)
{
    char dst[256];
    char dst_leaf[256], path_leaf[256];
    Flu_Tree_Browser::Node *n, *n2;
    Extrafile *e;

    for (n = ui->extrafiles_tree->first(); n; n = n->next()) {
        e = (Extrafile*)n->user_data();
        if (e && e->path != NULL && file_exists(e->path)) {
            if (n->is_branch()) {
                snprintf(dst, sizeof(dst), "ziso/%s", n->label());
                my_mkdir(dst);
                n2 = ui->extrafiles_tree->first_leaf();
                while (n2) {
                    if (n->is_descendent(n2)) {
                        snprintf(dst_leaf, sizeof(dst_leaf), "%s/%s", dst, n2->label());
                        snprintf(path_leaf, sizeof(path_leaf), "%s/%s", e->path, n2->label());
                        if (file_exists(path_leaf))
#ifdef _WIN32
                            copy_file(path_leaf, dst_leaf);
#else
                            symlink(path_leaf, dst_leaf);
#endif
                    }
                    n2 = n2->next_leaf();
                }
            }
            else {
                snprintf(dst, sizeof(dst), "ziso/%s", n->label());
                if (file_exists(e->path))
#ifdef _WIN32
                    copy_file(e->path, dst);
#else
                    symlink(e->path, dst);
#endif
            }
        }
    }
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
    if (my_mkdir("ziso") < 0) {
	fl_alert("Failed to create temporary ziso directory.\n");
	return 0;
    }

    if (!compile_zisotree()) {
	fl_alert("Failed to create ziso tree.\n");
	return 0;
    }

    update_progress(ui, "Cleaning temporary files...");
    cleanup_isotree();

    update_progress(ui, "Copying boot files...");
    my_mkdir("ziso/GEEXBOX/boot");
    multi_copy("iso/GEEXBOX/boot/", "ziso/GEEXBOX/boot/", "");

    if (!copy_theme_boot_files(ui) || copy_errors > 0)
	return 0;

    update_progress(ui, "Copying extra files...");
    compile_extrafiles(ui);
    multi_copy("iso/", "ziso/", "GEEXBOX");

    update_progress(ui, "Compiling iso file...");
    if (!compile_isoimage(ui)) {
	fl_alert("Failed to create ISO image.\n");
	return 0;
    }

    update_progress(ui, "Cleaning ziso tree...");

    cleanup_zisotree();

    update_progress(ui, "Cleaning base tree...");

    cleanup_basetree();

    update_progress(ui, "DONE");

    return 1;
}

int compile_iso(GeneratorUI *ui, int check_packages)
{
    int rc = 1;

    if (!check_packages) {
        ui->setting_tabs->deactivate();
        ui->compile_button->deactivate();
        Fl::check();

        rc = real_compile_iso(ui);

        ui->setting_tabs->activate();
        ui->compile_button->activate();
    }
    else
        /* run the compilation after the download */
        package_download(ui, 1);

    return rc;
}

void cleanup_compile(void)
{
    destroy_bg_program();
    cleanup_basetree();
    cleanup_isotree();
    cleanup_zisotree();
}

const char *find_program(const char *prog)
{
    char buf[100];

#ifdef _WIN32
    sprintf(buf, "tools\\win32\\%s.exe", prog);
#elif defined(__APPLE__)
    sprintf(buf, "tools/macosx/%s", prog);
#elif defined(__linux__) && defined(__i386__)
    sprintf(buf, "tools/linux/i386/%s", prog);
#else
    return prog;
#endif

    return file_exists(buf) ? strdup(buf) : prog;
}

int init_compile(GeneratorUI *ui)
{
    char buf[512], *tmp;
    FILE *f;

    if (file_exists(PATH_BASEISO "/boot/isolinux.bin"))
	target_arch = TARGET_ARCH_I386;
    else if (file_exists(PATH_BASEISO "/boot/yaboot"))
	target_arch = TARGET_ARCH_POWERPC;
    else {
	fl_alert("Failed to detect iso target arch.");
	return 0;
    }

    f = fopen("VERSION", "r");
    if (!f || !fgets(buf, sizeof(buf), f)) {
	if (f)
	    fclose(f);
	fl_alert("Failed to detect GeeXboX version.");
	return 0;
    }
    fclose(f);
    if ((tmp = strchr(buf, '\n')))
	*tmp = '\0';
    geexbox_version = strdup(buf);

    sprintf(buf, "GeeXboX Generator %s %s", geexbox_version, get_target_arch_string());
    ui->mainWindow->label(strdup(buf));

    path_mkisofs  = find_program("mkisofs");
    path_mkzftree = find_program("mkzftree");

    return 1;
}

int find_geexbox_tree(const char *prog)
{
    char *orig_cwd;
    char *prog_path, *tmp;

    if (fl_filename_isdir(PATH_BASEISO)) 
	return 1;

    orig_cwd  = getcwd(0, 4096);
    prog_path = strdup(prog);
    if (prog_path) {
	tmp = (char*)fl_filename_name(prog_path);
	if (tmp != prog_path) {
	    *tmp = '\0';
	    if (!chdir(prog_path) && fl_filename_isdir(PATH_BASEISO)) {
		free(prog_path);
		if (orig_cwd)
		    free(orig_cwd);
		return 1;
	    }
	}
	free(prog_path);
    }
    fl_alert("Failed to find GeeXboX iso directory.\nOriginal working directory is: '%s'.\n", orig_cwd ? orig_cwd : "unknown");
    if (orig_cwd)
	free(orig_cwd);
    return 0;
}

int tree_corrupted(void)
{
    char buf[30];
    const char *msg = NULL, *msg2 = NULL;
    int err = 0;
    FILE *f;

    f = fopen(PATH_BASEISO "/sbin/init", "rb");
    if (f) {
	if (fgets(buf, sizeof(buf)-1, f)) {
	    if (strchr(buf, '\r'))
	    {
		msg = "CR/LF";
		err++;
	    }
	} else {
	    err++;
	}
	fclose(f);
    } else {
	err++;
    }

    if (!fl_filename_isdir(PATH_BASEISO "/usr/share/fonts")) {
	msg2 = "empty directories";
	err++;
    }

    if (msg || msg2)
	fl_alert("GeeXboX generator files are corrupted.\nThis probably means you have extracted the archive with WinZIP\n"
		 "or any kind of unarchiver that can't handle %s%s%s properly.\n\nPlease use real software like 7Zip (http://www.7-zip.org).",
		 msg ? msg : (msg2 ? msg2 : ""), (msg && msg2) ? " and " : "", (msg && msg2) ? msg2 : "");
    else if (err)
	fl_alert("GeeXboX generator files are corrupted.\n\nPlease use real software like 7Zip (http://www.7-zip.org) to extract the archive.");

    return (err > 0);
}
