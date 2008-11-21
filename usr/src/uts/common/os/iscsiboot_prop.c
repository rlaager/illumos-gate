/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * Commmon routines, handling iscsi boot props
 */

#include <sys/types.h>
#include <sys/bootprops.h>
#include <sys/cmn_err.h>
#include <sys/socket.h>
#include <sys/kmem.h>
#include <netinet/in.h>

extern void *memset(void *s, int c, size_t n);
extern int memcmp(const void *s1, const void *s2, size_t n);
extern void bcopy(const void *s1, void *s2, size_t n);
extern size_t strlen(const char *s);
static void kinet_ntoa(char *buf, void *in, int af);
extern ib_boot_prop_t *iscsiboot_prop;

int  iscsi_print_bootprop	=	0;

#define	ISCSI_BOOTPROP_BUFLEN	256

#ifndef	NULL
#define	NULL	0
#endif

static void
iscsi_bootprop_print(int level, char *str)
{
	if (str == NULL) {
		return;
	}
	if (iscsi_print_bootprop == 1) {
		cmn_err(level, "%s", str);
	}
}

static void
iscsi_print_initiator_property(ib_ini_prop_t *ibinitp)
{
	char	outbuf[ISCSI_BOOTPROP_BUFLEN] = {0};

	if (ibinitp == NULL) {
		return;
	}

	if (ibinitp->ini_name != NULL) {
		(void) sprintf(outbuf,
		    "Initiator Name : %s\n",
		    ibinitp->ini_name);
		iscsi_bootprop_print(CE_CONT, outbuf);
	}

	if (ibinitp->ini_chap_name != NULL) {
		(void) memset(outbuf, 0, ISCSI_BOOTPROP_BUFLEN);
		(void) sprintf(outbuf,
		    "Initiator CHAP Name  : %s\n",
		    ibinitp->ini_chap_name);

		iscsi_bootprop_print(CE_CONT, outbuf);
	}
}

static void
iscsi_print_nic_property(ib_nic_prop_t *nicp)
{
	char	outbuf[ISCSI_BOOTPROP_BUFLEN] = {0};
	char	ipaddr[50]  =	{0};
	int	n	    =	0;

	if (nicp == NULL) {
		return;
	}

	kinet_ntoa(ipaddr, &nicp->nic_ip_u, nicp->sin_family);
	n = snprintf(outbuf, ISCSI_BOOTPROP_BUFLEN,
	    "Local IP addr  : %s\n", ipaddr);

	(void) memset(ipaddr, 0, 50);
	kinet_ntoa(ipaddr, &nicp->nic_gw_u, nicp->sin_family);
	n = n + snprintf(outbuf + n, ISCSI_BOOTPROP_BUFLEN - n,
	    "Local gateway  : %s\n", ipaddr);

	(void) memset(ipaddr, 0, 50);
	kinet_ntoa(ipaddr, &nicp->nic_dhcp_u, nicp->sin_family);
	n = n + snprintf(outbuf + n, ISCSI_BOOTPROP_BUFLEN - n,
	    "Local DHCP     : %s\n", ipaddr);

	(void) snprintf(outbuf + n, ISCSI_BOOTPROP_BUFLEN - n,
	    "Local MAC      : %02x:%02x:%02x:%02x:%02x:%02x\n",
	    nicp->nic_mac[0],
	    nicp->nic_mac[1],
	    nicp->nic_mac[2],
	    nicp->nic_mac[3],
	    nicp->nic_mac[4],
	    nicp->nic_mac[5]);

	iscsi_bootprop_print(CE_CONT, outbuf);
}

static void
iscsi_print_tgt_property(ib_tgt_prop_t *itgtp)
{
	char	outbuf[ISCSI_BOOTPROP_BUFLEN] = {0};
	char	ipaddr[50]  =	{0};

	if (itgtp == NULL) {
		return;
	}

	if (itgtp->tgt_name != NULL) {
		(void) memset(outbuf, 0, ISCSI_BOOTPROP_BUFLEN);
		(void) sprintf(outbuf,
		    "Target Name    : %s\n",
		    itgtp->tgt_name);
		iscsi_bootprop_print(CE_CONT, outbuf);
	}

	kinet_ntoa(ipaddr, &itgtp->tgt_ip_u, itgtp->sin_family);
	(void) sprintf(outbuf,
	    "Target IP      : %s\n"
	    "Target Port    : %d\n"
	    "Boot LUN       : %02x%02x-%02x%02x-%02x%02x-%02x%02x\n",
	    ipaddr,
	    itgtp->tgt_port,
	    itgtp->tgt_boot_lun[0],
	    itgtp->tgt_boot_lun[1],
	    itgtp->tgt_boot_lun[2],
	    itgtp->tgt_boot_lun[3],
	    itgtp->tgt_boot_lun[4],
	    itgtp->tgt_boot_lun[5],
	    itgtp->tgt_boot_lun[6],
	    itgtp->tgt_boot_lun[7]);
	iscsi_bootprop_print(CE_CONT, outbuf);

	if (itgtp->tgt_chap_name != NULL) {
		(void) memset(outbuf, 0, ISCSI_BOOTPROP_BUFLEN);
		(void) sprintf(outbuf,
		    "CHAP Name      : %s\n",
		    itgtp->tgt_chap_name);
		iscsi_bootprop_print(CE_CONT, outbuf);
	}
}

void
iscsi_print_boot_property()
{
	if (iscsiboot_prop == NULL) {
		return;
	}

	iscsi_print_initiator_property(
	    &iscsiboot_prop->boot_init);

	iscsi_print_nic_property(&iscsiboot_prop->boot_nic);

	iscsi_print_tgt_property(&iscsiboot_prop->boot_tgt);
}

void
iscsi_boot_free_ini(ib_ini_prop_t *init)
{
	if (init == NULL) {
		return;
	}

	if (init->ini_name != NULL) {
		kmem_free(init->ini_name, strlen((char *)init->ini_name) + 1);
		init->ini_name = NULL;
	}
	if (init->ini_chap_name != NULL) {
		kmem_free(init->ini_chap_name,
		    strlen((char *)init->ini_chap_name) + 1);
		init->ini_chap_name = NULL;
	}
	if (init->ini_chap_sec != NULL) {
		kmem_free(init->ini_chap_sec,
		    strlen((char *)init->ini_chap_sec) + 1);
		init->ini_chap_sec = NULL;
	}
}

void
iscsi_boot_free_tgt(ib_tgt_prop_t *target)
{
	if (target == NULL) {
		return;
	}

	if (target->tgt_name != NULL) {
		kmem_free(target->tgt_name,
		    strlen((char *)target->tgt_name) + 1);
		target->tgt_name = NULL;
	}
	if (target->tgt_chap_name != NULL) {
		kmem_free(target->tgt_chap_name,
		    strlen((char *)target->tgt_chap_name) + 1);
		target->tgt_chap_name = NULL;
	}
	if (target->tgt_chap_sec != NULL) {
		kmem_free(target->tgt_chap_sec,
		    strlen((char *)target->tgt_chap_sec) + 1);
		target->tgt_chap_sec = NULL;
	}
}

/*
 * Free the memory used by boot property.
 */
void
iscsi_boot_prop_free()
{
	ib_boot_prop_t	*tmp;

	if (iscsiboot_prop == NULL) {
		return;
	}
	tmp = iscsiboot_prop;
	iscsiboot_prop = NULL;
	iscsi_boot_free_ini(&(tmp->boot_init));
	iscsi_boot_free_tgt(&(tmp->boot_tgt));
}

static void
kinet_ntoa(char *buf, void *in, int af)
{
	unsigned char   *p =    NULL;
	int	i = 0;

	if (buf == NULL || in == NULL) {
		return;
	}
	p = (unsigned char *)in;
	if (af == AF_INET) {
		(void) sprintf(buf, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	} else {
		for (i = 0; i < 14; i = i + 2) {
			(void) sprintf(buf, "%02x%02x:", p[i], p[i+1]);
			buf = buf + 5;
		}
		(void) sprintf(buf, "%02x%02x", p[i], p[i+1]);
	}
}
