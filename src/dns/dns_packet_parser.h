
#ifndef GEODNS_DNS_PARSER_H
#define GEODNS_DNS_PARSER_H

#include "utility/log/log.h"
#include "dns_packet.h"

dns_info *create_info();

uint32_t parse_dns_packet(dns_info *const info);
void free_pointer(void **p);
uint8_t free_rr(dns_rr **p, uint16_t count);
uint8_t free_dns_packet(dns_info **info);

uint8_t print_dns_header(const dns_header *header);
uint8_t print_dns_question(const dns_question *question);
uint8_t print_dns_rr(const dns_rr *rr);
uint8_t print_dns_packet(const dns_info *info);

uint8_t inspect_dns_header(const dns_header *header,  log_context *ctx);
uint8_t inspect_dns_packet(const dns_info *const info,  log_context *ctx);
uint8_t inspect_dns_rr(const dns_rr *rr, log_context *ctx);
#endif // GEODNS_DNS_PARSER_H
