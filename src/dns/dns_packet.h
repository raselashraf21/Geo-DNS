
#ifndef GEODNS_DNS_PACKET_H
#define GEODNS_DNS_PACKET_H

// For standard int qtype declarations.
#include <stdint.h>
#include "config.h"

/*
   __BIG_ENDIAN__ and __LITTLE_ENDIAN__ are define in some gcc versions
  only, probably depending on the architecture. Try to use endian.h if
  the gcc way fails - endian.h also doesn not seem to be available on all
  platforms.
*/
#ifdef __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#else /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
#undef WORDS_BIGENDIAN
#else
#ifdef BSD
#include <sys/endian.h>
#else
#include <endian.h>
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#undef WORDS_BIGENDIAN
#else
#error "unable to determine endianess!"
#endif /* __BYTE_ORDER */
#endif /* __LITTLE_ENDIAN__ */
#endif /* __BIG_ENDIAN__ */

// According to :
//      rfc-1034: https://tools.ietf.org/html/rfc1034 (Nov, 1987) "DOMAIN NAMES - CONCEPTS AND FACILITIES"
//      rfc-1035: https://tools.ietf.org/html/rfc1035 (Nov, 1987) "DOMAIN NAMES - IMPLEMENTATION AND SPECIFICATION"
//      rfc-6891: https://tools.ietf.org/html/rfc6891 (Apr, 2013) "Extension Mechanisms for DNS (EDNS(0))"
//      rfc-7871: https://tools.ietf.org/html/rfc7871 (May, 2016) "Client Subnet in DNS Queries"
#define RD_ENABLE 0

typedef enum rcode {
    no_error,
    format_error,
    server_failure ,
    name_error ,
    not_implemented ,
    refused
} RCODE;

// Holds the header for a dns questions/answer.
// NOTE: Should be filled according to 'Network Byte Order' (Big-endian Byte Order).
typedef struct dns_header{
    // DNS Header Length : 12 bytes, fixed both in Q & R
    uint16_t id;            // Length - 2 bytes : Id in R same as in Q

    // Flags                // Length - 2 bytes
#if WORDS_BIGENDIAN
    // fields in third byte
    uint16_t qr : 1;        // bit 0 : 0 - quesion, 1 - answer                  (Q/R)
    uint16_t opcode : 4;    // bit 1,2,3,4 : 0000 - Standard Query,             (Q)
                            //               0100 -
    uint16_t aa : 1;        // bit 5 : (Authoritative Answer)   1 - authorttive (R)
    uint16_t tc : 1;        // bit 6 : (TrunCation)             1 - truncated   (Q)
    uint16_t rd : 1;        // bit 7 : (Recursion Desired)      1 - derired     (Q)

    // fields in third byte
    uint16_t ra : 1;        // bit 8 : (Recursion Available)    1 - available   (R)
    uint16_t z  : 3;        // bit 9,10,11      : (Reserved for future use) should be set to 0
    uint16_t rcode: 4;      // bit 12,13,14,15  :(Response code)                (R)
                            //                  0000 No error
                            //                  ?? 1 Format error - The qname server was unable to interpret the query.
                            //                  ?? 2 Server failure - The qname server was unable to process this query
                            //                                        due to a problem with the qname server.
                            //                  ?? 3 Name Error - Meaningful only for responses from an authoritative
                            //                                    qname server, this code signifies that the domain
                            //                                    qname referenced in the query does not exist.
                            //                  ?? 4 Not Implemented - The qname server does not support the requested
                            //                                         kind of query.
                            //                  ?? 5 Refused - The qname run_server refuses to perform the specified
                            //                                 operation for policy reasons.
//#endif
//#if BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
#else
    // fields in third byte
    uint16_t rd : 1;        // bit 7 : (Recursion Desired)      1 - derired     (Q)
    uint16_t tc : 1;        // bit 6 : (TrunCation)             1 - truncated   (Q)
    uint16_t aa : 1;        // bit 5 : (Authoritative Answer)   1 - authorttive (R)
    uint16_t opcode : 4;    // bit 1,2,3,4 : 0000 - Standard Query,             (Q)
                            //               0100 -
    uint16_t qr : 1;        // bit 0 : 0 - quesion, 1 - answer                  (Q/R)
    // fields in fourth byte
    uint16_t rcode: 4;      // bit 12,13,14,15  :(Response code)                (R)
                            //                  0000 No error
                            //                  ?? 1 Format error - The qname run_server was unable to interpret the query.
                            //                  ?? 2 Server failure - The qname run_server was unable to process this query
                            //                                        due to a problem with the qname run_server.
                            //                  ?? 3 Name Error - Meaningful only for responses from an authoritative
                            //                                    qname run_server, this code signifies that the domain
                            //                                    qname referenced in the query does not exist.
                            //                  ?? 4 Not Implemented - The qname run_server does not support the requested
                            //                                         kind of query.
                            //                  ?? 5 Refused - The qname run_server refuses to perform the specified
                            //                                 operation for policy reasons.
    uint16_t z  : 3;        // bit 9,10,11      : (Reserved for future use) should be set to 0
    uint16_t ra : 1;        // bit 8 : (Recursion Available)    1 - available   (R)
#endif

    uint16_t qdcount;       // Length - 2 bytes : Question count in 'Question' Section.
    uint16_t ancount;       // Length - 2 bytes : Answer Record count in 'Answer' Section.
    uint16_t nscount;       // Length - 2 bytes : Authority Record count in 'AR' Section.
    uint16_t arcount;       // Length - 2 bytes : Aditional Record count in 'Aditional' Section.
} dns_header;

// Recommended Syntax for domain name & label:
//
// <domain> ::= <subdomain> | " "
// <subdomain> ::= <label> | <subdomain> "." <label>
// <label> ::= <letter> [ [ <ldh-str> ] <let-dig> ]
// <ldh-str> ::= <let-dig-hyp> | <let-dig-hyp> <ldh-str>
// <let-dig-hyp> ::= <let-dig> | "-"
// <let-dig> ::= <letter> | <digit>
// <letter> ::= any one of the 52 alphabetic characters A through Z in upper case and a through z in lower case
// <digit> ::= any one of the ten digits 0 through 9
//
// NOTE:
// * This tree of subdivisions may have up to 127 levels.
// * <label> : zero to 63 characters.
// * The full domain name may not exceed the length of 253 characters in its textual representation
// * udp packet size : 512 bytes
// * Two names with the same spelling but different case are to be treated as if identical.

// Holds the information for a dns questions.
typedef struct dns_question {
    char *qname;            // Length - var     : domain name = a sequence of labels, terminates with the zero length
                            //                                  octet for the null label of the root.
                            //                    label = consists of a length octet followed by that number of octets.
                            //                    e.g.
    uint16_t qtype;         // Length - 2 bytes : Query Type (QTYPE) codes. Superset of TYPE.
    uint16_t qclass;        // Length - 2 bytes : Query Class (QCLASS) codes. Superset of CLASS.
    struct dns_question * next;
} dns_question;

//
// OPT Record TTL Field Use
//                                                                          │  EXTENDED-RCODE
//    The extended RCODE and flags, which OPT stores in the RR Time to Live │     Forms the upper 8 bits of extended 12-bit RCODE (together with the
//    (TTL) field, are structured as follows:                               │     4 bits defined in [RFC1035].  Note that EXTENDED-RCODE value 0
//                                                                          │     indicates that an unextended RCODE is in use (values 0 through
//                   +0 (MSB)                            +1 (LSB)           │     15).
//        +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ │
//     0: |         EXTENDED-RCODE        |            VERSION            | │  VERSION
//        +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ │     Indicates the implementation level of the setter.
//     2: | DO|                           Z                               | │
//        +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ │  DO
//                                                                          │     DNSSEC OK bit as defined by [RFC3225].
//                                                                          │
//                                                                          │  Z
//                                                                          │     Set to zero by senders and ignored by receivers, unless modified
//                                                                          │     in a subsequent specification.
//
typedef struct dns_opt_ttl {
    uint32_t extended_rcode : 8;
    uint32_t version : 8;
    uint32_t dnssec_ok : 1;
    uint32_t z : 15;
} dns_opt_ttl;

//
//                +0 (MSB)                            +1 (LSB)            │ o  SOURCE PREFIX-LENGTH, an unsigned octet representing the leftmost
//      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ │    number of significant bits of ADDRESS to be used for the lookup.
//   0: |                          OPTION-CODE                          | │    In responses, it mirrors the same value as in the queries.
//      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ │
//   2: |                         OPTION-LENGTH                         | │
//      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ │ o  SCOPE PREFIX-LENGTH, an unsigned octet representing the leftmost
//   4: |                            FAMILY                             | │    number of significant bits of ADDRESS that the response covers.
//      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ │    In queries, it MUST be set to 0.
//   6: |     SOURCE PREFIX-LENGTH      |     SCOPE PREFIX-LENGTH       | │
//      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ │ o  ADDRESS, variable number of octets, contains either an IPv4 or
//   8: |                           ADDRESS...                          / │    IPv6 address, depending on FAMILY, which MUST be truncated to the
//      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ │    number of bits indicated by the SOURCE PREFIX-LENGTH field,
//                                                                        │    padding with 0 bits to pad to the end of the last octet needed.
//   o  (Defined in [RFC6891]) OPTION-CODE, 2 octets, for ECS is 8 (0x00  │
//      0x08).                                                            │ o  A run_server receiving an ECS option that uses either too few or too
//                                                                        │    many ADDRESS octets, or that has non-zero ADDRESS bits set beyond
//   o  (Defined in [RFC6891]) OPTION-LENGTH, 2 octets, contains the      │    SOURCE PREFIX-LENGTH, SHOULD return FORMERR to reject the packet,
//      length of the payload (everything after OPTION-LENGTH) in octets. │    as a signal to the software developer making the request to fix
//                                                                        │    their implementation.
//   o  FAMILY, 2 octets, ecs only defines the format for FAMILY 1 (IPv4) │
//      and FAMILY 2 (IPv6), which are as follows:
//
typedef struct dns_ecs_data {
    uint16_t family;
    uint8_t source_pre_len;
    uint8_t scope_pre_len;
    char *address;
} dns_ecs_data;

//
//               +0 (MSB)                            +1 (LSB)            │ OPTION-CODE
//    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+  │    Assigned by the Expert Review process as
// 0: |                          OPTION-CODE                          |  │    defined by the DNSEXT working group and
//    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+  │    the IESG.
// 2: |                         OPTION-LENGTH                         |  │
//    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+  │ OPTION-LENGTH
// 4: |                                                               |  │    Size (in octets) of OPTION-DATA.
//    /                          OPTION-DATA                          /  │
//    /                                                               /  │ OPTION-DATA
//    +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+  │    Varies per OPTION-CODE.  MUST be treated
//                                                                       │    as a bit field.
//
typedef struct dns_opt_data {
    uint16_t opt_code;
    uint16_t opt_length;
    void *opt_data;               // Length var : normal - char *, ecs_data - dns_ecs_data *

} dns_opt_data;

typedef union dns_rr_ttl {
    dns_opt_ttl opt_ttl;          // Length 4 : see above or,
    uint32_t norm_ttl;            // Length 4 : TTL
} dns_rr_ttl;

//
// RR definitions: (answer, nameserver, aditional)        │ Length = var + 2 + 2 + 4 + var                               │  A RDATA format
//                                    1  1  1  1  1  1    │        = var + 8 + var                                       │
//      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5    │                                                              │  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   │ For Authoritative DNS, important types are:                  │  |                    ADDRESS                    |
//    |                                               |   │    TYPE            value and meaning                         │  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//    /                                               /   │                                                              │
//    /                      NAME                     /   │    A               1 a host address                          │  where:
//    |                                               |   │    NS              2 an authoritative name run_server            │
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   │    CNAME           5 the canonical name for an alias         │  ADDRESS         A 32 bit Internet address.
//    |                      TYPE                     |   │    SOA             6 marks the start of a zone of authority  │
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   │    PTR             12 a domain name pointer                  │  Hosts that have multiple Internet addresses will
//    |                     CLASS                     |   │    HINFO           13 host information                       │  have multiple A records.
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   │
//    |                      TTL                      |   │ CLASS values                                                    or,
//    |                                               |   │                                                                 OPT RDATA format : see above
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   │    IN              1 the Internet
//    |                   RDLENGTH                    |   │    CH              3 the CHAOS class
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|   │    HS              4 Hesiod [Dyer 87]
//    /                     RDATA                     /   │
//    /                                               /   │
//    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+   │
//
typedef struct dns_rr {
    char * name;            // Length - var     : domain name = same as qname
    uint16_t type;          // Length - 2 bytes : RR TYPE codes. Subset of QTYPE.
    uint16_t cls;           // Length - 2 bytes : RR CLASS codes. Subset of QCLASS.
    const char * rr_name;
    dns_rr_ttl ttl;         // Length - 4 bytes : Specifies the time interval (in seconds) that the resource record may
                            //                    be cached before it should be discarded. Zero values are interpreted
                            //                    to mean that the RR can only be used for the transaction in progress,
                            //                    and should not be cached.
    uint16_t rdlength;      // Length - 2 bytes : Specifies the length in octets of the RDATA field.
    void *rdata;            // Length - var     : The format of this information varies according to the TYPE and CLASS
                            //                    of the resource record.
                            //                  : normal rdata - char *
                            // or               : edns rdata   - dns_opt_data *
    struct dns_rr * next;
} dns_rr;

typedef union {
    dns_header header;
    uint8_t buf[MAX_PACKET];
} dns_packet;

// Holds general DNS information.
typedef struct dns_info {
    dns_packet packet;
    uint16_t packet_length;     // unsigned
    dns_question * questions;
    dns_rr * answers;
    dns_rr * name_servers;
    dns_rr * additional;
    char * conn_ip;
} dns_info;

#endif // GEODNS_DNS_PACKET_H
