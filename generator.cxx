#include "generatorUI.h"

#include "audio.h"
#include "config.h"
#include "language.h"
#include "network.h"
#include "remote.h"
#include "theme.h"
#include "utils.h"

#include <stdlib.h> /* exit */

void update_tabs_status(GeneratorUI *ui)
{
    if (ui->override_sub_font->value())
	ui->sub_font->activate();
    else
	ui->sub_font->deactivate();

    if (ui->override_menu_font->value())
	ui->menu_font->activate();
    else
	ui->menu_font->deactivate();

    switch (ui->soundcard_mode->value())
    {
    case GeneratorUI::SOUNDCARD_MODE_SPDIF:
	ui->hwac3->activate();
	if (ui->hwac3->value())
	    ui->channels->deactivate();
	else
	    ui->channels->activate();
	break;
    case GeneratorUI::SOUNDCARD_MODE_ANALOG:
	ui->hwac3->deactivate();
	ui->channels->activate();
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
}

void generator_exit(GeneratorUI *ui)
{
    exit(0);
}

#include <FL/fl_ask.H>

static int init_tabs(GeneratorUI *ui)
{
    return init_language_tab(ui)
	&& init_audio_tab(ui)
	&& init_remote_tab(ui)
	&& init_network_tab(ui)
	&& init_theme_tab(ui);
}

int
main(int argc, char **argv) 
{
    GeneratorUI *ui = new GeneratorUI;

    Fl::scheme("plastic");

    if (!init_tabs(ui)) {
	fprintf(stderr, "Tabs initilizing failed.\n");
	return 1;
    }

    update_tabs_status(ui);

    ui->show(argc, argv);

    // take focus
    ui->interface_tab->show();

    return Fl::run();
}
