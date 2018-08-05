//
// Get from https://github.com/pflarr/dns_parse
//

#include <stdint.h>

#ifndef GEODNS_STR_UTILS_H
#define GEODNS_STR_UTILS_H

// Encodes the rdata into plaintext (minus newlines and delimiters).  Escaped
// characters are in the format \x33 (an ! in this case).  The escaped 
// characters are:
//  All characters < \x20
//  Backslash (\x5c)
//  All characters >= \x7f
// Arguments (packet, start, end):
//  packet - The uint8_t array of the whole packet.
//  start - the position of the first character in the rdata.
//  end - the position + 1 of the last character in the rdata.
char *escape_data(const uint8_t *, uint32_t, uint32_t);

// Read a reservation record style qname, dealing with any compression.
// A newly allocated string of the read qname with length bytes
// converted to periods is placed in the char * argument. 
// If there was an error reading the qname, NULL is returned and the
// position argument is left with it's passed value.
// Args (packet, pos, id_pos, len, qname)
//  packet - The uint8_t array of the whole packet.
//  pos - the start of the rr qname.
//  id_pos - the start of the dns packet (id field)
//  len - the length of the whole packet
//  qname - We will return read qname via this pointer.
char *read_rr_name(const uint8_t *, uint32_t, uint32_t, uint32_t *);

char *fail_name(const uint8_t *, uint32_t, uint32_t, const char *);

char *b64encode(const uint8_t *, uint32_t, uint16_t);

#endif // GEODNS_STR_UTILS_H
