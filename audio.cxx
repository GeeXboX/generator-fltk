/*
 *  Audio support for GeeXboX FLTK Generator
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
#include "config.h"
#include "configparser.h"
#include "system.h"
#include "utils.h"

#include <stdlib.h> /* atoi */

#include <FL/fl_ask.H> /* fl_alert */

int init_audio_tab(GeneratorUI *ui)
{
    char buf[256];
    int i;
    config_t *config = config_open(PATH_BASEISO "/etc/audio", 1);

    if (!config) {
	fl_alert("Missing audio configuration files.\n");
	return 0;
    }

    config_getvar(config, "ALSA_CARD", buf, sizeof(buf));
    ui->alsacard_id->value(atoi(buf));

    config_getvar(config, "SOUNDCARD_MODE", buf, sizeof(buf));
    ui->soundcard_mode->value(my_strcasecmp(buf, "spdif") ? 
				GeneratorUI::SOUNDCARD_MODE_ANALOG :
				GeneratorUI::SOUNDCARD_MODE_SPDIF);

    config_getvar(config, "AC3_DECODER", buf, sizeof(buf));
    ui->hwac3->value(!my_strcasecmp(buf, "hardware"));

    config_getvar(config, "CHANNELS", buf, sizeof(buf));
    i = atoi(buf);
    ui->channels->value(i == 6 ? GeneratorUI::CHANNELS_6 :
                        i == 4 ? GeneratorUI::CHANNELS_4 :
                                 GeneratorUI::CHANNELS_2);

    config_destroy(config);

    return 1;
}

int write_audio_settings(GeneratorUI *ui)
{
    config_t *config = config_open(PATH_BASEISO "/etc/audio", 1);
    int i = 0;

    if (!config) {
	fl_alert("Failed to write audio configuration.\n");
	return 0;
    }

    config_setvar_int(config, "ALSA_CARD", (int)ui->alsacard_id->value());
    config_setvar(config, "SOUNDCARD_MODE", 
		(ui->soundcard_mode->value() == GeneratorUI::SOUNDCARD_MODE_SPDIF)
		    ? "SPDIF" : "analog");
    config_setvar(config, "AC3_DECODER",
                ui->hwac3->value() ? "hardware" : "software");

    switch (ui->channels->value())
    {
    case GeneratorUI::CHANNELS_2:
        i = 2;
        break;
    case GeneratorUI::CHANNELS_4:
        i = 4;
        break;
    case GeneratorUI::CHANNELS_6:
        i = 6;
        break;
    }
    config_setvar_int(config, "CHANNELS", i);

    config_write(config, PATH_BASEISO "/etc/audio");
    config_destroy(config);

    return 1;
}
