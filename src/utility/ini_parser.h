
#ifndef INIPARSER_CONF_H
#define INIPARSER_CONF_H

#define MAX_SECTION 50
#define MAX_NAME 50
#define  MAX_VALUE 200



typedef struct _cfg_param _cfg_param;

struct _cfg_param{

    char name[MAX_NAME];
    char value[MAX_VALUE];
    char sections[MAX_SECTION];
};

typedef struct {
    _cfg_param *cfg;
    int lcount;
} cfg_param_t;


#include <stdio.h>


typedef int (*ini_handler)(cfg_param_t *);
typedef char* (*ini_reader)(char* str, int num, void* stream);
int ini_parse(const char* filename, cfg_param_t *);
int ini_parse_file(FILE* file, cfg_param_t *);
int ini_parse_stream(ini_reader reader, void* stream, _cfg_param *, int *);
int get_param_value(cfg_param_t *, char *, char **);
int get_int(cfg_param_t *cfg, char *target, int *temp);
int get_sections(cfg_param_t *cfg, char *target, char *buf);

int get_value_str(cfg_param_t *cfg, char *section, char *key, char **result);
int get_value_init(cfg_param_t *cfg, char *section, char *key, int *temp);


#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 0
#endif


#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif


#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 0
#endif
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";"
#define INI_INLINE_COMMENT_PREFIXES_HASH "#"
#endif


#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif


#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif


#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

#endif //INIPARSER_INI_H



