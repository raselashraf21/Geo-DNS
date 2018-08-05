
#ifndef GEODNS_DNS_RESPONSE_H
#define GEODNS_DNS_RESPONSE_H

#include "dns/dns_packet.h"
#include "dns/dns_app.h"

int lookup_zone(char *qname);
int recursive_call(char *qname);
int get_dns_response(dns_info *const query, dns_ctx *app_ctx, common_ctx *var_ctx);

#endif // GEODNS_DNS_RESPONSE_H
