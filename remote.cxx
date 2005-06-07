#include "generatorUI.h"

#include "config.h"
#include "remote.h"
#include "utils.h"

#include <sys/types.h>
#include <dirent.h> /* opendir */
#include <string.h> /* strcmp, strncmp, strlen */

#include <FL/fl_ask.H> /* fl_alert */

int init_remote_tab(GeneratorUI *ui)
{
    DIR *dirp;
    struct dirent *dp;
    const char *fname;
    const Fl_Menu_Item *m;

    dirp = opendir("lirc");
    if (dirp)
    {
	while ((dp = readdir(dirp)) != NULL)
        {
	    fname = dp->d_name;
	    if (!strncmp("lircd_", fname, 6) &&
		    strcmp(".conf", &fname[strlen(fname)-5]))
		ui->lirc_receiver->add(&fname[6]);
	    else if (!strncmp("lircrc_", fname, 7))
		ui->lirc_remote->add(&fname[7]);
	}
	closedir(dirp);
    }

    if (ui->lirc_receiver->size() < 1 || ui->lirc_remote->size() < 1) {
	fl_alert("Missing remote/receiver configuration files.\n");
	return 0;
    }

    if ((m = ui->lirc_receiver->find_item("atiusb")))
	ui->lirc_receiver->value(m);
    else
	ui->lirc_receiver->value(0);

    if ((m = ui->lirc_remote->find_item("atiusb")))
	ui->lirc_remote->value(m);
    else
	ui->lirc_remote->value(0);

    return 1;
}

int copy_remote_files(GeneratorUI *ui)
{
    char buf[256], buf2[256];
    const char *remote, *receiver;

    if (!ui->lirc_remote->mvalue()) {
	fl_alert("Please pick remote controler.\n");
	return 0;
    }

    if (!ui->lirc_receiver->mvalue()) {
	fl_alert("Please pick remote receiver.\n");
	return 0;
    }

    remote = ui->lirc_remote->mvalue()->label();
    receiver = ui->lirc_receiver->mvalue()->label();

    sprintf(buf, "lirc/lircrc_%s", remote);
    sprintf(buf2, PATH_BASEISO "/etc/lircrc");
    copy_file(buf, buf2);

    sprintf(buf, "lirc/lircd_%s", receiver);
    sprintf(buf2, PATH_BASEISO "/etc/lircd");
    copy_file(buf, buf2);

    sprintf(buf, "lirc/lircd_%s.conf", remote);
    sprintf(buf2, PATH_BASEISO "/etc/lircd.conf");
    copy_file(buf, buf2);

    return 1;
}
