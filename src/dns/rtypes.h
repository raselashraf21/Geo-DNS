//
// Get from https://github.com/pflarr/dns_parse
//

#ifndef GEODNS_RTYPES_H
#define GEODNS_RTYPES_H

#include <stdint.h>

typedef void *rr_data_parser(const uint8_t *, uint32_t, uint32_t,
                             uint16_t, uint32_t);

typedef struct {
    uint16_t cls;
    uint16_t rtype;
    rr_data_parser *parser;
    const char *name;
    const char *doc;
    unsigned long long count;
} rr_parser_container;

rr_parser_container *find_parser(uint16_t, uint16_t);

char *read_dns_name(uint8_t *, uint32_t, uint32_t);

rr_data_parser opts;
rr_data_parser escape;

// Add them to the list of rdata parsers in rtypes.c.
extern rr_parser_container rr_parsers[];

// This is for handling rr's with errors or an unhandled rtype.
rr_parser_container default_rr_parser;

void print_parsers();

void print_parser_usage();

#endif // GEODNS_RTYPES_H
