/*
 *  Main code for GeeXboX FLTK Generator
 *  Copyright (C) 2005-2007  Amir Shalem
 *                           Mathieu Schroeter
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
#include "curl.h"
#include "dvdnav.h"
#include "keymap.h"
#include "language.h"
#include "lcd.h"
#include "network.h"
#include "packages.h"
#include "remote.h"
#include "theme.h"
#include "video.h"

#include <stdlib.h> /* exit */
#include <stdio.h> /* fprintf */

#include <FL/Fl.H> /* Fl::run Fl::scheme */

void update_tabs_status(GeneratorUI *ui)
{
    if (ui->lcd_enabled->value()) {
	ui->lcd_model->activate();
	ui->lcd_width->activate();
	ui->lcd_height->activate();
    }
    else {
	ui->lcd_model->deactivate();
	ui->lcd_width->deactivate();
	ui->lcd_height->deactivate();
    }

    if (ui->vesa_res->value() == GeneratorUI::VESA_CUSTOM) {
	ui->vesa_custom->activate();
	ui->vesa_custom->show();
	ui->vesa_depth->deactivate();
	ui->video_splash->deactivate();
    }
    else {
	ui->vesa_custom->deactivate();
	ui->vesa_custom->hide();

	if (target_arch == TARGET_ARCH_I386) {
	    ui->vesa_depth->activate();
	}
	ui->video_splash->activate();
    }

    if (ui->streaming_shoutcasttv->value()) {
	ui->streaming_whitelist->activate();
	ui->streaming_blacklist->activate();
    }
    else {
	ui->streaming_whitelist->deactivate();
	ui->streaming_blacklist->deactivate();
    }

    if (ui->override_sub_font->value())
	ui->sub_font->activate();
    else
	ui->sub_font->deactivate();

    if (ui->override_menu_font->value())
	ui->menu_font->activate();
    else
	ui->menu_font->deactivate();

    if (ui->server_ftp->value()) {
        ui->ftp_user->activate();
        ui->ftp_pass->activate();
    }
    else {
        ui->ftp_user->deactivate();
        ui->ftp_pass->deactivate();
    }

    switch (ui->soundcard_mode->value())
    {
    case GeneratorUI::SOUNDCARD_MODE_SPDIF:
	ui->hwac3->activate();
	ui->ac97_spsa->activate();
	break;
    case GeneratorUI::SOUNDCARD_MODE_ANALOG:
	ui->hwac3->deactivate();
	ui->channels->activate();
	ui->ac97_spsa->deactivate();
	break;
    }

    switch (ui->phy_iface->value())
    {
    case GeneratorUI::NETWORK_PHY_IFACE_AUTO:
    case GeneratorUI::NETWORK_PHY_IFACE_WIFI:
	ui->wifi_settings->activate();
	break;
    case GeneratorUI::NETWORK_PHY_IFACE_ETHER:
	ui->wifi_settings->deactivate();
	break;
    }

    switch (ui->wifi_enc->value())
    {
    case GeneratorUI::WIFI_ENC_WPA:
	ui->wifi_key->activate();
	ui->wpa_drv->activate();
	break;
    case GeneratorUI::WIFI_ENC_WEP:
	ui->wifi_key->activate();
	ui->wpa_drv->deactivate();
	break;
    case GeneratorUI::WIFI_ENC_NONE:
	ui->wifi_key->deactivate();
	ui->wpa_drv->deactivate();
	break;
    }

    switch (ui->network_conf->value())
    {
    case GeneratorUI::NETWORK_CONF_MANUAL:
	ui->network_ip->activate();
	ui->network_subnet->activate();
	ui->network_gateway->activate();
	ui->network_dns->activate();
	break;
    case GeneratorUI::NETWORK_CONF_DHCP:
	ui->network_ip->deactivate();
	ui->network_subnet->deactivate();
	ui->network_gateway->deactivate();
	ui->network_dns->deactivate();
	break;
    }

    update_theme_tab(ui);
}

void generator_exit(GeneratorUI *ui)
{
    cleanup_compile();
    exit(0);
}

static int init_tabs(GeneratorUI *ui)
{
    return 1 
	&& init_compile(ui)
	&& init_language_tab(ui)
	&& init_autoplay_tab(ui)
	&& init_dvdnav_tab(ui)
	&& init_keymap_tab(ui)
	&& init_audio_tab(ui)
	&& init_video_tab(ui)
	&& init_remote_tab(ui)
	&& init_network_tab(ui)
	&& init_lcd_tab(ui)
	&& init_theme_tab(ui)
	&& init_curl()
	&& init_packages_tab(ui)
	;
}

#ifdef __APPLE__
#include <CoreServices/CoreServices.h>

void setMacResources(const char *filename)
{
    FSRef fsref;        /* File Reference */
    FSSpec fsspec;      /* FileSpec */
    short rref;         /* Resource Reference */

    if (FSPathMakeRef((const UInt8*)filename, &fsref, NULL))
	return;
      
    if (FSGetCatalogInfo(&fsref, kFSCatInfoNone, NULL, NULL, &fsspec, NULL))
	return;

    rref = FSpOpenResFile(&fsspec, fsRdPerm);
    switch (ResError())
    {
    case eofErr:
	FSpCreateResFile(&fsspec, 0, 0, smCurrentScript);
	break;
    case 0:
	CloseResFile(rref);
	break;
    }
}
#endif

int
main(int argc, char **argv) 
{
#ifdef __APPLE__
    setMacResources(argv[0]);
#endif

    Fl::scheme("plastic");

    GeneratorUI *ui = new GeneratorUI;

    if (!find_geexbox_tree(argv[0]) || tree_corrupted())
	return 1;

    if (!init_tabs(ui)) {
	fprintf(stderr, "Tabs initilizing failed.\n");
	return 1;
    }
    ui->license_window = NULL;

    update_tabs_status(ui);

    ui->show(argc, argv);

    // take focus
    ui->interface_tab->show();

    return Fl::run();
}
