/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* NetworkManager Wireless Applet -- Display wireless access points and allow user control
 *
 * Dan Williams <dcbw@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * (C) Copyright 2007 - 2010 Red Hat, Inc.
 */

#ifndef EAP_METHOD_SIMPLE_H
#define EAP_METHOD_SIMPLE_H

#include "wireless-security.h"

typedef enum {
	EAP_METHOD_SIMPLE_TYPE_PAP = 0,
	EAP_METHOD_SIMPLE_TYPE_MSCHAP,
	EAP_METHOD_SIMPLE_TYPE_MSCHAP_V2,
	EAP_METHOD_SIMPLE_TYPE_MD5,
	EAP_METHOD_SIMPLE_TYPE_CHAP,
	EAP_METHOD_SIMPLE_TYPE_GTC
} EAPMethodSimpleType;

typedef struct _EAPMethodSimple EAPMethodSimple;

EAPMethodSimple *eap_method_simple_new (WirelessSecurity *ws_parent,
                                        NMConnection *connection,
                                        EAPMethodSimpleType type,
                                        gboolean phase2,
                                        gboolean is_editor);

#endif /* EAP_METHOD_SIMPLE_H */

