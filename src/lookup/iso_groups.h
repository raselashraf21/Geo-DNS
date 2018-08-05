
#ifndef GEODNS_ISO_GROUP_H
#define GEODNS_ISO_GROUP_H

#include <stdint.h>


/*!
 * USAGE:
 *          void *ctx = new(IsoGroupsCtx, NULL);
 *          ...
 *          ...
 *          delete(ctx)
 *
 */

extern const void *IsoGroupsCtx;

int get_iso_host_group(int *host_groups, int hgroups_count, char *_country_iso, void *ctx);

#endif // GEODNS_ISO_GROUP_H
