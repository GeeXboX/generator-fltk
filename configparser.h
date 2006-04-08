#ifndef configparser_h
#define configparser_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct config config_t;

config_t *config_open(const char *filename, int shell_escape);
int config_write(config_t *config, const char *filename);
void config_destroy(config_t *config);

int config_getvar (config_t *config, const char *var, char *dst, size_t dstlen);
int config_setvar (config_t *config, const char *var, const char *value);
int config_setvar_int (config_t *config, const char *var, int value);

#ifdef __cplusplus
}
#endif

#endif
