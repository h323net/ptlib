
/*
* android_resolv.h
*
* Copyright (c) 2016 Spranto International Pte Ltd. All Rights Reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, you can obtain one at http://mozilla.org/MPL/2.0/
*
* Software distributed under the License is distributed on an "AS IS"
* basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
* the License for the specific language governing rights and limitations
* under the License.
*
* Contributor(s): ______________________________________.
*
*
*/

#pragma once

#if defined(P_ANDROID)
// Add missing Android resolve headers

#define PACKETSZ	512   // default UDP packet size
#define MAXDNAME    1025  // maximum domain name

#define QFIXEDSZ	4	  // # bytes of fixed data in query 

#define  T_A      1     // Internet Address
#define  T_NS     2     // Name Server
#define  T_MX     15    // Mail Routing
#define  T_AAAA   28    // IPv6 Addresses
#define  T_SRV    33    // SRV record
#define  C_IN     1

typedef struct {
    unsigned    id : 16;     /*%< query identification number */
#if BYTE_ORDER == BIG_ENDIAN
                             /* fields in third byte */
    unsigned    qr : 1;      /*%< response flag */
    unsigned    opcode : 4;  /*%< purpose of message */
    unsigned    aa : 1;      /*%< authoritive answer */
    unsigned    tc : 1;      /*%< truncated message */
    unsigned    rd : 1;      /*%< recursion desired */
                             /* fields in fourth byte */
    unsigned    ra : 1;      /*%< recursion available */
    unsigned    unused : 1;  /*%< unused bits (MBZ as of 4.9.3a3) */
    unsigned    ad : 1;      /*%< authentic data from named */
    unsigned    cd : 1;      /*%< checking disabled by resolver */
    unsigned    rcode : 4;   /*%< response code */
#endif
#if BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
                             /* fields in third byte */
    unsigned    rd : 1;      /*%< recursion desired */
    unsigned    tc : 1;      /*%< truncated message */
    unsigned    aa : 1;      /*%< authoritive answer */
    unsigned    opcode : 4;  /*%< purpose of message */
    unsigned    qr : 1;      /*%< response flag */
                             /* fields in fourth byte */
    unsigned    rcode : 4;   /*%< response code */
    unsigned    cd : 1;      /*%< checking disabled by resolver */
    unsigned    ad : 1;      /*%< authentic data from named */
    unsigned    unused : 1;  /*%< unused bits (MBZ as of 4.9.3a3) */
    unsigned    ra : 1;      /*%< recursion available */
#endif
                             /* remaining bytes */
    unsigned    qdcount : 16;    /*%< number of question entries */
    unsigned    ancount : 16;    /*%< number of answer entries */
    unsigned    nscount : 16;    /*%< number of authority entries */
    unsigned    arcount : 16;    /*%< number of resource entries */
} HEADER;

#define NS_INT16SZ    2
#define NS_INT32SZ    4

#define NS_GET16(s, cp) do { \
     const u_char *t_cp = (const u_char *)(cp); \
     (s) = ((u_int16_t)t_cp[0] << 8) \
         | ((u_int16_t)t_cp[1]) \
         ; \
     (cp) += NS_INT16SZ; \
 } while (0)

#define GETSHORT   NS_GET16

#define NS_GET32(l, cp) do { \
	const u_char *t_cp = (const u_char *)(cp); \
	(l) = ((uint32_t)t_cp[0] << 24) \
	    | ((uint32_t)t_cp[1] << 16) \
	    | ((uint32_t)t_cp[2] << 8) \
	    | ((uint32_t)t_cp[3]) \
	    ; \
	(cp) += NS_INT32SZ; \
} while (0)
#define GETLONG   NS_GET32

int dn_expand(const unsigned char * srv, const unsigned char * end, const unsigned char * pos, char * name, const unsigned int n);

int res_init(void);

int res_search(const char * dname, int classx, int type, u_char *answer, int anslen);


#endif  // P_ANDROID

