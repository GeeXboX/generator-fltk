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

#include <FL/fl_ask.H> /* fl_alert */

int init_audio_tab(GeneratorUI *ui)
{
    char buf[256];
    int i;
    unsigned long k;
    config_t *config = config_open(PATH_BASEISO "/etc/audio", 1);

    const char *ac97_spsa[] = {
	"PCM1",
	"PCM2, PCM1 (rear)",
	"Centre and LFE",
	"PCM3, Modem, Dedicated SPDIF"
    };

    if (!config) {
	fl_alert("Missing audio configuration files.\n");
	return 0;
    }

    for (k = 0; k < sizeof(ac97_spsa)/sizeof(ac97_spsa[0]); k++) {
	ui->ac97_spsa->add(ac97_spsa[k], 0, NULL, (void *)ac97_spsa[k]);
    }

    config_getvar_int(config, "ALSA_CARD", &i);
    ui->alsacard_id->value(i);

    config_getvar(config, "SOUNDCARD_MODE", buf, sizeof(buf));
    ui->soundcard_mode->value(my_strcasecmp(buf, "spdif") ?
                              GeneratorUI::SOUNDCARD_MODE_ANALOG :
                              GeneratorUI::SOUNDCARD_MODE_SPDIF);

    config_getvar(config, "SPDIF_PT_MODE", buf, sizeof(buf));
    ui->hwac3->value(!my_strcasecmp(buf, "none") ? GeneratorUI::HW_NONE :
                     !my_strcasecmp(buf, "ac3") ? GeneratorUI::HW_AC3 :
                     !my_strcasecmp(buf, "dts") ? GeneratorUI::HW_DTS :
                     GeneratorUI::HW_AC3DTS);

    config_getvar_int(config, "CHANNELS", &i);
    ui->channels->value(i == 6 ? GeneratorUI::CHANNELS_6 :
                        i == 4 ? GeneratorUI::CHANNELS_4 :
                                 GeneratorUI::CHANNELS_2);

    config_getvar(config, "SBL_AUDIGY", buf, sizeof(buf));
    ui->sbl_audigy->value(!my_strcasecmp(buf, "0"));

    config_getvar_int(config, "AC97_SPSA", &i);
    ui->ac97_spsa->value(i > 3 || i < 0 ? 0 : i);

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
    config_setvar(config, "SPDIF_PT_MODE",
                  ui->hwac3->value() == GeneratorUI::HW_NONE ? "none" :
                  ui->hwac3->value() == GeneratorUI::HW_AC3 ? "ac3" :
                  ui->hwac3->value() == GeneratorUI::HW_DTS ? "dts" :
                  "ac3dts");

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

    config_setvar_int(config, "SBL_AUDIGY", ui->sbl_audigy->value() ? 0 : 1);
    config_setvar_int(config, "AC97_SPSA", (int)ui->ac97_spsa->value());

    config_write(config, PATH_BASEISO "/etc/audio");
    config_destroy(config);

    return 1;
}
