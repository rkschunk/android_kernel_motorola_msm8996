/*******************************************************************************

  Intel 10 Gigabit PCI Express Linux driver
  Copyright(c) 1999 - 2009 Intel Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:
  Linux NICS <linux.nics@intel.com>
  e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
  Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497

*******************************************************************************/

#include "ixgbe.h"
#include <linux/dcbnl.h>

/* Callbacks for DCB netlink in the kernel */
#define BIT_DCB_MODE	0x01
#define BIT_PFC		0x02
#define BIT_PG_RX	0x04
#define BIT_PG_TX	0x08
#define BIT_BCN         0x10
#define BIT_LINKSPEED   0x80

int ixgbe_copy_dcb_cfg(struct ixgbe_dcb_config *src_dcb_cfg,
                       struct ixgbe_dcb_config *dst_dcb_cfg, int tc_max)
{
	struct tc_configuration *src_tc_cfg = NULL;
	struct tc_configuration *dst_tc_cfg = NULL;
	int i;

	if (!src_dcb_cfg || !dst_dcb_cfg)
		return -EINVAL;

	for (i = DCB_PG_ATTR_TC_0; i < tc_max + DCB_PG_ATTR_TC_0; i++) {
		src_tc_cfg = &src_dcb_cfg->tc_config[i - DCB_PG_ATTR_TC_0];
		dst_tc_cfg = &dst_dcb_cfg->tc_config[i - DCB_PG_ATTR_TC_0];

		dst_tc_cfg->path[DCB_TX_CONFIG].prio_type =
				src_tc_cfg->path[DCB_TX_CONFIG].prio_type;

		dst_tc_cfg->path[DCB_TX_CONFIG].bwg_id =
				src_tc_cfg->path[DCB_TX_CONFIG].bwg_id;

		dst_tc_cfg->path[DCB_TX_CONFIG].bwg_percent =
				src_tc_cfg->path[DCB_TX_CONFIG].bwg_percent;

		dst_tc_cfg->path[DCB_TX_CONFIG].up_to_tc_bitmap =
				src_tc_cfg->path[DCB_TX_CONFIG].up_to_tc_bitmap;

		dst_tc_cfg->path[DCB_RX_CONFIG].prio_type =
				src_tc_cfg->path[DCB_RX_CONFIG].prio_type;

		dst_tc_cfg->path[DCB_RX_CONFIG].bwg_id =
				src_tc_cfg->path[DCB_RX_CONFIG].bwg_id;

		dst_tc_cfg->path[DCB_RX_CONFIG].bwg_percent =
				src_tc_cfg->path[DCB_RX_CONFIG].bwg_percent;

		dst_tc_cfg->path[DCB_RX_CONFIG].up_to_tc_bitmap =
				src_tc_cfg->path[DCB_RX_CONFIG].up_to_tc_bitmap;
	}

	for (i = DCB_PG_ATTR_BW_ID_0; i < DCB_PG_ATTR_BW_ID_MAX; i++) {
		dst_dcb_cfg->bw_percentage[DCB_TX_CONFIG]
			[i-DCB_PG_ATTR_BW_ID_0] = src_dcb_cfg->bw_percentage
				[DCB_TX_CONFIG][i-DCB_PG_ATTR_BW_ID_0];
		dst_dcb_cfg->bw_percentage[DCB_RX_CONFIG]
			[i-DCB_PG_ATTR_BW_ID_0] = src_dcb_cfg->bw_percentage
				[DCB_RX_CONFIG][i-DCB_PG_ATTR_BW_ID_0];
	}

	for (i = DCB_PFC_UP_ATTR_0; i < DCB_PFC_UP_ATTR_MAX; i++) {
		dst_dcb_cfg->tc_config[i - DCB_PFC_UP_ATTR_0].dcb_pfc =
			src_dcb_cfg->tc_config[i - DCB_PFC_UP_ATTR_0].dcb_pfc;
	}

	return 0;
}

static u8 ixgbe_dcbnl_get_state(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	DPRINTK(DRV, INFO, "Get DCB Admin Mode.\n");

	return !!(adapter->flags & IXGBE_FLAG_DCB_ENABLED);
}

static u8 ixgbe_dcbnl_set_state(struct net_device *netdev, u8 state)
{
	u8 err = 0;
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	DPRINTK(DRV, INFO, "Set DCB Admin Mode.\n");

	if (state > 0) {
		/* Turn on DCB */
		if (adapter->flags & IXGBE_FLAG_DCB_ENABLED)
			goto out;

		if (!(adapter->flags & IXGBE_FLAG_MSIX_ENABLED)) {
			DPRINTK(DRV, ERR, "Enable failed, needs MSI-X\n");
			err = 1;
			goto out;
		}

		if (netif_running(netdev))
			netdev->netdev_ops->ndo_stop(netdev);
		ixgbe_reset_interrupt_capability(adapter);
		ixgbe_napi_del_all(adapter);
		INIT_LIST_HEAD(&netdev->napi_list);
		kfree(adapter->tx_ring);
		kfree(adapter->rx_ring);
		adapter->tx_ring = NULL;
		adapter->rx_ring = NULL;

		adapter->hw.fc.requested_mode = ixgbe_fc_pfc;
		adapter->flags &= ~IXGBE_FLAG_RSS_ENABLED;
		adapter->flags |= IXGBE_FLAG_DCB_ENABLED;
		ixgbe_init_interrupt_scheme(adapter);
		if (netif_running(netdev))
			netdev->netdev_ops->ndo_open(netdev);
	} else {
		/* Turn off DCB */
		if (adapter->flags & IXGBE_FLAG_DCB_ENABLED) {
			adapter->hw.fc.requested_mode = ixgbe_fc_default;
			if (netif_running(netdev))
				netdev->netdev_ops->ndo_stop(netdev);
			ixgbe_reset_interrupt_capability(adapter);
			ixgbe_napi_del_all(adapter);
			INIT_LIST_HEAD(&netdev->napi_list);
			kfree(adapter->tx_ring);
			kfree(adapter->rx_ring);
			adapter->tx_ring = NULL;
			adapter->rx_ring = NULL;

			adapter->flags &= ~IXGBE_FLAG_DCB_ENABLED;
			adapter->flags |= IXGBE_FLAG_RSS_ENABLED;
			ixgbe_init_interrupt_scheme(adapter);
			if (netif_running(netdev))
				netdev->netdev_ops->ndo_open(netdev);
		}
	}
out:
	return err;
}

static void ixgbe_dcbnl_get_perm_hw_addr(struct net_device *netdev,
					 u8 *perm_addr)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	int i;

	for (i = 0; i < netdev->addr_len; i++)
		perm_addr[i] = adapter->hw.mac.perm_addr[i];
}

static void ixgbe_dcbnl_set_pg_tc_cfg_tx(struct net_device *netdev, int tc,
                                         u8 prio, u8 bwg_id, u8 bw_pct,
                                         u8 up_map)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	if (prio != DCB_ATTR_VALUE_UNDEFINED)
		adapter->temp_dcb_cfg.tc_config[tc].path[0].prio_type = prio;
	if (bwg_id != DCB_ATTR_VALUE_UNDEFINED)
		adapter->temp_dcb_cfg.tc_config[tc].path[0].bwg_id = bwg_id;
	if (bw_pct != DCB_ATTR_VALUE_UNDEFINED)
		adapter->temp_dcb_cfg.tc_config[tc].path[0].bwg_percent =
			bw_pct;
	if (up_map != DCB_ATTR_VALUE_UNDEFINED)
		adapter->temp_dcb_cfg.tc_config[tc].path[0].up_to_tc_bitmap =
			up_map;

	if ((adapter->temp_dcb_cfg.tc_config[tc].path[0].prio_type !=
	     adapter->dcb_cfg.tc_config[tc].path[0].prio_type) ||
	    (adapter->temp_dcb_cfg.tc_config[tc].path[0].bwg_id !=
	     adapter->dcb_cfg.tc_config[tc].path[0].bwg_id) ||
	    (adapter->temp_dcb_cfg.tc_config[tc].path[0].bwg_percent !=
	     adapter->dcb_cfg.tc_config[tc].path[0].bwg_percent) ||
	    (adapter->temp_dcb_cfg.tc_config[tc].path[0].up_to_tc_bitmap !=
	     adapter->dcb_cfg.tc_config[tc].path[0].up_to_tc_bitmap))
		adapter->dcb_set_bitmap |= BIT_PG_TX;
}

static void ixgbe_dcbnl_set_pg_bwg_cfg_tx(struct net_device *netdev, int bwg_id,
                                          u8 bw_pct)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	adapter->temp_dcb_cfg.bw_percentage[0][bwg_id] = bw_pct;

	if (adapter->temp_dcb_cfg.bw_percentage[0][bwg_id] !=
	    adapter->dcb_cfg.bw_percentage[0][bwg_id])
		adapter->dcb_set_bitmap |= BIT_PG_RX;
}

static void ixgbe_dcbnl_set_pg_tc_cfg_rx(struct net_device *netdev, int tc,
                                         u8 prio, u8 bwg_id, u8 bw_pct,
                                         u8 up_map)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	if (prio != DCB_ATTR_VALUE_UNDEFINED)
		adapter->temp_dcb_cfg.tc_config[tc].path[1].prio_type = prio;
	if (bwg_id != DCB_ATTR_VALUE_UNDEFINED)
		adapter->temp_dcb_cfg.tc_config[tc].path[1].bwg_id = bwg_id;
	if (bw_pct != DCB_ATTR_VALUE_UNDEFINED)
		adapter->temp_dcb_cfg.tc_config[tc].path[1].bwg_percent =
			bw_pct;
	if (up_map != DCB_ATTR_VALUE_UNDEFINED)
		adapter->temp_dcb_cfg.tc_config[tc].path[1].up_to_tc_bitmap =
			up_map;

	if ((adapter->temp_dcb_cfg.tc_config[tc].path[1].prio_type !=
	     adapter->dcb_cfg.tc_config[tc].path[1].prio_type) ||
	    (adapter->temp_dcb_cfg.tc_config[tc].path[1].bwg_id !=
	     adapter->dcb_cfg.tc_config[tc].path[1].bwg_id) ||
	    (adapter->temp_dcb_cfg.tc_config[tc].path[1].bwg_percent !=
	     adapter->dcb_cfg.tc_config[tc].path[1].bwg_percent) ||
	    (adapter->temp_dcb_cfg.tc_config[tc].path[1].up_to_tc_bitmap !=
	     adapter->dcb_cfg.tc_config[tc].path[1].up_to_tc_bitmap))
		adapter->dcb_set_bitmap |= BIT_PG_RX;
}

static void ixgbe_dcbnl_set_pg_bwg_cfg_rx(struct net_device *netdev, int bwg_id,
                                          u8 bw_pct)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	adapter->temp_dcb_cfg.bw_percentage[1][bwg_id] = bw_pct;

	if (adapter->temp_dcb_cfg.bw_percentage[1][bwg_id] !=
	    adapter->dcb_cfg.bw_percentage[1][bwg_id])
		adapter->dcb_set_bitmap |= BIT_PG_RX;
}

static void ixgbe_dcbnl_get_pg_tc_cfg_tx(struct net_device *netdev, int tc,
                                         u8 *prio, u8 *bwg_id, u8 *bw_pct,
                                         u8 *up_map)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	*prio = adapter->dcb_cfg.tc_config[tc].path[0].prio_type;
	*bwg_id = adapter->dcb_cfg.tc_config[tc].path[0].bwg_id;
	*bw_pct = adapter->dcb_cfg.tc_config[tc].path[0].bwg_percent;
	*up_map = adapter->dcb_cfg.tc_config[tc].path[0].up_to_tc_bitmap;
}

static void ixgbe_dcbnl_get_pg_bwg_cfg_tx(struct net_device *netdev, int bwg_id,
                                          u8 *bw_pct)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	*bw_pct = adapter->dcb_cfg.bw_percentage[0][bwg_id];
}

static void ixgbe_dcbnl_get_pg_tc_cfg_rx(struct net_device *netdev, int tc,
                                         u8 *prio, u8 *bwg_id, u8 *bw_pct,
                                         u8 *up_map)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	*prio = adapter->dcb_cfg.tc_config[tc].path[1].prio_type;
	*bwg_id = adapter->dcb_cfg.tc_config[tc].path[1].bwg_id;
	*bw_pct = adapter->dcb_cfg.tc_config[tc].path[1].bwg_percent;
	*up_map = adapter->dcb_cfg.tc_config[tc].path[1].up_to_tc_bitmap;
}

static void ixgbe_dcbnl_get_pg_bwg_cfg_rx(struct net_device *netdev, int bwg_id,
                                          u8 *bw_pct)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	*bw_pct = adapter->dcb_cfg.bw_percentage[1][bwg_id];
}

static void ixgbe_dcbnl_set_pfc_cfg(struct net_device *netdev, int priority,
                                    u8 setting)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	adapter->temp_dcb_cfg.tc_config[priority].dcb_pfc = setting;
	if (adapter->temp_dcb_cfg.tc_config[priority].dcb_pfc !=
	    adapter->dcb_cfg.tc_config[priority].dcb_pfc)
		adapter->dcb_set_bitmap |= BIT_PFC;
}

static void ixgbe_dcbnl_get_pfc_cfg(struct net_device *netdev, int priority,
                                    u8 *setting)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	*setting = adapter->dcb_cfg.tc_config[priority].dcb_pfc;
}

static u8 ixgbe_dcbnl_set_all(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	int ret;

	adapter->dcb_set_bitmap &= ~BIT_BCN;	/* no set for BCN */
	if (!adapter->dcb_set_bitmap)
		return 1;

	while (test_and_set_bit(__IXGBE_RESETTING, &adapter->state))
		msleep(1);

	if (netif_running(netdev))
		ixgbe_down(adapter);

	ret = ixgbe_copy_dcb_cfg(&adapter->temp_dcb_cfg, &adapter->dcb_cfg,
				 adapter->ring_feature[RING_F_DCB].indices);
	if (ret) {
		clear_bit(__IXGBE_RESETTING, &adapter->state);
		return ret;
	}

	if (netif_running(netdev))
		ixgbe_up(adapter);

	adapter->dcb_set_bitmap = 0x00;
	clear_bit(__IXGBE_RESETTING, &adapter->state);
	return ret;
}

static u8 ixgbe_dcbnl_getcap(struct net_device *netdev, int capid, u8 *cap)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	u8 rval = 0;

	if (adapter->flags & IXGBE_FLAG_DCB_ENABLED) {
		switch (capid) {
		case DCB_CAP_ATTR_PG:
			*cap = true;
			break;
		case DCB_CAP_ATTR_PFC:
			*cap = true;
			break;
		case DCB_CAP_ATTR_UP2TC:
			*cap = false;
			break;
		case DCB_CAP_ATTR_PG_TCS:
			*cap = 0x80;
			break;
		case DCB_CAP_ATTR_PFC_TCS:
			*cap = 0x80;
			break;
		case DCB_CAP_ATTR_GSP:
			*cap = true;
			break;
		case DCB_CAP_ATTR_BCN:
			*cap = false;
			break;
		default:
			rval = -EINVAL;
			break;
		}
	} else {
		rval = -EINVAL;
	}

	return rval;
}

static u8 ixgbe_dcbnl_getnumtcs(struct net_device *netdev, int tcid, u8 *num)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);
	u8 rval = 0;

	if (adapter->flags & IXGBE_FLAG_DCB_ENABLED) {
		switch (tcid) {
		case DCB_NUMTCS_ATTR_PG:
			*num = MAX_TRAFFIC_CLASS;
			break;
		case DCB_NUMTCS_ATTR_PFC:
			*num = MAX_TRAFFIC_CLASS;
			break;
		default:
			rval = -EINVAL;
			break;
		}
	} else {
		rval = -EINVAL;
	}

	return rval;
}

static u8 ixgbe_dcbnl_setnumtcs(struct net_device *netdev, int tcid, u8 num)
{
	return -EINVAL;
}

static u8 ixgbe_dcbnl_getpfcstate(struct net_device *netdev)
{
	struct ixgbe_adapter *adapter = netdev_priv(netdev);

	return !!(adapter->flags & IXGBE_FLAG_DCB_ENABLED);
}

static void ixgbe_dcbnl_setpfcstate(struct net_device *netdev, u8 state)
{
	return;
}

struct dcbnl_rtnl_ops dcbnl_ops = {
	.getstate	= ixgbe_dcbnl_get_state,
	.setstate	= ixgbe_dcbnl_set_state,
	.getpermhwaddr	= ixgbe_dcbnl_get_perm_hw_addr,
	.setpgtccfgtx	= ixgbe_dcbnl_set_pg_tc_cfg_tx,
	.setpgbwgcfgtx	= ixgbe_dcbnl_set_pg_bwg_cfg_tx,
	.setpgtccfgrx	= ixgbe_dcbnl_set_pg_tc_cfg_rx,
	.setpgbwgcfgrx	= ixgbe_dcbnl_set_pg_bwg_cfg_rx,
	.getpgtccfgtx	= ixgbe_dcbnl_get_pg_tc_cfg_tx,
	.getpgbwgcfgtx	= ixgbe_dcbnl_get_pg_bwg_cfg_tx,
	.getpgtccfgrx	= ixgbe_dcbnl_get_pg_tc_cfg_rx,
	.getpgbwgcfgrx	= ixgbe_dcbnl_get_pg_bwg_cfg_rx,
	.setpfccfg	= ixgbe_dcbnl_set_pfc_cfg,
	.getpfccfg	= ixgbe_dcbnl_get_pfc_cfg,
	.setall		= ixgbe_dcbnl_set_all,
	.getcap		= ixgbe_dcbnl_getcap,
	.getnumtcs	= ixgbe_dcbnl_getnumtcs,
	.setnumtcs	= ixgbe_dcbnl_setnumtcs,
	.getpfcstate	= ixgbe_dcbnl_getpfcstate,
	.setpfcstate	= ixgbe_dcbnl_setpfcstate,
};

