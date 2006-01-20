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
#include "system.h"
#include "utils.h"

#include <stdlib.h> /* atoi */

#include <FL/fl_ask.H> /* fl_alert */

int init_audio_tab(GeneratorUI *ui)
{
    char buf[256];
    int i;
    FILE *f;

    f = fopen(PATH_BASEISO "/etc/audio", "r");
    if (!f) {
	fl_alert("Missing audio configuration files.\n");
	return 0;
    }

    get_shvar_value(f, "ALSA_CARD", buf);
    ui->alsacard_id->value(atoi(buf));

    get_shvar_value(f, "SOUNDCARD_MODE", buf);
    ui->soundcard_mode->value(my_strcasecmp(buf, "spdif") ? 
				GeneratorUI::SOUNDCARD_MODE_ANALOG :
				GeneratorUI::SOUNDCARD_MODE_SPDIF);

    get_shvar_value(f, "AC3_DECODER", buf);
    ui->hwac3->value(!my_strcasecmp(buf, "hardware"));

    get_shvar_value(f, "CHANNELS", buf);
    i = atoi(buf);
    ui->channels->value(i == 6 ? GeneratorUI::CHANNELS_6 :
                        i == 4 ? GeneratorUI::CHANNELS_4 :
                                 GeneratorUI::CHANNELS_2);

    fclose(f);

    return 1;
}

int write_audio_settings(GeneratorUI *ui)
{
    FILE *fp;
    int i = 0;

    fp = fopen(PATH_BASEISO "/etc/audio", "wb");
    if (!fp) {
	fl_alert("Failed to write audio configuration.\n");
	return 0;
    }

    fprintf(fp, "ALSA_CARD=\"%d\"\n", (int)ui->alsacard_id->value());
    fprintf(fp, "SOUNDCARD_MODE=\"%s\"\n",
		(ui->soundcard_mode->value() == GeneratorUI::SOUNDCARD_MODE_SPDIF)
		    ? "SPDIF" : "analog");
    fprintf(fp, "AC3_DECODER=\"%s\"\n",
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
    fprintf(fp, "CHANNELS=\"%d\"\n", i); 

    fclose(fp);

    return 1;
}
