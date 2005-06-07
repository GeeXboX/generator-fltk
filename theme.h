#ifndef theme_h
#define theme_h

int init_theme_tab(GeneratorUI *ui);
char *valid_theme_font(const char *theme_name, struct charset_info *c);
int copy_theme_files(GeneratorUI *ui);
int append_bootplash_to_initrd(GeneratorUI *ui);

#endif
