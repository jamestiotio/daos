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
/**
 * This file is part of the DAOS server. It implements the modular interface
 * to load server-side code on demand. DAOS modules are effectively dynamic
 * libraries loaded on-the-fly in the DAOS server via dlopen(3).
 */

#include <daos_srv/daos_server.h>
#include <daos/daos_rpc.h>
#include <daos/daos_transport.h>
/**
 * register rpc handlers of dss.
 *
 * Register RPC handlers of one module to DTP for each module.
 * \param[in] hdlrs	handlers to be registered.
 * \param[in] mod_id	module id of the module.
 * \param[in] server_only 0 means client, 1 means server.
 *
 * \retval	0 if registration succeeds
 * \retval	negative errno if registration fails.
 */
int
dss_rpc_register(struct dss_handler *hdlrs, int mod_id,
		 int server_only)
{
	struct dss_handler	*dsh;
	int			 rc;

	if (hdlrs == NULL)
		return 0;

	/* walk through the handler list and register each individual RPC */
	for (dsh = hdlrs; dsh->sh_opc != 0; dsh++) {
		dtp_opcode_t opcode;

		opcode = DSS_RPC_OPCODE(dsh->sh_opc, mod_id,
					dsh->sh_ver, server_only);
		rc = dtp_rpc_srv_reg(opcode, dsh->sh_in_hdlr,
				     dsh->sh_out_hdlr, dsh->sh_in_sz,
				     dsh->sh_out_sz, dsh->sh_hdlr);
		if (rc)
			return rc;
	}
	return 0;
}

int
dss_rpc_unregister(struct dss_handler *hdlrs)
{
	if (hdlrs == NULL)
		return 0;

	/* no supported for now */
	return 0;
}
