/*
 * drivers/net/bond/bond_options.c - bonding options
 * Copyright (c) 2013 Jiri Pirko <jiri@resnulli.us>
 * Copyright (c) 2013 Scott Feldman <sfeldma@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/errno.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/rwlock.h>
#include <linux/rcupdate.h>
#include <linux/ctype.h>
#include <linux/inet.h>
#include "bonding.h"

static struct bond_opt_value bond_mode_tbl[] = {
	{ "balance-rr",    BOND_MODE_ROUNDROBIN,   BOND_VALFLAG_DEFAULT},
	{ "active-backup", BOND_MODE_ACTIVEBACKUP, 0},
	{ "balance-xor",   BOND_MODE_XOR,          0},
	{ "broadcast",     BOND_MODE_BROADCAST,    0},
	{ "802.3ad",       BOND_MODE_8023AD,       0},
	{ "balance-tlb",   BOND_MODE_TLB,          0},
	{ "balance-alb",   BOND_MODE_ALB,          0},
	{ NULL,            -1,                     0},
};

static struct bond_opt_value bond_pps_tbl[] = {
	{ "default", 1,         BOND_VALFLAG_DEFAULT},
	{ "maxval",  USHRT_MAX, BOND_VALFLAG_MAX},
	{ NULL,      -1,        0},
};

static struct bond_opt_value bond_xmit_hashtype_tbl[] = {
	{ "layer2",   BOND_XMIT_POLICY_LAYER2, BOND_VALFLAG_DEFAULT},
	{ "layer3+4", BOND_XMIT_POLICY_LAYER34, 0},
	{ "layer2+3", BOND_XMIT_POLICY_LAYER23, 0},
	{ "encap2+3", BOND_XMIT_POLICY_ENCAP23, 0},
	{ "encap3+4", BOND_XMIT_POLICY_ENCAP34, 0},
	{ NULL,       -1,                       0},
};

static struct bond_opt_value bond_arp_validate_tbl[] = {
	{ "none",   BOND_ARP_VALIDATE_NONE,   BOND_VALFLAG_DEFAULT},
	{ "active", BOND_ARP_VALIDATE_ACTIVE, 0},
	{ "backup", BOND_ARP_VALIDATE_BACKUP, 0},
	{ "all",    BOND_ARP_VALIDATE_ALL,    0},
	{ NULL,     -1,                       0},
};

static struct bond_opt_value bond_arp_all_targets_tbl[] = {
	{ "any", BOND_ARP_TARGETS_ANY, BOND_VALFLAG_DEFAULT},
	{ "all", BOND_ARP_TARGETS_ALL, 0},
	{ NULL,  -1,                   0},
};

static struct bond_opt_value bond_fail_over_mac_tbl[] = {
	{ "none",   BOND_FOM_NONE,   BOND_VALFLAG_DEFAULT},
	{ "active", BOND_FOM_ACTIVE, 0},
	{ "follow", BOND_FOM_FOLLOW, 0},
	{ NULL,     -1,              0},
};

static struct bond_opt_value bond_intmax_tbl[] = {
	{ "off",     0,       BOND_VALFLAG_DEFAULT},
	{ "maxval",  INT_MAX, BOND_VALFLAG_MAX},
};

static struct bond_opt_value bond_lacp_rate_tbl[] = {
	{ "slow", AD_LACP_SLOW, 0},
	{ "fast", AD_LACP_FAST, 0},
	{ NULL,   -1,           0},
};

static struct bond_opt_value bond_ad_select_tbl[] = {
	{ "stable",    BOND_AD_STABLE,    BOND_VALFLAG_DEFAULT},
	{ "bandwidth", BOND_AD_BANDWIDTH, 0},
	{ "count",     BOND_AD_COUNT,     0},
	{ NULL,        -1,                0},
};

static struct bond_opt_value bond_num_peer_notif_tbl[] = {
	{ "off",     0,   0},
	{ "maxval",  255, BOND_VALFLAG_MAX},
	{ "default", 1,   BOND_VALFLAG_DEFAULT},
	{ NULL,      -1,  0}
};

static struct bond_opt_value bond_primary_reselect_tbl[] = {
	{ "always",  BOND_PRI_RESELECT_ALWAYS,  BOND_VALFLAG_DEFAULT},
	{ "better",  BOND_PRI_RESELECT_BETTER,  0},
	{ "failure", BOND_PRI_RESELECT_FAILURE, 0},
	{ NULL,      -1},
};

static struct bond_opt_value bond_use_carrier_tbl[] = {
	{ "off", 0,  0},
	{ "on",  1,  BOND_VALFLAG_DEFAULT},
	{ NULL,  -1, 0}
};

static struct bond_option bond_opts[] = {
	[BOND_OPT_MODE] = {
		.id = BOND_OPT_MODE,
		.name = "mode",
		.desc = "bond device mode",
		.flags = BOND_OPTFLAG_NOSLAVES | BOND_OPTFLAG_IFDOWN,
		.values = bond_mode_tbl,
		.set = bond_option_mode_set
	},
	[BOND_OPT_PACKETS_PER_SLAVE] = {
		.id = BOND_OPT_PACKETS_PER_SLAVE,
		.name = "packets_per_slave",
		.desc = "Packets to send per slave in RR mode",
		.unsuppmodes = BOND_MODE_ALL_EX(BIT(BOND_MODE_ROUNDROBIN)),
		.values = bond_pps_tbl,
		.set = bond_option_pps_set
	},
	[BOND_OPT_XMIT_HASH] = {
		.id = BOND_OPT_XMIT_HASH,
		.name = "xmit_hash_policy",
		.desc = "balance-xor and 802.3ad hashing method",
		.values = bond_xmit_hashtype_tbl,
		.set = bond_option_xmit_hash_policy_set
	},
	[BOND_OPT_ARP_VALIDATE] = {
		.id = BOND_OPT_ARP_VALIDATE,
		.name = "arp_validate",
		.desc = "validate src/dst of ARP probes",
		.unsuppmodes = BOND_MODE_ALL_EX(BIT(BOND_MODE_ACTIVEBACKUP)),
		.values = bond_arp_validate_tbl,
		.set = bond_option_arp_validate_set
	},
	[BOND_OPT_ARP_ALL_TARGETS] = {
		.id = BOND_OPT_ARP_ALL_TARGETS,
		.name = "arp_all_targets",
		.desc = "fail on any/all arp targets timeout",
		.values = bond_arp_all_targets_tbl,
		.set = bond_option_arp_all_targets_set
	},
	[BOND_OPT_FAIL_OVER_MAC] = {
		.id = BOND_OPT_FAIL_OVER_MAC,
		.name = "fail_over_mac",
		.desc = "For active-backup, do not set all slaves to the same MAC",
		.flags = BOND_OPTFLAG_NOSLAVES,
		.values = bond_fail_over_mac_tbl,
		.set = bond_option_fail_over_mac_set
	},
	[BOND_OPT_ARP_INTERVAL] = {
		.id = BOND_OPT_ARP_INTERVAL,
		.name = "arp_interval",
		.desc = "arp interval in milliseconds",
		.unsuppmodes = BIT(BOND_MODE_8023AD) | BIT(BOND_MODE_TLB) |
			       BIT(BOND_MODE_ALB),
		.values = bond_intmax_tbl,
		.set = bond_option_arp_interval_set
	},
	[BOND_OPT_ARP_TARGETS] = {
		.id = BOND_OPT_ARP_TARGETS,
		.name = "arp_ip_target",
		.desc = "arp targets in n.n.n.n form",
		.flags = BOND_OPTFLAG_RAWVAL,
		.set = bond_option_arp_ip_targets_set
	},
	[BOND_OPT_DOWNDELAY] = {
		.id = BOND_OPT_DOWNDELAY,
		.name = "downdelay",
		.desc = "Delay before considering link down, in milliseconds",
		.values = bond_intmax_tbl,
		.set = bond_option_downdelay_set
	},
	[BOND_OPT_UPDELAY] = {
		.id = BOND_OPT_UPDELAY,
		.name = "updelay",
		.desc = "Delay before considering link up, in milliseconds",
		.values = bond_intmax_tbl,
		.set = bond_option_updelay_set
	},
	[BOND_OPT_LACP_RATE] = {
		.id = BOND_OPT_LACP_RATE,
		.name = "lacp_rate",
		.desc = "LACPDU tx rate to request from 802.3ad partner",
		.flags = BOND_OPTFLAG_IFDOWN,
		.unsuppmodes = BOND_MODE_ALL_EX(BIT(BOND_MODE_8023AD)),
		.values = bond_lacp_rate_tbl,
		.set = bond_option_lacp_rate_set
	},
	[BOND_OPT_MINLINKS] = {
		.id = BOND_OPT_MINLINKS,
		.name = "min_links",
		.desc = "Minimum number of available links before turning on carrier",
		.values = bond_intmax_tbl,
		.set = bond_option_min_links_set
	},
	[BOND_OPT_AD_SELECT] = {
		.id = BOND_OPT_AD_SELECT,
		.name = "ad_select",
		.desc = "803.ad aggregation selection logic",
		.flags = BOND_OPTFLAG_IFDOWN,
		.values = bond_ad_select_tbl,
		.set = bond_option_ad_select_set
	},
	[BOND_OPT_NUM_PEER_NOTIF] = {
		.id = BOND_OPT_NUM_PEER_NOTIF,
		.name = "num_unsol_na",
		.desc = "Number of peer notifications to send on failover event",
		.values = bond_num_peer_notif_tbl,
		.set = bond_option_num_peer_notif_set
	},
	[BOND_OPT_MIIMON] = {
		.id = BOND_OPT_MIIMON,
		.name = "miimon",
		.desc = "Link check interval in milliseconds",
		.values = bond_intmax_tbl,
		.set = bond_option_miimon_set
	},
	[BOND_OPT_PRIMARY] = {
		.id = BOND_OPT_PRIMARY,
		.name = "primary",
		.desc = "Primary network device to use",
		.flags = BOND_OPTFLAG_RAWVAL,
		.unsuppmodes = BOND_MODE_ALL_EX(BIT(BOND_MODE_ACTIVEBACKUP) |
						BIT(BOND_MODE_TLB) |
						BIT(BOND_MODE_ALB)),
		.set = bond_option_primary_set
	},
	[BOND_OPT_PRIMARY_RESELECT] = {
		.id = BOND_OPT_PRIMARY_RESELECT,
		.name = "primary_reselect",
		.desc = "Reselect primary slave once it comes up",
		.values = bond_primary_reselect_tbl,
		.set = bond_option_primary_reselect_set
	},
	[BOND_OPT_USE_CARRIER] = {
		.id = BOND_OPT_USE_CARRIER,
		.name = "use_carrier",
		.desc = "Use netif_carrier_ok (vs MII ioctls) in miimon",
		.values = bond_use_carrier_tbl,
		.set = bond_option_use_carrier_set
	},
	{ }
};

/* Searches for a value in opt's values[] table */
struct bond_opt_value *bond_opt_get_val(unsigned int option, u64 val)
{
	struct bond_option *opt;
	int i;

	opt = bond_opt_get(option);
	if (WARN_ON(!opt))
		return NULL;
	for (i = 0; opt->values && opt->values[i].string; i++)
		if (opt->values[i].value == val)
			return &opt->values[i];

	return NULL;
}

/* Searches for a value in opt's values[] table which matches the flagmask */
static struct bond_opt_value *bond_opt_get_flags(const struct bond_option *opt,
						 u32 flagmask)
{
	int i;

	for (i = 0; opt->values && opt->values[i].string; i++)
		if (opt->values[i].flags & flagmask)
			return &opt->values[i];

	return NULL;
}

/* If maxval is missing then there's no range to check. In case minval is
 * missing then it's considered to be 0.
 */
static bool bond_opt_check_range(const struct bond_option *opt, u64 val)
{
	struct bond_opt_value *minval, *maxval;

	minval = bond_opt_get_flags(opt, BOND_VALFLAG_MIN);
	maxval = bond_opt_get_flags(opt, BOND_VALFLAG_MAX);
	if (!maxval || (minval && val < minval->value) || val > maxval->value)
		return false;

	return true;
}

/**
 * bond_opt_parse - parse option value
 * @opt: the option to parse against
 * @val: value to parse
 *
 * This function tries to extract the value from @val and check if it's
 * a possible match for the option and returns NULL if a match isn't found,
 * or the struct_opt_value that matched. It also strips the new line from
 * @val->string if it's present.
 */
struct bond_opt_value *bond_opt_parse(const struct bond_option *opt,
				      struct bond_opt_value *val)
{
	char *p, valstr[BOND_OPT_MAX_NAMELEN + 1] = { 0, };
	struct bond_opt_value *tbl, *ret = NULL;
	bool checkval;
	int i, rv;

	/* No parsing if the option wants a raw val */
	if (opt->flags & BOND_OPTFLAG_RAWVAL)
		return val;

	tbl = opt->values;
	if (!tbl)
		goto out;

	/* ULLONG_MAX is used to bypass string processing */
	checkval = val->value != ULLONG_MAX;
	if (!checkval) {
		if (!val->string)
			goto out;
		p = strchr(val->string, '\n');
		if (p)
			*p = '\0';
		for (p = val->string; *p; p++)
			if (!(isdigit(*p) || isspace(*p)))
				break;
		/* The following code extracts the string to match or the value
		 * and sets checkval appropriately
		 */
		if (*p) {
			rv = sscanf(val->string, "%32s", valstr);
		} else {
			rv = sscanf(val->string, "%llu", &val->value);
			checkval = true;
		}
		if (!rv)
			goto out;
	}

	for (i = 0; tbl[i].string; i++) {
		/* Check for exact match */
		if (checkval) {
			if (val->value == tbl[i].value)
				ret = &tbl[i];
		} else {
			if (!strcmp(valstr, "default") &&
			    (tbl[i].flags & BOND_VALFLAG_DEFAULT))
				ret = &tbl[i];

			if (!strcmp(valstr, tbl[i].string))
				ret = &tbl[i];
		}
		/* Found an exact match */
		if (ret)
			goto out;
	}
	/* Possible range match */
	if (checkval && bond_opt_check_range(opt, val->value))
		ret = val;
out:
	return ret;
}

/* Check opt's dependencies against bond mode and currently set options */
static int bond_opt_check_deps(struct bonding *bond,
			       const struct bond_option *opt)
{
	struct bond_params *params = &bond->params;

	if (test_bit(params->mode, &opt->unsuppmodes))
		return -EACCES;
	if ((opt->flags & BOND_OPTFLAG_NOSLAVES) && bond_has_slaves(bond))
		return -ENOTEMPTY;
	if ((opt->flags & BOND_OPTFLAG_IFDOWN) && (bond->dev->flags & IFF_UP))
		return -EBUSY;

	return 0;
}

static void bond_opt_dep_print(struct bonding *bond,
			       const struct bond_option *opt)
{
	struct bond_opt_value *modeval;
	struct bond_params *params;

	params = &bond->params;
	modeval = bond_opt_get_val(BOND_OPT_MODE, params->mode);
	if (test_bit(params->mode, &opt->unsuppmodes))
		pr_err("%s: option %s: mode dependency failed, not supported in mode %s(%llu)\n",
		       bond->dev->name, opt->name,
		       modeval->string, modeval->value);
}

static void bond_opt_error_interpret(struct bonding *bond,
				     const struct bond_option *opt,
				     int error, struct bond_opt_value *val)
{
	struct bond_opt_value *minval, *maxval;
	char *p;

	switch (error) {
	case -EINVAL:
		if (val) {
			if (val->string) {
				/* sometimes RAWVAL opts may have new lines */
				p = strchr(val->string, '\n');
				if (p)
					*p = '\0';
				pr_err("%s: option %s: invalid value (%s).\n",
				       bond->dev->name, opt->name, val->string);
			} else {
				pr_err("%s: option %s: invalid value (%llu).\n",
				       bond->dev->name, opt->name, val->value);
			}
		}
		minval = bond_opt_get_flags(opt, BOND_VALFLAG_MIN);
		maxval = bond_opt_get_flags(opt, BOND_VALFLAG_MAX);
		if (!maxval)
			break;
		pr_err("%s: option %s: allowed values %llu - %llu.\n",
		       bond->dev->name, opt->name, minval ? minval->value : 0,
		       maxval->value);
		break;
	case -EACCES:
		bond_opt_dep_print(bond, opt);
		break;
	case -ENOTEMPTY:
		pr_err("%s: option %s: unable to set because the bond device has slaves.\n",
		       bond->dev->name, opt->name);
		break;
	case -EBUSY:
		pr_err("%s: option %s: unable to set because the bond device is up.\n",
		       bond->dev->name, opt->name);
		break;
	default:
		break;
	}
}

/**
 * __bond_opt_set - set a bonding option
 * @bond: target bond device
 * @option: option to set
 * @val: value to set it to
 *
 * This function is used to change the bond's option value, it can be
 * used for both enabling/changing an option and for disabling it. RTNL lock
 * must be obtained before calling this function.
 */
int __bond_opt_set(struct bonding *bond,
		   unsigned int option, struct bond_opt_value *val)
{
	struct bond_opt_value *retval = NULL;
	const struct bond_option *opt;
	int ret = -ENOENT;

	ASSERT_RTNL();

	opt = bond_opt_get(option);
	if (WARN_ON(!val) || WARN_ON(!opt))
		goto out;
	ret = bond_opt_check_deps(bond, opt);
	if (ret)
		goto out;
	retval = bond_opt_parse(opt, val);
	if (!retval) {
		ret = -EINVAL;
		goto out;
	}
	ret = opt->set(bond, retval);
out:
	if (ret)
		bond_opt_error_interpret(bond, opt, ret, val);

	return ret;
}

/**
 * bond_opt_tryset_rtnl - try to acquire rtnl and call __bond_opt_set
 * @bond: target bond device
 * @option: option to set
 * @buf: value to set it to
 *
 * This function tries to acquire RTNL without blocking and if successful
 * calls __bond_opt_set. It is mainly used for sysfs option manipulation.
 */
int bond_opt_tryset_rtnl(struct bonding *bond, unsigned int option, char *buf)
{
	struct bond_opt_value optval;
	int ret;

	if (!rtnl_trylock())
		return restart_syscall();
	bond_opt_initstr(&optval, buf);
	ret = __bond_opt_set(bond, option, &optval);
	rtnl_unlock();

	return ret;
}

/**
 * bond_opt_get - get a pointer to an option
 * @option: option for which to return a pointer
 *
 * This function checks if option is valid and if so returns a pointer
 * to its entry in the bond_opts[] option array.
 */
struct bond_option *bond_opt_get(unsigned int option)
{
	if (!BOND_OPT_VALID(option))
		return NULL;

	return &bond_opts[option];
}

int bond_option_mode_set(struct bonding *bond, struct bond_opt_value *newval)
{
	if (BOND_NO_USES_ARP(newval->value) && bond->params.arp_interval) {
		pr_info("%s: %s mode is incompatible with arp monitoring, start mii monitoring\n",
			bond->dev->name, newval->string);
		/* disable arp monitoring */
		bond->params.arp_interval = 0;
		/* set miimon to default value */
		bond->params.miimon = BOND_DEFAULT_MIIMON;
		pr_info("%s: Setting MII monitoring interval to %d.\n",
			bond->dev->name, bond->params.miimon);
	}

	/* don't cache arp_validate between modes */
	bond->params.arp_validate = BOND_ARP_VALIDATE_NONE;
	bond->params.mode = newval->value;

	return 0;
}

static struct net_device *__bond_option_active_slave_get(struct bonding *bond,
							 struct slave *slave)
{
	return USES_PRIMARY(bond->params.mode) && slave ? slave->dev : NULL;
}

struct net_device *bond_option_active_slave_get_rcu(struct bonding *bond)
{
	struct slave *slave = rcu_dereference(bond->curr_active_slave);

	return __bond_option_active_slave_get(bond, slave);
}

struct net_device *bond_option_active_slave_get(struct bonding *bond)
{
	return __bond_option_active_slave_get(bond, bond->curr_active_slave);
}

int bond_option_active_slave_set(struct bonding *bond,
				 struct net_device *slave_dev)
{
	int ret = 0;

	if (slave_dev) {
		if (!netif_is_bond_slave(slave_dev)) {
			pr_err("Device %s is not bonding slave.\n",
			       slave_dev->name);
			return -EINVAL;
		}

		if (bond->dev != netdev_master_upper_dev_get(slave_dev)) {
			pr_err("%s: Device %s is not our slave.\n",
			       bond->dev->name, slave_dev->name);
			return -EINVAL;
		}
	}

	if (!USES_PRIMARY(bond->params.mode)) {
		pr_err("%s: Unable to change active slave; %s is in mode %d\n",
		       bond->dev->name, bond->dev->name, bond->params.mode);
		return -EINVAL;
	}

	block_netpoll_tx();
	write_lock_bh(&bond->curr_slave_lock);

	/* check to see if we are clearing active */
	if (!slave_dev) {
		pr_info("%s: Clearing current active slave.\n",
		bond->dev->name);
		rcu_assign_pointer(bond->curr_active_slave, NULL);
		bond_select_active_slave(bond);
	} else {
		struct slave *old_active = bond->curr_active_slave;
		struct slave *new_active = bond_slave_get_rtnl(slave_dev);

		BUG_ON(!new_active);

		if (new_active == old_active) {
			/* do nothing */
			pr_info("%s: %s is already the current active slave.\n",
				bond->dev->name, new_active->dev->name);
		} else {
			if (old_active && (new_active->link == BOND_LINK_UP) &&
			    IS_UP(new_active->dev)) {
				pr_info("%s: Setting %s as active slave.\n",
					bond->dev->name, new_active->dev->name);
				bond_change_active_slave(bond, new_active);
			} else {
				pr_err("%s: Could not set %s as active slave; either %s is down or the link is down.\n",
				       bond->dev->name, new_active->dev->name,
				       new_active->dev->name);
				ret = -EINVAL;
			}
		}
	}

	write_unlock_bh(&bond->curr_slave_lock);
	unblock_netpoll_tx();
	return ret;
}

int bond_option_miimon_set(struct bonding *bond, struct bond_opt_value *newval)
{
	pr_info("%s: Setting MII monitoring interval to %llu.\n",
		bond->dev->name, newval->value);
	bond->params.miimon = newval->value;
	if (bond->params.updelay)
		pr_info("%s: Note: Updating updelay (to %d) since it is a multiple of the miimon value.\n",
			bond->dev->name,
			bond->params.updelay * bond->params.miimon);
	if (bond->params.downdelay)
		pr_info("%s: Note: Updating downdelay (to %d) since it is a multiple of the miimon value.\n",
			bond->dev->name,
			bond->params.downdelay * bond->params.miimon);
	if (newval->value && bond->params.arp_interval) {
		pr_info("%s: MII monitoring cannot be used with ARP monitoring. Disabling ARP monitoring...\n",
			bond->dev->name);
		bond->params.arp_interval = 0;
		if (bond->params.arp_validate)
			bond->params.arp_validate = BOND_ARP_VALIDATE_NONE;
	}
	if (bond->dev->flags & IFF_UP) {
		/* If the interface is up, we may need to fire off
		 * the MII timer. If the interface is down, the
		 * timer will get fired off when the open function
		 * is called.
		 */
		if (!newval->value) {
			cancel_delayed_work_sync(&bond->mii_work);
		} else {
			cancel_delayed_work_sync(&bond->arp_work);
			queue_delayed_work(bond->wq, &bond->mii_work, 0);
		}
	}

	return 0;
}

int bond_option_updelay_set(struct bonding *bond, struct bond_opt_value *newval)
{
	if (!bond->params.miimon) {
		pr_err("%s: Unable to set up delay as MII monitoring is disabled\n",
		       bond->dev->name);
		return -EPERM;
	}
	if ((newval->value % bond->params.miimon) != 0) {
		pr_warn("%s: Warning: up delay (%llu) is not a multiple of miimon (%d), updelay rounded to %llu ms\n",
			bond->dev->name, newval->value,
			bond->params.miimon,
			(newval->value / bond->params.miimon) *
			bond->params.miimon);
	}
	bond->params.updelay = newval->value / bond->params.miimon;
	pr_info("%s: Setting up delay to %d.\n",
		bond->dev->name,
		bond->params.updelay * bond->params.miimon);

	return 0;
}

int bond_option_downdelay_set(struct bonding *bond,
			      struct bond_opt_value *newval)
{
	if (!bond->params.miimon) {
		pr_err("%s: Unable to set down delay as MII monitoring is disabled\n",
		       bond->dev->name);
		return -EPERM;
	}
	if ((newval->value % bond->params.miimon) != 0) {
		pr_warn("%s: Warning: down delay (%llu) is not a multiple of miimon (%d), delay rounded to %llu ms\n",
			bond->dev->name, newval->value,
			bond->params.miimon,
			(newval->value / bond->params.miimon) *
			bond->params.miimon);
	}
	bond->params.downdelay = newval->value / bond->params.miimon;
	pr_info("%s: Setting down delay to %d.\n",
		bond->dev->name,
		bond->params.downdelay * bond->params.miimon);

	return 0;
}

int bond_option_use_carrier_set(struct bonding *bond,
				struct bond_opt_value *newval)
{
	pr_info("%s: Setting use_carrier to %llu.\n",
		bond->dev->name, newval->value);
	bond->params.use_carrier = newval->value;

	return 0;
}

int bond_option_arp_interval_set(struct bonding *bond,
				 struct bond_opt_value *newval)
{
	pr_info("%s: Setting ARP monitoring interval to %llu.\n",
		bond->dev->name, newval->value);
	bond->params.arp_interval = newval->value;
	if (newval->value) {
		if (bond->params.miimon) {
			pr_info("%s: ARP monitoring cannot be used with MII monitoring. %s Disabling MII monitoring.\n",
				bond->dev->name, bond->dev->name);
			bond->params.miimon = 0;
		}
		if (!bond->params.arp_targets[0])
			pr_info("%s: ARP monitoring has been set up, but no ARP targets have been specified.\n",
				bond->dev->name);
	}
	if (bond->dev->flags & IFF_UP) {
		/* If the interface is up, we may need to fire off
		 * the ARP timer.  If the interface is down, the
		 * timer will get fired off when the open function
		 * is called.
		 */
		if (!newval->value) {
			if (bond->params.arp_validate)
				bond->recv_probe = NULL;
			cancel_delayed_work_sync(&bond->arp_work);
		} else {
			/* arp_validate can be set only in active-backup mode */
			if (bond->params.arp_validate)
				bond->recv_probe = bond_arp_rcv;
			cancel_delayed_work_sync(&bond->mii_work);
			queue_delayed_work(bond->wq, &bond->arp_work, 0);
		}
	}

	return 0;
}

static void _bond_options_arp_ip_target_set(struct bonding *bond, int slot,
					    __be32 target,
					    unsigned long last_rx)
{
	__be32 *targets = bond->params.arp_targets;
	struct list_head *iter;
	struct slave *slave;

	if (slot >= 0 && slot < BOND_MAX_ARP_TARGETS) {
		bond_for_each_slave(bond, slave, iter)
			slave->target_last_arp_rx[slot] = last_rx;
		targets[slot] = target;
	}
}

static int _bond_option_arp_ip_target_add(struct bonding *bond, __be32 target)
{
	__be32 *targets = bond->params.arp_targets;
	int ind;

	if (IS_IP_TARGET_UNUSABLE_ADDRESS(target)) {
		pr_err("%s: invalid ARP target %pI4 specified for addition\n",
		       bond->dev->name, &target);
		return -EINVAL;
	}

	if (bond_get_targets_ip(targets, target) != -1) { /* dup */
		pr_err("%s: ARP target %pI4 is already present\n",
		       bond->dev->name, &target);
		return -EINVAL;
	}

	ind = bond_get_targets_ip(targets, 0); /* first free slot */
	if (ind == -1) {
		pr_err("%s: ARP target table is full!\n",
		       bond->dev->name);
		return -EINVAL;
	}

	pr_info("%s: adding ARP target %pI4.\n", bond->dev->name, &target);

	_bond_options_arp_ip_target_set(bond, ind, target, jiffies);

	return 0;
}

int bond_option_arp_ip_target_add(struct bonding *bond, __be32 target)
{
	int ret;

	/* not to race with bond_arp_rcv */
	write_lock_bh(&bond->lock);
	ret = _bond_option_arp_ip_target_add(bond, target);
	write_unlock_bh(&bond->lock);

	return ret;
}

int bond_option_arp_ip_target_rem(struct bonding *bond, __be32 target)
{
	__be32 *targets = bond->params.arp_targets;
	struct list_head *iter;
	struct slave *slave;
	unsigned long *targets_rx;
	int ind, i;

	if (IS_IP_TARGET_UNUSABLE_ADDRESS(target)) {
		pr_err("%s: invalid ARP target %pI4 specified for removal\n",
		       bond->dev->name, &target);
		return -EINVAL;
	}

	ind = bond_get_targets_ip(targets, target);
	if (ind == -1) {
		pr_err("%s: unable to remove nonexistent ARP target %pI4.\n",
		       bond->dev->name, &target);
		return -EINVAL;
	}

	if (ind == 0 && !targets[1] && bond->params.arp_interval)
		pr_warn("%s: removing last arp target with arp_interval on\n",
			bond->dev->name);

	pr_info("%s: removing ARP target %pI4.\n", bond->dev->name,
		&target);

	/* not to race with bond_arp_rcv */
	write_lock_bh(&bond->lock);

	bond_for_each_slave(bond, slave, iter) {
		targets_rx = slave->target_last_arp_rx;
		for (i = ind; (i < BOND_MAX_ARP_TARGETS-1) && targets[i+1]; i++)
			targets_rx[i] = targets_rx[i+1];
		targets_rx[i] = 0;
	}
	for (i = ind; (i < BOND_MAX_ARP_TARGETS-1) && targets[i+1]; i++)
		targets[i] = targets[i+1];
	targets[i] = 0;

	write_unlock_bh(&bond->lock);

	return 0;
}

void bond_option_arp_ip_targets_clear(struct bonding *bond)
{
	int i;

	/* not to race with bond_arp_rcv */
	write_lock_bh(&bond->lock);
	for (i = 0; i < BOND_MAX_ARP_TARGETS; i++)
		_bond_options_arp_ip_target_set(bond, i, 0, 0);
	write_unlock_bh(&bond->lock);
}

int bond_option_arp_ip_targets_set(struct bonding *bond,
				   struct bond_opt_value *newval)
{
	int ret = -EPERM;
	__be32 target;

	if (newval->string) {
		if (!in4_pton(newval->string+1, -1, (u8 *)&target, -1, NULL)) {
			pr_err("%s: invalid ARP target %pI4 specified\n",
			       bond->dev->name, &target);
			return ret;
		}
		if (newval->string[0] == '+')
			ret = bond_option_arp_ip_target_add(bond, target);
		else if (newval->string[0] == '-')
			ret = bond_option_arp_ip_target_rem(bond, target);
		else
			pr_err("no command found in arp_ip_targets file for bond %s. Use +<addr> or -<addr>.\n",
			       bond->dev->name);
	} else {
		target = newval->value;
		ret = bond_option_arp_ip_target_add(bond, target);
	}

	return ret;
}

int bond_option_arp_validate_set(struct bonding *bond,
				 struct bond_opt_value *newval)
{
	pr_info("%s: setting arp_validate to %s (%llu).\n",
		bond->dev->name, newval->string, newval->value);

	if (bond->dev->flags & IFF_UP) {
		if (!newval->value)
			bond->recv_probe = NULL;
		else if (bond->params.arp_interval)
			bond->recv_probe = bond_arp_rcv;
	}
	bond->params.arp_validate = newval->value;

	return 0;
}

int bond_option_arp_all_targets_set(struct bonding *bond,
				    struct bond_opt_value *newval)
{
	pr_info("%s: setting arp_all_targets to %s (%llu).\n",
		bond->dev->name, newval->string, newval->value);
	bond->params.arp_all_targets = newval->value;

	return 0;
}

int bond_option_primary_set(struct bonding *bond, struct bond_opt_value *newval)
{
	char *p, *primary = newval->string;
	struct list_head *iter;
	struct slave *slave;

	block_netpoll_tx();
	read_lock(&bond->lock);
	write_lock_bh(&bond->curr_slave_lock);

	p = strchr(primary, '\n');
	if (p)
		*p = '\0';
	/* check to see if we are clearing primary */
	if (!strlen(primary)) {
		pr_info("%s: Setting primary slave to None.\n",
			bond->dev->name);
		bond->primary_slave = NULL;
		memset(bond->params.primary, 0, sizeof(bond->params.primary));
		bond_select_active_slave(bond);
		goto out;
	}

	bond_for_each_slave(bond, slave, iter) {
		if (strncmp(slave->dev->name, primary, IFNAMSIZ) == 0) {
			pr_info("%s: Setting %s as primary slave.\n",
				bond->dev->name, slave->dev->name);
			bond->primary_slave = slave;
			strcpy(bond->params.primary, slave->dev->name);
			bond_select_active_slave(bond);
			goto out;
		}
	}

	strncpy(bond->params.primary, primary, IFNAMSIZ);
	bond->params.primary[IFNAMSIZ - 1] = 0;

	pr_info("%s: Recording %s as primary, but it has not been enslaved to %s yet.\n",
		bond->dev->name, primary, bond->dev->name);

out:
	write_unlock_bh(&bond->curr_slave_lock);
	read_unlock(&bond->lock);
	unblock_netpoll_tx();

	return 0;
}

int bond_option_primary_reselect_set(struct bonding *bond,
				     struct bond_opt_value *newval)
{
	pr_info("%s: setting primary_reselect to %s (%llu).\n",
		bond->dev->name, newval->string, newval->value);
	bond->params.primary_reselect = newval->value;

	block_netpoll_tx();
	write_lock_bh(&bond->curr_slave_lock);
	bond_select_active_slave(bond);
	write_unlock_bh(&bond->curr_slave_lock);
	unblock_netpoll_tx();

	return 0;
}

int bond_option_fail_over_mac_set(struct bonding *bond,
				  struct bond_opt_value *newval)
{
	pr_info("%s: Setting fail_over_mac to %s (%llu).\n",
		bond->dev->name, newval->string, newval->value);
	bond->params.fail_over_mac = newval->value;

	return 0;
}

int bond_option_xmit_hash_policy_set(struct bonding *bond,
				     struct bond_opt_value *newval)
{
	pr_info("%s: setting xmit hash policy to %s (%llu).\n",
		bond->dev->name, newval->string, newval->value);
	bond->params.xmit_policy = newval->value;

	return 0;
}

int bond_option_resend_igmp_set(struct bonding *bond, int resend_igmp)
{
	if (resend_igmp < 0 || resend_igmp > 255) {
		pr_err("%s: Invalid resend_igmp value %d not in range 0-255; rejected.\n",
		       bond->dev->name, resend_igmp);
		return -EINVAL;
	}

	bond->params.resend_igmp = resend_igmp;
	pr_info("%s: Setting resend_igmp to %d.\n",
		bond->dev->name, resend_igmp);

	return 0;
}

int bond_option_num_peer_notif_set(struct bonding *bond,
				   struct bond_opt_value *newval)
{
	bond->params.num_peer_notif = newval->value;

	return 0;
}

int bond_option_all_slaves_active_set(struct bonding *bond,
				      int all_slaves_active)
{
	struct list_head *iter;
	struct slave *slave;

	if (all_slaves_active == bond->params.all_slaves_active)
		return 0;

	if ((all_slaves_active == 0) || (all_slaves_active == 1)) {
		bond->params.all_slaves_active = all_slaves_active;
	} else {
		pr_info("%s: Ignoring invalid all_slaves_active value %d.\n",
			bond->dev->name, all_slaves_active);
		return -EINVAL;
	}

	bond_for_each_slave(bond, slave, iter) {
		if (!bond_is_active_slave(slave)) {
			if (all_slaves_active)
				slave->inactive = 0;
			else
				slave->inactive = 1;
		}
	}

	return 0;
}

int bond_option_min_links_set(struct bonding *bond,
			      struct bond_opt_value *newval)
{
	pr_info("%s: Setting min links value to %llu\n",
		bond->dev->name, newval->value);
	bond->params.min_links = newval->value;

	return 0;
}

int bond_option_lp_interval_set(struct bonding *bond, int lp_interval)
{
	if (lp_interval <= 0) {
		pr_err("%s: lp_interval must be between 1 and %d\n",
		       bond->dev->name, INT_MAX);
		return -EINVAL;
	}

	bond->params.lp_interval = lp_interval;

	return 0;
}

int bond_option_pps_set(struct bonding *bond, struct bond_opt_value *newval)
{
	bond->params.packets_per_slave = newval->value;
	if (newval->value > 0) {
		bond->params.reciprocal_packets_per_slave =
			reciprocal_value(newval->value);
	} else {
		/* reciprocal_packets_per_slave is unused if
		 * packets_per_slave is 0 or 1, just initialize it
		 */
		bond->params.reciprocal_packets_per_slave =
			(struct reciprocal_value) { 0 };
	}

	return 0;
}

int bond_option_lacp_rate_set(struct bonding *bond,
			      struct bond_opt_value *newval)
{
	pr_info("%s: Setting LACP rate to %s (%llu).\n",
		bond->dev->name, newval->string, newval->value);
	bond->params.lacp_fast = newval->value;
	bond_3ad_update_lacp_rate(bond);

	return 0;
}

int bond_option_ad_select_set(struct bonding *bond,
			      struct bond_opt_value *newval)
{
	pr_info("%s: Setting ad_select to %s (%llu).\n",
		bond->dev->name, newval->string, newval->value);
	bond->params.ad_select = newval->value;

	return 0;
}
