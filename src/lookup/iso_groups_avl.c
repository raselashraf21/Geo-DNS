
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "config.h"
#include "iso_groups.h"
#include "utility/avl.h"
#include "utility/new.h"
#include "utility/log/log.h"

#ifndef EXIT_CODE_ISO1
    #define EXIT_CODE_ISO1  102
#endif
#ifndef EXIT_CODE_ISO2
    #define EXIT_CODE_ISO2  103
#endif

#define ISO_MAX 702

typedef struct _iso_groups_ctx {
    const void * class;             /* Must be first, must not use it */
    char *iso_path;
    avl_tree_node **iso_header;
    uint16_t iso_grp_count;
} iso_groups_ctx;

static void _init_iso_groups(iso_groups_ctx *ctx);
static void _deinit_iso_groups(iso_groups_ctx *ctx);

/********** Object Implementation for type IsoGroupsCtx(iso_groups_ctx) **********/

static void *iso_ctx_ctor (void * _self, size_t argc, va_list *argv)
{
    iso_groups_ctx *self = _self;
    if (argc > 0) {
        const char *text = va_arg(*argv, const char *);
        self->iso_path = strdup(text);
    }

    _init_iso_groups(self);

    return self;
}

static void *iso_ctx_dtor (void * _self)
{
    iso_groups_ctx *self = _self;
    if (self->iso_path) {
        free(self->iso_path);
    }

    _deinit_iso_groups(self);

    return self;
}

static const struct Class _IsoGroupsCtx = {
        sizeof(iso_groups_ctx),
        iso_ctx_ctor,
        iso_ctx_dtor
};
const void *IsoGroupsCtx = &_IsoGroupsCtx;
/****************************************************************************/

static void _init_iso_groups(iso_groups_ctx *ctx) {
    avl_tree_node **iso_header = calloc(ISO_MAX, sizeof(avl_tree_node *));

    FILE *file_pointer = fopen(ctx->iso_path, "r");
    if(file_pointer == NULL) {
        LM_PRINT(L_ALL, "[ERROR] Cannot open country-iso (%s) file.\n", ctx->iso_path);
        exit(EXIT_CODE_ISO1);
    }

    size_t dummy_size = 6;
    char *line = malloc(sizeof(char) * dummy_size);
    ssize_t ccount = getline(&line, &dummy_size, file_pointer);
    char *delim = ".";
    int cvalue;
    char *sptr;
    char *iso;

    uint16_t gcount = 0;
    if (ccount > 0) {
        char *dline = strdup(line);
        iso = strtok_r(dline, delim, &sptr);
        iso = strtok_r(NULL, delim, &sptr);
        while (iso) {
            gcount++;
            iso = strtok_r(NULL, delim, &sptr);
        }
        free(dline);
    }
    else {
        LM_PRINT(L_ALL, "[ERROR] Empty country-iso file.\n");
        exit(EXIT_CODE_ISO2);
    }

    while (ccount > 0) {
        sptr = NULL;
        iso = strtok_r(line, delim, &sptr);

        if (isalpha(iso[0]) && isalpha(iso[1])) {
            avl_tree_node *avl = NULL;

            cvalue = (iso[0] - 'A') * 26 + (iso[1] - 'A');

            iso = strtok_r(NULL, delim, &sptr);
            int i = 0;
            while (iso && i < gcount) {
                insert_value_in_avl(atoi(iso), &avl);

                i++;
                iso = strtok_r(NULL, delim, &sptr);
            }

            iso_header[cvalue] = avl;
        }
        ccount = getline(&line, &dummy_size, file_pointer);
    }
    free(line);

    ctx->iso_header = iso_header;
    ctx->iso_grp_count = gcount;

    fclose(file_pointer);
    LM_PRINT(L_ALL, "Successfully loaded iso-group in memory\n");
}

static void _deinit_iso_groups(iso_groups_ctx *ctx) {
    int count = ISO_MAX - 1;
    avl_tree_node **avl_array = ctx->iso_header;

    if (avl_array) {
        while (count >= 0) {
            delete_avl_node(&avl_array[count]);
            count--;
        }
    }
    ctx->iso_header = NULL;
}

int get_iso_host_group(int *host_groups, int hgroups_count, char *_country_iso, void *ctx) {
    int cvalue = (_country_iso[0] - 'A') * 26 + (_country_iso[1] - 'A');
    avl_tree_node **header = ((iso_groups_ctx *)ctx)->iso_header;

    for(int i = 0; i < hgroups_count; i++){
        avl_tree_node *found = search_from_avl(host_groups[i], header[cvalue]);
        if (found) {
            return found->value;
        }
    }

    return host_groups[0];  // TODO: default to first group
}
