#ifndef language_h
#define language_h

struct charset_info {
    char *name;
    char *codename;
    char *menu_font;
    char *sub_font;
};

struct lang_info {
    char *shortname;
    struct charset_info *c;
};

void change_font(Fl_Output *o, Fl_Check_Button *b);
int init_language_tab(GeneratorUI *ui);
int copy_language_files(GeneratorUI *ui);

#endif /* language_h */
