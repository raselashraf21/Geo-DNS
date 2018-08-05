
#ifndef GEODNS_CREATE_ISO_FILE_H
#define GEODNS_CREATE_ISO_FILE_H

#include <sys/types.h>
#include <inttypes.h>
#include <wchar.h>

#define _MAX 10

struct iso_group {
    char *iso;
    __uint8_t *group_id;
    struct iso_group *next;
};

typedef struct iso_group iso_node;

iso_node *head_ptr;
iso_node *counter;
//uint8_t **iso_array;

int create_iso_file();

#endif // GEODNS_CREATE_ISO_FILE_H
