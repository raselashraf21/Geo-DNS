
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ini_parser.h"

static char *rstrip(char *s) {
    char *p = s + strlen(s);
    while (p > s && isspace((unsigned char) (*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char *lskip(const char *s) {
    while (*s && isspace((unsigned char) (*s)))
        s++;
    return (char *) s;
}

static char *find_chars_or_comment(const char *s, const char *chars) {

#if INI_ALLOW_INLINE_COMMENTS
    int was_space = 0;
    while (*s && (!chars || !strchr(chars, *s)) &&
           !(was_space && (strchr(INI_INLINE_COMMENT_PREFIXES, *s) || strchr(INI_INLINE_COMMENT_PREFIXES_HASH, *s)))) {
        was_space = isspace((unsigned char) (*s));
        s++;

    }
#else
    while (*s && (!chars || !strchr(chars, *s))) {
        s++;
    }
#endif
    return (char *) s;
}

int get_int(cfg_param_t *cfg, char *target, int *temp) {

    int indx = 0;
    int ret = -1;

    while (indx < cfg->lcount) {

        if (!strcmp(cfg->cfg[indx].name, target)) {
            *temp = atoi(cfg->cfg[indx].value);
            ret = 0;
            break;
        }
        ++indx;
    }

    return ret;
}

int get_param_value(cfg_param_t *cfg, char *target, char **buf) {

    int indx = 0;
    int ret = -1;

    while (indx < cfg->lcount) {

        if (!strcmp(cfg->cfg[indx].name, target)) {
            *buf = malloc(sizeof(char) * INI_MAX_LINE);
            sprintf(*buf, "%s", cfg->cfg[indx].value);
            ret = 0;
            break;
        }
        ++indx;
    }

    return ret;
}

int get_sections(cfg_param_t *cfg, char *target, char *buf) {

    int indx = 0;
    int ret = -1;
    memset(buf, 0, INI_MAX_LINE);

    while (indx < cfg->lcount) {

        if (!strcmp(cfg->cfg[indx].sections, target)) {

            strcat(buf, cfg->cfg[indx].value);
            strcat(buf, " ");
            ret = 0;
        }
        ++indx;
    }

    return ret;
}

int ini_parse_stream(ini_reader reader, void *stream, _cfg_param *cfg, int *lcount) {

    char line[INI_MAX_LINE];
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char *start;
    char *end;
    char *name;
    char *value;
    int lineno = 0;
    int error = 0;
    int count = 0;

    while (fgets(line, INI_MAX_LINE, stream) != NULL) {
        lineno++;
        start = line;
#if INI_ALLOW_BOM
        if (lineno == 1 && (unsigned char) start[0] == 0xEF &&
            (unsigned char) start[1] == 0xBB &&
            (unsigned char) start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = lskip(rstrip(start));

        if (*start == '\0' || *start == ';' || *start == '#') {
            continue;
        }
#if INI_ALLOW_MULTILINE
            else if (*prev_name && *start && start > line) {
              if (!handler(cfg))
                error = lineno;
        }
#endif
        else if (*start == '[') {

            end = find_chars_or_comment(start + 1, "]");
            if (*end == ']') {
                *end = '\0';
                strncpy(section, start + 1, sizeof(section));

                *prev_name = '\0';
            }
            else if (!error) {
                error = lineno;
            }
        }
        if (*start) {
            end = find_chars_or_comment(start, "=:");
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
#if INI_ALLOW_INLINE_COMMENTS
                end = find_chars_or_comment(value, NULL);
                if (*end)
                    *end = '\0';
#endif
                rstrip(value);
                strncpy(prev_name, name, sizeof(prev_name));

                strncpy(cfg[count].name, name, MAX_NAME);
                strncpy(cfg[count].sections, section, MAX_SECTION);
                strncpy(cfg[count].value, value, MAX_VALUE);
                ++count;
            }
        }
        else if (!error) {
            error = lineno;
        }

    }
    *lcount = count;
    return error;
}

int ini_parse_file(FILE *file, cfg_param_t *cfg_t) {

    _cfg_param *cfg = (_cfg_param *) malloc(sizeof(_cfg_param) * INI_MAX_LINE);
    int line_count;
    int error = ini_parse_stream((ini_reader) fgets, file, cfg, &line_count);

    cfg_t->cfg = cfg;
    cfg_t->lcount = line_count;
    return error;
}

int ini_parse(const char *filename, cfg_param_t *cfg_t) {
    FILE *file;
    int error;

    file = fopen(filename, "r");
    if (!file) {
        return -1;
    }

    error = ini_parse_file(file, cfg_t);
    fclose(file);
    return error;
}
