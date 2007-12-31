/*
 *  External packages support for GeeXboX FLTK Generator
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

#include "compile.h"
#include "config.h"
#include "curl.h"
#include "fs.h"
#include "generator.h"
#include "packages.h"
#include "utils.h"

#include <FL/fl_ask.H> /* fl_alert */
#include <FL/Fl_File_Chooser.H> /* fl_file_chooser fl_dir_chooser */

#include <vector>
#include <string>

typedef struct {
    char *name;
    std::string desc;
    std::vector<char*> file;
    std::vector<char*> md5;
    std::vector<char*> rename;
    char *dir;
    char *license;
    int agree;
} Package;

static int g_compile;

static void insert_package_node(Flu_Tree_Browser *tree, const char *path, Package *p)
{
    Flu_Tree_Browser::Node *n;
    size_t len;

    n = tree->add(path);
    n->swap_label_and_widget(true);
    n->auto_label_color(true);
    n->user_data(p);

    len = strlen(path);
    if (path[len-1] != '/') {
        n->widget(new Fl_Check_Button(0, 0, 20, 20));
    } else if (strchr(path, '/') == &path[len-1]) {
	n->open(true);
    }
}

int read_packages_file(Flu_Tree_Browser *tree, const char *fname)
{
    char buf[1024], path[256];
    char *value;
    Package *p = NULL;
    FILE *f;
    size_t len;
    int package_target_me = 2;

    f = fopen(fname, "r");
    if (!f)
	return 0;

    while (fgets(buf, sizeof(buf), f))
    {
	len = strlen(buf);
	while (len && isspace(buf[len-1]))
	    buf[--len] = '\0';

	if (buf[0] == '[' && buf[len-1] == ']') {
	    buf[len-1] = '\0';
	    if (p && package_target_me)
		insert_package_node(tree, path, p);
	    p = new Package;
	    strcpy(path, &buf[1]);

	    p->name = "";
	    p->desc = "";
	    p->file.clear();
	    p->md5.clear();
	    p->dir = NULL;
	    p->license = NULL;
	    p->agree = 0;
	    package_target_me = 2;
	} else if (p && (value = strchr(buf, '='))) {
	    *value++ = '\0';
	    if (!strcmp(buf, "name")) {
		p->name = strdup(value);
	    } else if (!strcmp(buf, "desc")) {
		if (!p->desc.empty())
		    p->desc += '\n';
		p->desc += value;
	    } else if (!strcmp(buf, "file")) {
		p->file.push_back(strdup(value));
	    } else if (!strcmp(buf, "md5")) {
		p->md5.push_back(strdup(value));
	    } else if (!strcmp(buf, "rename")) {
		p->rename.push_back(strdup(value));
	    } else if (!strcmp(buf, "dir")) {
		p->dir = strdup(value);
	    } else if (!strcmp(buf, "license")) {
		p->license = strdup(value);
	    } else if (!strcmp(buf, "target")) {
		if (package_target_me != 1)
		    package_target_me = (strcmp(value, get_target_arch_string()) == 0);
	    }
	}
    }

    if (p && package_target_me)
	insert_package_node(tree, path, p);

    fclose(f);
    return 1;
}

static void tree_callback(Fl_Widget* w, void* data)
{
    GeneratorUI *ui;
    Flu_Tree_Browser::Node *n;
    Flu_Tree_Browser *tree;
    Package *p;

    ui = (GeneratorUI *)data;
    tree = (Flu_Tree_Browser*)w;
    n = tree->callback_node();
    p = (Package*) n->user_data();

    switch (tree->callback_reason())
    {
    case FLU_SELECTED:
	if (ui->package_name->value() == p->name) {
	    Fl_Button *b = (Fl_Button*)n->widget();
	    if (b)
		b->value(!b->value());
	}
	ui->package_name->static_value(p->name);
	ui->package_desc->static_value(p->desc.c_str());
	break;
    }
}

void init_nodes(GeneratorUI *ui)
{
    Flu_Tree_Browser::Node *n;

    for (n = ui->package_tree->first_leaf(); n; n = n->next_leaf())
    {
        if (is_package_downloaded(n))
        {
            n->clear();
            n->deactivate();
        }
    }
}

int init_packages_tab(GeneratorUI *ui)
{
    Flu_Tree_Browser *tree = ui->package_tree;

    tree->get_root()->always_open(true);
    tree->when(FL_WHEN_NOT_CHANGED);
    tree->show_root(false);
    tree->animate(false);
    tree->selection_mode(FLU_SINGLE_SELECT);
    tree->insertion_mode(FLU_INSERT_SORTED);
    tree->callback(tree_callback, ui);

    read_packages_file(tree, "packages.ini");
    init_nodes(ui);

    return 1;
}

static void find_url(Flu_Tree_Browser::Node *n, const char *file, std::string &url)
{
    std::string tmp;
    Package *p;

    if (strstr(file, "://")) {
	url = file;
	return;
    }

    url = "";
    for (n = n->parent(); n; n = n->parent())
    {
	p = (Package*)n->user_data();
	if (p && p->file.size() > 0)
	{
	    tmp = p->file[0];
	    url = tmp + url;
	    if (strstr(p->file[0], "://"))
		break;
	}
    }
    url += file;
}

static void find_path(Flu_Tree_Browser::Node *n, const char *file, std::string &path)
{
    const char *filename;
    Package *p;

    for (; n; n = n->parent())
    {
	p = (Package*)n->user_data();
	if (p && p->dir)
	{
	    path = p->dir;
	    break;
	}
    }

    if ((filename = strrchr(file, '/')))
	filename++;
    else
	filename = file;

    path += filename;
}

static void start_package_downloading(GeneratorUI *ui)
{
    std::vector<char*>::const_iterator fii, mii, rii;
    Flu_Tree_Browser::Node *n;
    Fl_Button *b;
    Package *p;
    std::string url, path, label;
    int rc;
    int pp_data = 0;

    rc = 0;
    for (n = ui->package_tree->first_leaf(); n; n = n->next_leaf())
    {
	p = (Package*)n->user_data();
	b = (Fl_Button*)n->widget();
	if (b && b->value() && (!p->license || p->agree))
	    rc+=p->file.size();
    }
    if (!rc && g_compile) {
        /* only compile the iso */
        compile_iso(ui, 0);
        g_compile = 0;
        return;
    }
    else if (!rc)
	return;

    ui->setting_tabs->value(ui->packages_tab);
    ui->packages_tabs->value(ui->packages_subtab);

    ui->package_progress->minimum(0);
    ui->package_progress->maximum(rc*100);
    ui->package_progress->value(0);
    ui->package_progress->user_data(&pp_data);
    ui->package_progress->label("");

    ui->package_tree->deactivate();
    ui->package_button->label("Stop");
    ui->compile_button->deactivate();

    rc = 0;
    for (n = ui->package_tree->first_leaf(); !rc && n; n = n->next_leaf())
    {
	p = (Package*)n->user_data();
	b = (Fl_Button*)n->widget();
	if (b && b->value() && (!p->license || p->agree) && p->file.size() > 0)
	{
	    ui->package_progress->label(p->name);
	    for(fii=p->file.begin() , mii=p->md5.begin() , rii=p->rename.begin();
		fii!=p->file.end(); fii++)
	    {
		find_url(n, *fii, url);

		const char *filename = (rii != p->rename.end()) ? *rii++ : *fii;
	        find_path(n, filename, path);

		Fl::check();

		const char *md5 = (mii != p->md5.end()) ? *mii++ : "";

		rc = download_file(ui->package_button, ui->package_progress, url.c_str(), (char*)path.c_str(), md5);
		if (!rc)
		{
		    int t = *((int*) ui->package_progress->user_data()) + 100;
		    ui->package_progress->value(t);
    	    	    ui->package_progress->user_data(&t);
		} else
		    break;
	    }
	    if (!rc)
            {
                n->clear();
                n->deactivate();
            }
	}
    }

    if (rc) {
        ui->package_progress->value(0);
	switch (rc)
	{
	case 1:
	    fl_alert("Had download errors in %s\n", url.c_str());
	    break;
	case 2:
	    fl_alert("Couldn't open output file %s\n", path.c_str());
	    break;
	case 3:
	    fl_alert("Wrong MD5 sum for %s\n", url.c_str());
	    break;
	case 4:
	    fl_alert("Failed to decompress %s\n", url.c_str());
	    break;
	}
	ui->package_progress->label("");
    } else {
	ui->package_progress->label("Done");
        if (g_compile) {
            /* only compile */
            compile_iso(ui, 0);
            g_compile = 0;
        }
    }

    ui->package_button->label("Download");
    ui->package_tree->activate();
    ui->compile_button->activate();

    update_tabs_status(ui);
}

static size_t display_license(char *buf, size_t size, size_t nmemb, void *data)
{
    GeneratorUI *ui = (GeneratorUI*)data;
    std::string *buffer;

    buffer = (std::string*) ui->license_text->user_data();
    buffer->append(buf, size*nmemb);

    ui->license_text->value(buffer->c_str());

    return size*nmemb;
}

static void process_package_license(GeneratorUI *ui, Flu_Tree_Browser::Node *n)
{
    Fl_Button *b;
    Package *p;
    std::string url, buffer;

    while (n)
    {
	p = (Package*)n->user_data();
	b = (Fl_Button*)n->widget();
	if (b && b->value() && p->license && !p->agree)
	    break;
	n = n->next_leaf();
    }

    if (!n) {
	ui->license_window->hide();
	start_package_downloading(ui);
	return;
    }

    p = (Package*)n->user_data();

    ui->license_name->user_data(n);
    ui->license_name->static_value(p->name);

    find_url(n, p->license, url);

    buffer = "";
    ui->license_text->user_data(&buffer);

    ui->license_agree_button->deactivate();
    ui->license_disagree_button->label("Stop");
    ui->license_text->static_value("");
    if (!download_progress(ui->license_disagree_button, ui->license_progress, url.c_str(), display_license, ui)) {
	ui->license_disagree_button->label("I disagree");
	ui->license_agree_button->label("I agree");
	ui->license_agree_button->activate();
    } else {
	ui->license_text->static_value("Failed to receive license file.");
	ui->license_disagree_button->label("Next");
    }

    static int c;
    c = *((int*)ui->license_progress->user_data()) + 100;

    ui->license_progress->value(c);
    ui->license_progress->user_data(&c);
}

void package_license_agree(GeneratorUI *ui, int agree)
{
    Flu_Tree_Browser::Node *n;
    Package *p;

    n = (Flu_Tree_Browser::Node *) ui->license_name->user_data();

    p = (Package*)n->user_data();
    p->agree = agree;

    process_package_license(ui, n->next_leaf());
}

void open_licenses_window(GeneratorUI *ui, int count)
{
    Flu_Tree_Browser::Node *n;
    int lp_data = 0;

    if (!ui->license_window) ui->make_license_window();

    ui->license_progress->minimum(0);
    ui->license_progress->maximum(count*100);
    ui->license_progress->value(0);
    ui->license_progress->user_data(&lp_data);

    ui->license_window->show();

    n = ui->package_tree->first_leaf();

    process_package_license(ui, n);
}

void package_download(GeneratorUI *ui, int run_compile)
{
    Flu_Tree_Browser::Node *n;
    Fl_Button *b;
    Package *p;
    int count;

    g_compile = run_compile;

    count = 0;
    for (n = ui->package_tree->first_leaf(); n; n = n->next_leaf())
    {
	p = (Package*)n->user_data();
	b = (Fl_Button*)n->widget();
	if (b && b->value() && p->license && !p->agree)
	    count++;
    }

    if (count)
	open_licenses_window(ui, count);
    else
	start_package_downloading(ui);
}

int is_package_downloaded(Flu_Tree_Browser::Node *n)
{
    std::vector<char*>::const_iterator fii, rii;
    std::string path;
    Package *p;

    p = (Package*)n->user_data();
    for(fii=p->file.begin(), rii=p->rename.begin(); fii!=p->file.end(); fii++)
    {
        const char *filename = (rii != p->rename.end()) ? *rii++ : *fii;
        find_path(n, filename, path);
        if (path.find(".bz2", path.length()-4) != std::string::npos)
            path.resize(path.length()-4);
        if (!file_exists(path.c_str()))
            return 0;
    }
    return 1;
}
