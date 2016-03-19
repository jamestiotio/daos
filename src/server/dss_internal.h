/**
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * GOVERNMENT LICENSE RIGHTS-OPEN SOURCE SOFTWARE
 * The Government's rights to use, modify, reproduce, release, perform, display,
 * or disclose this software are subject to the terms of the LGPL License as
 * provided in Contract No. B609815.
 * Any reproduction of computer software, computer software documentation, or
 * portions thereof marked with this legend must also reproduce the markings.
 *
 * (C) Copyright 2016 Intel Corporation.
 */

#include <daos_srv/daos_server.h>

/* module.cc */
int dss_module_init(void);
int dss_module_fini(bool force);
int dss_module_load(const char *modname);
int dss_module_unload(const char *modname);

/* rpc.cc */
int dss_rpc_register(struct dss_handler *hdlrs, int modid,
		     int client_or_server);
int dss_rpc_unregister(struct dss_handler *hdlrs);

/* srv.cc */
int dss_srv_init(void);
int dss_srv_fini();

/* tls.cc */
void dss_tls_fini(void *arg);
struct dss_tls *dss_tls_init();
