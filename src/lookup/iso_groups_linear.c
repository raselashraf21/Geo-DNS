
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "config.h"
#include "iso_groups.h"
#include "utility/new.h"
#include "utility/log/log.h"

#define ISO_MAX 702

typedef struct _iso_groups_ctx {
    uint32_t **iso_header;
    char *iso_path;
    uint16_t iso_grp_count;
} iso_groups_ctx;

static void _init_iso_groups(iso_groups_ctx *ctx);

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

    uint32_t **header = self->iso_header;

    if (header) {
        for (int i = 0; i < ISO_MAX; ++i) {
            if (header[i])
                free(header[i]);
        }
        free(header);
        header = NULL;
    }
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

    uint32_t **iso_header = calloc(ISO_MAX, sizeof(uint32_t *));

    FILE *file_pointer = fopen(ctx->iso_path, "r");
    if(file_pointer == NULL) {
        LM_PRINT(L_ALL, "error opening file\n");
        exit(-1);               // TODO: work here
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
        exit(-1);               // TODO
    }

    uint32_t *group_id;
    while (ccount > 0) {
        sptr = NULL;
        iso = strtok_r(line, delim, &sptr);

        if (isalpha(iso[0]) && isalpha(iso[1])) {

            group_id = malloc(sizeof(uint32_t) * gcount);
            _DEBUG_CHANGE printf("%s ", iso);
            cvalue = (iso[0] - 'A') * 26 + (iso[1] - 'A');

            iso = strtok_r(NULL, delim, &sptr);
            int i = 0;
            while (iso && i < gcount) {

                group_id[i] = atoi(iso);
                _DEBUG_CHANGE printf("%d ", group_id[i]);
                i++;
                iso = strtok_r(NULL, delim, &sptr);
            }

            _DEBUG_CHANGE printf("\n");
            iso_header[cvalue] = group_id;
        }
        ccount = getline(&line, &dummy_size, file_pointer);
    }
    free(line);

    ctx->iso_header = iso_header;
    ctx->iso_grp_count = gcount;

    fclose(file_pointer);
}

int get_iso_host_group(int *host_groups, int hgroups_count, char *_country_iso, void *ctx) {
    int cvalue = (_country_iso[0] - 'A') * 26 + (_country_iso[1] - 'A');
    iso_groups_ctx *iso_ctx = ctx;
    int cgroups_count = iso_ctx->iso_grp_count;
    uint32_t *country_groups = iso_ctx->iso_header[cvalue];

    int i;
    for(i = 0; i < hgroups_count; i++){
        for(int j=0; j<cgroups_count; j++){
            if(host_groups[i] == country_groups[j]){
                i++;
                goto exit;
            }
        }
    }

exit:
    return host_groups[i-1];  // TODO: default to last group
}
