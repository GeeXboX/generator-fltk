/*
 *  Video support for GeeXboX FLTK Generator
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
#include "compile.h"
#include "configparser.h"
#include "video.h"
#include "system.h"
#include "fs.h"

#include <FL/fl_ask.H> /* fl_alert */

int init_video_tab(GeneratorUI *ui)
{
    char *video_res[] = {
	"640x480",
	"800x600",
	"1024x768",
	"1152x864",
	"1280x960",
	"1280x1024",
	"1400x1050",
	"1600x1200",
    };

    char *video_depth[] = {
	"15",
	"16",
	"24",
	"32",
    };

    char buf[256];

    for (unsigned long int i = 0; i < sizeof(video_res)/sizeof(video_res[0]); ++i)
    {
	ui->vesa_res->add(video_res[i], 0, NULL, video_res[i]);
    }
    ui->vesa_res->add("Custom", 0, NULL, NULL);

    for (unsigned long int i = 0; i < sizeof(video_depth)/sizeof(video_depth[0]); ++i)
    {
	sprintf(buf, "%s bits", video_depth[i]);
	ui->vesa_depth->add(buf, 0, NULL, video_depth[i]);
    }

    ui->vesa_res->value(ui->vesa_res->find_item("800x600"));
    ui->vesa_depth->value(ui->vesa_depth->find_item("24 bits"));

    if (target_arch == TARGET_ARCH_I386) {
        config_t *config;

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        if (!config) {
            fl_alert("Missing isolinux configuration files.\n");
            return 0;
        }

        config_getvar_location(config, "splash", 1, buf, sizeof(buf));
        ui->video_splash->value(my_strcasecmp(buf, "silent"));

        config_getvar(config, "video", buf, sizeof(buf));

	if (!strncmp("vesafb:", buf, 7))
	{
    	    const Fl_Menu_Item *m;
	    char *sep, *video_mode;
	    
	    video_mode = buf + 7;
	    sep = strchr(video_mode, '@');
	    if (!sep)
		sep = strchr(video_mode, ',');
	    if (sep)
	    {
		ui->vesa_res->user_data(strdup(sep));
		*sep = '\0';
	    } else
	    {
		ui->vesa_res->user_data(NULL);
	    }

	    sep = strchr(video_mode, '-');
	    if (sep)
	    {
		char buf2[15];

		*sep++ = '\0';
		sprintf(buf2, "%s bits", sep);

		if ((m = ui->vesa_depth->find_item(buf2)))
		    ui->vesa_depth->value(m);
	    }

	    if ((m = ui->vesa_res->find_item(video_mode)))
	    {
		ui->vesa_res->value(m);
	    } else
	    {
		ui->vesa_res->value(ui->vesa_res->find_item("Custom"));
		ui->vesa_custom->value(video_mode);
	    }
	}

        config_destroy(config);
    }
    else
    {
        ui->vesa_res->deactivate();
        ui->vesa_depth->deactivate();
        ui->video_splash->deactivate();
    }

    return 1;
}

int write_video_settings(GeneratorUI *ui)
{
    if (target_arch == TARGET_ARCH_I386) {
        char buf[1024];
        config_t *config;

        config = config_open(PATH_BASEISO "/boot/isolinux.cfg", 0);
        if (!config) {
            fl_alert("Failed to open for write isolinux configuration.\n");
            return 0;
        }

        config_setvar_location(config, "splash", 1,
				    ui->video_splash->value() ? "0" : "silent");

	sprintf(buf, "vesafb:%s-%s%s", 	ui->vesa_res->mvalue()->user_data() ?
					    (char*)ui->vesa_res->mvalue()->user_data() :
					    ui->vesa_custom->value(),
					(char*)ui->vesa_depth->mvalue()->user_data(),
					ui->vesa_res->user_data() ?
					    (char*)ui->vesa_res->user_data() : 
					    "");

        config_setvar(config, "video", buf);

        config_write(config, PATH_BASEISO "/boot/isolinux.cfg");
        config_destroy(config);
    }

    return 1;
}
