/*
 *  Network support for GeeXboX FLTK Generator
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "generatorUI.h"

#include "config.h"
#include "network.h"
#include "system.h"
#include "utils.h"

#include <FL/fl_ask.H> /* fl_alert */

#define yes_no(x) ((x) ? "yes" : "no") 

int init_network_tab(GeneratorUI *ui)
{
    char buf[256];
    FILE *f;

    f = fopen(PATH_BASEISO "/etc/network", "r");
    if (!f) {
	fl_alert("Missing network configuration files.\n");
	return 0;
    }

    get_shvar_value(f, "PHY_TYPE", buf);
    ui->phy_iface->value(!my_strcasecmp(buf, "wifi") ? 
				GeneratorUI::NETWORK_PHY_IFACE_WIFI :
                         !my_strcasecmp(buf, "ethernet") ?
				GeneratorUI::NETWORK_PHY_IFACE_ETHER :
				GeneratorUI::NETWORK_PHY_IFACE_AUTO);

    get_shvar_value(f, "WIFI_MODE", buf);
    ui->wifi_mode->value(!my_strcasecmp(buf, "ad-hoc") ? 
				GeneratorUI::WIFI_MODE_ADHOC :
				GeneratorUI::WIFI_MODE_MANAGED);

    get_shvar_value(f, "WIFI_WEP", buf);
    ui->wifi_wep->value(buf);

    get_shvar_value(f, "WIFI_ESSID", buf);
    ui->wifi_ssid->value(buf);

    get_shvar_value(f, "GATEWAY", buf);
    ui->network_gateway->value(buf);

    get_shvar_value(f, "DNS_SERVER", buf);
    ui->network_dns->value(buf);

    get_shvar_value(f, "HOST", buf);
    ui->network_ip->value(buf);

    ui->network_conf->value(buf[0] ? GeneratorUI::NETWORK_CONF_MANUAL :
                                     GeneratorUI::NETWORK_CONF_DHCP);

    get_shvar_value(f, "SMB_USER", buf);
    ui->samba_user->value(buf);

    get_shvar_value(f, "SMB_PWD", buf);
    ui->samba_pass->value(buf);

    get_shvar_value(f, "TELNET_SERVER", buf);
    ui->server_telnet->value(!my_strcasecmp(buf, "yes"));

    get_shvar_value(f, "FTP_SERVER", buf);
    ui->server_ftp->value(!my_strcasecmp(buf, "yes"));

    get_shvar_value(f, "HTTP_SERVER", buf);
    ui->server_http->value(!my_strcasecmp(buf, "yes"));

    fclose(f);

    return 1;
}

int write_network_settings(GeneratorUI *ui)
{
    FILE *fp;
    const char *str = NULL;

    fp = fopen(PATH_BASEISO "/etc/network", "wb");
    if (!fp) {
	fl_alert("Failed to write network configuration.\n");
	return 0;
    }

    switch (ui->phy_iface->value())
    {
    case GeneratorUI::NETWORK_PHY_IFACE_AUTO:
        str = "auto";
        break;
    case GeneratorUI::NETWORK_PHY_IFACE_WIFI:
        str = "wifi";
        break;
    case GeneratorUI::NETWORK_PHY_IFACE_ETHER:
        str = "ethernet";
        break;
    }
    fprintf(fp, "PHY_TYPE=\"%s\"\n", str);

    fprintf(fp, "WIFI_MODE=\"%s\"\n",
		(ui->wifi_mode->value() == GeneratorUI::WIFI_MODE_ADHOC)
		    ? "ad-hoc" : "managed");
    fprintf(fp, "WIFI_WEP=\"%s\"\n", ui->wifi_wep->value());
    fprintf(fp, "WIFI_ESSID=\"%s\"\n", ui->wifi_ssid->value());

    fprintf(fp, "HOST=\"%s\"\n", ui->network_ip->value());
    fprintf(fp, "SUBNET=\"%s\"\n", ui->network_subnet->value());
    fprintf(fp, "GATEWAY=\"%s\"\n", ui->network_gateway->value());
    fprintf(fp, "DNS_SERVER=\"%s\"\n", ui->network_dns->value());
    
    fprintf(fp, "SMB_USER=\"%s\"\n", ui->samba_user->value());
    fprintf(fp, "SMB_PWD=\"%s\"\n", ui->samba_pass->value());

    fprintf(fp, "TELNET_SERVER=\"%s\"\n", yes_no(ui->server_telnet->value()));
    fprintf(fp, "FTP_SERVER=\"%s\"\n", yes_no(ui->server_ftp->value()));
    fprintf(fp, "HTTP_SERVER=\"%s\"\n", yes_no(ui->server_http->value()));

    fclose(fp);

    return 1;
}
