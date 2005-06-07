#include "generatorUI.h"

#include "audio.h"
#include "config.h"
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
