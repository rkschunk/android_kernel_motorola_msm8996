/*
 *  include/linux/nfs4.h
 *
 *  NFSv4 protocol definitions.
 *
 *  Copyright (c) 2002 The Regents of the University of Michigan.
 *  All rights reserved.
 *
 *  Kendrick Smith <kmsmith@umich.edu>
 *  Andy Adamson   <andros@umich.edu>
 */

#ifndef _LINUX_NFS4_H
#define _LINUX_NFS4_H

#include <linux/types.h>

#define NFS4_BITMAP_SIZE	2
#define NFS4_VERIFIER_SIZE	8
#define NFS4_STATEID_SEQID_SIZE 4
#define NFS4_STATEID_OTHER_SIZE 12
#define NFS4_STATEID_SIZE	(NFS4_STATEID_SEQID_SIZE + NFS4_STATEID_OTHER_SIZE)
#define NFS4_FHSIZE		128
#define NFS4_MAXPATHLEN		PATH_MAX
#define NFS4_MAXNAMLEN		NAME_MAX
#define NFS4_OPAQUE_LIMIT	1024
#define NFS4_MAX_SESSIONID_LEN	16

#define NFS4_ACCESS_READ        0x0001
#define NFS4_ACCESS_LOOKUP      0x0002
#define NFS4_ACCESS_MODIFY      0x0004
#define NFS4_ACCESS_EXTEND      0x0008
#define NFS4_ACCESS_DELETE      0x0010
#define NFS4_ACCESS_EXECUTE     0x0020

#define NFS4_FH_PERSISTENT		0x0000
#define NFS4_FH_NOEXPIRE_WITH_OPEN	0x0001
#define NFS4_FH_VOLATILE_ANY		0x0002
#define NFS4_FH_VOL_MIGRATION		0x0004
#define NFS4_FH_VOL_RENAME		0x0008

#define NFS4_OPEN_RESULT_CONFIRM 0x0002
#define NFS4_OPEN_RESULT_LOCKTYPE_POSIX 0x0004

#define NFS4_SHARE_ACCESS_MASK	0x000F
#define NFS4_SHARE_ACCESS_READ	0x0001
#define NFS4_SHARE_ACCESS_WRITE	0x0002
#define NFS4_SHARE_ACCESS_BOTH	0x0003
#define NFS4_SHARE_DENY_READ	0x0001
#define NFS4_SHARE_DENY_WRITE	0x0002
#define NFS4_SHARE_DENY_BOTH	0x0003

/* nfs41 */
#define NFS4_SHARE_WANT_MASK		0xFF00
#define NFS4_SHARE_WANT_NO_PREFERENCE	0x0000
#define NFS4_SHARE_WANT_READ_DELEG	0x0100
#define NFS4_SHARE_WANT_WRITE_DELEG	0x0200
#define NFS4_SHARE_WANT_ANY_DELEG	0x0300
#define NFS4_SHARE_WANT_NO_DELEG	0x0400
#define NFS4_SHARE_WANT_CANCEL		0x0500

#define NFS4_SHARE_WHEN_MASK		0xF0000
#define NFS4_SHARE_SIGNAL_DELEG_WHEN_RESRC_AVAIL	0x10000
#define NFS4_SHARE_PUSH_DELEG_WHEN_UNCONTENDED		0x20000

#define NFS4_CDFC4_FORE	0x1
#define NFS4_CDFC4_BACK 0x2
#define NFS4_CDFC4_BOTH 0x3
#define NFS4_CDFC4_FORE_OR_BOTH 0x3
#define NFS4_CDFC4_BACK_OR_BOTH 0x7

#define NFS4_SET_TO_SERVER_TIME	0
#define NFS4_SET_TO_CLIENT_TIME	1

#define NFS4_ACE_ACCESS_ALLOWED_ACE_TYPE 0
#define NFS4_ACE_ACCESS_DENIED_ACE_TYPE  1
#define NFS4_ACE_SYSTEM_AUDIT_ACE_TYPE   2
#define NFS4_ACE_SYSTEM_ALARM_ACE_TYPE   3

#define ACL4_SUPPORT_ALLOW_ACL 0x01
#define ACL4_SUPPORT_DENY_ACL  0x02
#define ACL4_SUPPORT_AUDIT_ACL 0x04
#define ACL4_SUPPORT_ALARM_ACL 0x08

#define NFS4_ACE_FILE_INHERIT_ACE             0x00000001
#define NFS4_ACE_DIRECTORY_INHERIT_ACE        0x00000002
#define NFS4_ACE_NO_PROPAGATE_INHERIT_ACE     0x00000004
#define NFS4_ACE_INHERIT_ONLY_ACE             0x00000008
#define NFS4_ACE_SUCCESSFUL_ACCESS_ACE_FLAG   0x00000010
#define NFS4_ACE_FAILED_ACCESS_ACE_FLAG       0x00000020
#define NFS4_ACE_IDENTIFIER_GROUP             0x00000040

#define NFS4_ACE_READ_DATA                    0x00000001
#define NFS4_ACE_LIST_DIRECTORY               0x00000001
#define NFS4_ACE_WRITE_DATA                   0x00000002
#define NFS4_ACE_ADD_FILE                     0x00000002
#define NFS4_ACE_APPEND_DATA                  0x00000004
#define NFS4_ACE_ADD_SUBDIRECTORY             0x00000004
#define NFS4_ACE_READ_NAMED_ATTRS             0x00000008
#define NFS4_ACE_WRITE_NAMED_ATTRS            0x00000010
#define NFS4_ACE_EXECUTE                      0x00000020
#define NFS4_ACE_DELETE_CHILD                 0x00000040
#define NFS4_ACE_READ_ATTRIBUTES              0x00000080
#define NFS4_ACE_WRITE_ATTRIBUTES             0x00000100
#define NFS4_ACE_DELETE                       0x00010000
#define NFS4_ACE_READ_ACL                     0x00020000
#define NFS4_ACE_WRITE_ACL                    0x00040000
#define NFS4_ACE_WRITE_OWNER                  0x00080000
#define NFS4_ACE_SYNCHRONIZE                  0x00100000
#define NFS4_ACE_GENERIC_READ                 0x00120081
#define NFS4_ACE_GENERIC_WRITE                0x00160106
#define NFS4_ACE_GENERIC_EXECUTE              0x001200A0
#define NFS4_ACE_MASK_ALL                     0x001F01FF

#define EXCHGID4_FLAG_SUPP_MOVED_REFER		0x00000001
#define EXCHGID4_FLAG_SUPP_MOVED_MIGR		0x00000002
#define EXCHGID4_FLAG_BIND_PRINC_STATEID	0x00000100

#define EXCHGID4_FLAG_USE_NON_PNFS		0x00010000
#define EXCHGID4_FLAG_USE_PNFS_MDS		0x00020000
#define EXCHGID4_FLAG_USE_PNFS_DS		0x00040000
#define EXCHGID4_FLAG_MASK_PNFS			0x00070000

#define EXCHGID4_FLAG_UPD_CONFIRMED_REC_A	0x40000000
#define EXCHGID4_FLAG_CONFIRMED_R		0x80000000
/*
 * Since the validity of these bits depends on whether
 * they're set in the argument or response, have separate
 * invalid flag masks for arg (_A) and resp (_R).
 */
#define EXCHGID4_FLAG_MASK_A			0x40070103
#define EXCHGID4_FLAG_MASK_R			0x80070103

#define SEQ4_STATUS_CB_PATH_DOWN		0x00000001
#define SEQ4_STATUS_CB_GSS_CONTEXTS_EXPIRING	0x00000002
#define SEQ4_STATUS_CB_GSS_CONTEXTS_EXPIRED	0x00000004
#define SEQ4_STATUS_EXPIRED_ALL_STATE_REVOKED	0x00000008
#define SEQ4_STATUS_EXPIRED_SOME_STATE_REVOKED	0x00000010
#define SEQ4_STATUS_ADMIN_STATE_REVOKED		0x00000020
#define SEQ4_STATUS_RECALLABLE_STATE_REVOKED	0x00000040
#define SEQ4_STATUS_LEASE_MOVED			0x00000080
#define SEQ4_STATUS_RESTART_RECLAIM_NEEDED	0x00000100
#define SEQ4_STATUS_CB_PATH_DOWN_SESSION	0x00000200
#define SEQ4_STATUS_BACKCHANNEL_FAULT		0x00000400

#define NFS4_SECINFO_STYLE4_CURRENT_FH	0
#define NFS4_SECINFO_STYLE4_PARENT	1

#define NFS4_MAX_UINT64	(~(u64)0)

/* An NFS4 sessions server must support at least NFS4_MAX_OPS operations.
 * If a compound requires more operations, adjust NFS4_MAX_OPS accordingly.
 */
#define NFS4_MAX_OPS   8

/* Our NFS4 client back channel server only wants the cb_sequene and the
 * actual operation per compound
 */
#define NFS4_MAX_BACK_CHANNEL_OPS 2

enum nfs4_acl_whotype {
	NFS4_ACL_WHO_NAMED = 0,
	NFS4_ACL_WHO_OWNER,
	NFS4_ACL_WHO_GROUP,
	NFS4_ACL_WHO_EVERYONE,
};

#ifdef __KERNEL__
#include <linux/list.h>

struct nfs4_ace {
	uint32_t	type;
	uint32_t	flag;
	uint32_t	access_mask;
	int		whotype;
	uid_t		who;
};

struct nfs4_acl {
	uint32_t	naces;
	struct nfs4_ace	aces[0];
};

typedef struct { char data[NFS4_VERIFIER_SIZE]; } nfs4_verifier;

struct nfs41_stateid {
	__be32 seqid;
	char other[NFS4_STATEID_OTHER_SIZE];
} __attribute__ ((packed));

typedef union {
	char data[NFS4_STATEID_SIZE];
	struct nfs41_stateid stateid;
} nfs4_stateid;

enum nfs_opnum4 {
	OP_ACCESS = 3,
	OP_CLOSE = 4,
	OP_COMMIT = 5,
	OP_CREATE = 6,
	OP_DELEGPURGE = 7,
	OP_DELEGRETURN = 8,
	OP_GETATTR = 9,
	OP_GETFH = 10,
	OP_LINK = 11,
	OP_LOCK = 12,
	OP_LOCKT = 13,
	OP_LOCKU = 14,
	OP_LOOKUP = 15,
	OP_LOOKUPP = 16,
	OP_NVERIFY = 17,
	OP_OPEN = 18,
	OP_OPENATTR = 19,
	OP_OPEN_CONFIRM = 20,
	OP_OPEN_DOWNGRADE = 21,
	OP_PUTFH = 22,
	OP_PUTPUBFH = 23,
	OP_PUTROOTFH = 24,
	OP_READ = 25,
	OP_READDIR = 26,
	OP_READLINK = 27,
	OP_REMOVE = 28,
	OP_RENAME = 29,
	OP_RENEW = 30,
	OP_RESTOREFH = 31,
	OP_SAVEFH = 32,
	OP_SECINFO = 33,
	OP_SETATTR = 34,
	OP_SETCLIENTID = 35,
	OP_SETCLIENTID_CONFIRM = 36,
	OP_VERIFY = 37,
	OP_WRITE = 38,
	OP_RELEASE_LOCKOWNER = 39,

	/* nfs41 */
	OP_BACKCHANNEL_CTL = 40,
	OP_BIND_CONN_TO_SESSION = 41,
	OP_EXCHANGE_ID = 42,
	OP_CREATE_SESSION = 43,
	OP_DESTROY_SESSION = 44,
	OP_FREE_STATEID = 45,
	OP_GET_DIR_DELEGATION = 46,
	OP_GETDEVICEINFO = 47,
	OP_GETDEVICELIST = 48,
	OP_LAYOUTCOMMIT = 49,
	OP_LAYOUTGET = 50,
	OP_LAYOUTRETURN = 51,
	OP_SECINFO_NO_NAME = 52,
	OP_SEQUENCE = 53,
	OP_SET_SSV = 54,
	OP_TEST_STATEID = 55,
	OP_WANT_DELEGATION = 56,
	OP_DESTROY_CLIENTID = 57,
	OP_RECLAIM_COMPLETE = 58,

	OP_ILLEGAL = 10044,
};

/*Defining first and last NFS4 operations implemented.
Needs to be updated if more operations are defined in future.*/

#define FIRST_NFS4_OP	OP_ACCESS
#define LAST_NFS4_OP 	OP_RECLAIM_COMPLETE

enum nfsstat4 {
	NFS4_OK = 0,
	NFS4ERR_PERM = 1,
	NFS4ERR_NOENT = 2,
	NFS4ERR_IO = 5,
	NFS4ERR_NXIO = 6,
	NFS4ERR_ACCESS = 13,
	NFS4ERR_EXIST = 17,
	NFS4ERR_XDEV = 18,
	/* Unused/reserved 19 */
	NFS4ERR_NOTDIR = 20,
	NFS4ERR_ISDIR = 21,
	NFS4ERR_INVAL = 22,
	NFS4ERR_FBIG = 27,
	NFS4ERR_NOSPC = 28,
	NFS4ERR_ROFS = 30,
	NFS4ERR_MLINK = 31,
	NFS4ERR_NAMETOOLONG = 63,
	NFS4ERR_NOTEMPTY = 66,
	NFS4ERR_DQUOT = 69,
	NFS4ERR_STALE = 70,
	NFS4ERR_BADHANDLE = 10001,
	NFS4ERR_BAD_COOKIE = 10003,
	NFS4ERR_NOTSUPP = 10004,
	NFS4ERR_TOOSMALL = 10005,
	NFS4ERR_SERVERFAULT = 10006,
	NFS4ERR_BADTYPE = 10007,
	NFS4ERR_DELAY = 10008,
	NFS4ERR_SAME = 10009,
	NFS4ERR_DENIED = 10010,
	NFS4ERR_EXPIRED = 10011,
	NFS4ERR_LOCKED = 10012,
	NFS4ERR_GRACE = 10013,
	NFS4ERR_FHEXPIRED = 10014,
	NFS4ERR_SHARE_DENIED = 10015,
	NFS4ERR_WRONGSEC = 10016,
	NFS4ERR_CLID_INUSE = 10017,
	NFS4ERR_RESOURCE = 10018,
	NFS4ERR_MOVED = 10019,
	NFS4ERR_NOFILEHANDLE = 10020,
	NFS4ERR_MINOR_VERS_MISMATCH = 10021,
	NFS4ERR_STALE_CLIENTID = 10022,
	NFS4ERR_STALE_STATEID = 10023,
	NFS4ERR_OLD_STATEID = 10024,
	NFS4ERR_BAD_STATEID = 10025,
	NFS4ERR_BAD_SEQID = 10026,
	NFS4ERR_NOT_SAME = 10027,
	NFS4ERR_LOCK_RANGE = 10028,
	NFS4ERR_SYMLINK = 10029,
	NFS4ERR_RESTOREFH = 10030,
	NFS4ERR_LEASE_MOVED = 10031,
	NFS4ERR_ATTRNOTSUPP = 10032,
	NFS4ERR_NO_GRACE = 10033,
	NFS4ERR_RECLAIM_BAD = 10034,
	NFS4ERR_RECLAIM_CONFLICT = 10035,
	NFS4ERR_BADXDR = 10036,
	NFS4ERR_LOCKS_HELD = 10037,
	NFS4ERR_OPENMODE = 10038,
	NFS4ERR_BADOWNER = 10039,
	NFS4ERR_BADCHAR = 10040,
	NFS4ERR_BADNAME = 10041,
	NFS4ERR_BAD_RANGE = 10042,
	NFS4ERR_LOCK_NOTSUPP = 10043,
	NFS4ERR_OP_ILLEGAL = 10044,
	NFS4ERR_DEADLOCK = 10045,
	NFS4ERR_FILE_OPEN = 10046,
	NFS4ERR_ADMIN_REVOKED = 10047,
	NFS4ERR_CB_PATH_DOWN = 10048,

	/* nfs41 */
	NFS4ERR_BADIOMODE	= 10049,
	NFS4ERR_BADLAYOUT	= 10050,
	NFS4ERR_BAD_SESSION_DIGEST = 10051,
	NFS4ERR_BADSESSION	= 10052,
	NFS4ERR_BADSLOT		= 10053,
	NFS4ERR_COMPLETE_ALREADY = 10054,
	NFS4ERR_CONN_NOT_BOUND_TO_SESSION = 10055,
	NFS4ERR_DELEG_ALREADY_WANTED = 10056,
	NFS4ERR_BACK_CHAN_BUSY	= 10057,	/* backchan reqs outstanding */
	NFS4ERR_LAYOUTTRYLATER	= 10058,
	NFS4ERR_LAYOUTUNAVAILABLE = 10059,
	NFS4ERR_NOMATCHING_LAYOUT = 10060,
	NFS4ERR_RECALLCONFLICT	= 10061,
	NFS4ERR_UNKNOWN_LAYOUTTYPE = 10062,
	NFS4ERR_SEQ_MISORDERED = 10063, 	/* unexpected seq.id in req */
	NFS4ERR_SEQUENCE_POS	= 10064,	/* [CB_]SEQ. op not 1st op */
	NFS4ERR_REQ_TOO_BIG	= 10065,	/* request too big */
	NFS4ERR_REP_TOO_BIG	= 10066,	/* reply too big */
	NFS4ERR_REP_TOO_BIG_TO_CACHE = 10067,	/* rep. not all cached */
	NFS4ERR_RETRY_UNCACHED_REP = 10068,	/* retry & rep. uncached */
	NFS4ERR_UNSAFE_COMPOUND = 10069,	/* retry/recovery too hard */
	NFS4ERR_TOO_MANY_OPS	= 10070,	/* too many ops in [CB_]COMP */
	NFS4ERR_OP_NOT_IN_SESSION = 10071,	/* op needs [CB_]SEQ. op */
	NFS4ERR_HASH_ALG_UNSUPP = 10072,	/* hash alg. not supp. */
						/* Error 10073 is unused. */
	NFS4ERR_CLIENTID_BUSY	= 10074,	/* clientid has state */
	NFS4ERR_PNFS_IO_HOLE	= 10075,	/* IO to _SPARSE file hole */
	NFS4ERR_SEQ_FALSE_RETRY	= 10076,	/* retry not original */
	NFS4ERR_BAD_HIGH_SLOT	= 10077,	/* sequence arg bad */
	NFS4ERR_DEADSESSION	= 10078,	/* persistent session dead */
	NFS4ERR_ENCR_ALG_UNSUPP = 10079,	/* SSV alg mismatch */
	NFS4ERR_PNFS_NO_LAYOUT	= 10080,	/* direct I/O with no layout */
	NFS4ERR_NOT_ONLY_OP	= 10081,	/* bad compound */
	NFS4ERR_WRONG_CRED	= 10082,	/* permissions:state change */
	NFS4ERR_WRONG_TYPE	= 10083,	/* current operation mismatch */
	NFS4ERR_DIRDELEG_UNAVAIL = 10084,	/* no directory delegation */
	NFS4ERR_REJECT_DELEG	= 10085,	/* on callback */
	NFS4ERR_RETURNCONFLICT	= 10086,	/* outstanding layoutreturn */
	NFS4ERR_DELEG_REVOKED	= 10087,	/* deleg./layout revoked */
};

/*
 * Note: NF4BAD is not actually part of the protocol; it is just used
 * internally by nfsd.
 */
enum nfs_ftype4 {
	NF4BAD		= 0,
        NF4REG          = 1,    /* Regular File */
        NF4DIR          = 2,    /* Directory */
        NF4BLK          = 3,    /* Special File - block device */
        NF4CHR          = 4,    /* Special File - character device */
        NF4LNK          = 5,    /* Symbolic Link */
        NF4SOCK         = 6,    /* Special File - socket */
        NF4FIFO         = 7,    /* Special File - fifo */
        NF4ATTRDIR      = 8,    /* Attribute Directory */
        NF4NAMEDATTR    = 9     /* Named Attribute */
};

enum open_claim_type4 {
	NFS4_OPEN_CLAIM_NULL = 0,
	NFS4_OPEN_CLAIM_PREVIOUS = 1,
	NFS4_OPEN_CLAIM_DELEGATE_CUR = 2,
	NFS4_OPEN_CLAIM_DELEGATE_PREV = 3
};

enum opentype4 {
	NFS4_OPEN_NOCREATE = 0,
	NFS4_OPEN_CREATE = 1
};

enum createmode4 {
	NFS4_CREATE_UNCHECKED = 0,
	NFS4_CREATE_GUARDED = 1,
	NFS4_CREATE_EXCLUSIVE = 2,
	/*
	 * New to NFSv4.1. If session is persistent,
	 * GUARDED4 MUST be used. Otherwise, use
	 * EXCLUSIVE4_1 instead of EXCLUSIVE4.
	 */
	NFS4_CREATE_EXCLUSIVE4_1 = 3
};

enum limit_by4 {
	NFS4_LIMIT_SIZE = 1,
	NFS4_LIMIT_BLOCKS = 2
};

enum open_delegation_type4 {
	NFS4_OPEN_DELEGATE_NONE = 0,
	NFS4_OPEN_DELEGATE_READ = 1,
	NFS4_OPEN_DELEGATE_WRITE = 2
};

enum lock_type4 {
	NFS4_UNLOCK_LT = 0,
	NFS4_READ_LT = 1,
	NFS4_WRITE_LT = 2,
	NFS4_READW_LT = 3,
	NFS4_WRITEW_LT = 4
};


/* Mandatory Attributes */
#define FATTR4_WORD0_SUPPORTED_ATTRS    (1UL << 0)
#define FATTR4_WORD0_TYPE               (1UL << 1)
#define FATTR4_WORD0_FH_EXPIRE_TYPE     (1UL << 2)
#define FATTR4_WORD0_CHANGE             (1UL << 3)
#define FATTR4_WORD0_SIZE               (1UL << 4)
#define FATTR4_WORD0_LINK_SUPPORT       (1UL << 5)
#define FATTR4_WORD0_SYMLINK_SUPPORT    (1UL << 6)
#define FATTR4_WORD0_NAMED_ATTR         (1UL << 7)
#define FATTR4_WORD0_FSID               (1UL << 8)
#define FATTR4_WORD0_UNIQUE_HANDLES     (1UL << 9)
#define FATTR4_WORD0_LEASE_TIME         (1UL << 10)
#define FATTR4_WORD0_RDATTR_ERROR       (1UL << 11)
/* Mandatory in NFSv4.1 */
#define FATTR4_WORD2_SUPPATTR_EXCLCREAT (1UL << 11)

/* Recommended Attributes */
#define FATTR4_WORD0_ACL                (1UL << 12)
#define FATTR4_WORD0_ACLSUPPORT         (1UL << 13)
#define FATTR4_WORD0_ARCHIVE            (1UL << 14)
#define FATTR4_WORD0_CANSETTIME         (1UL << 15)
#define FATTR4_WORD0_CASE_INSENSITIVE   (1UL << 16)
#define FATTR4_WORD0_CASE_PRESERVING    (1UL << 17)
#define FATTR4_WORD0_CHOWN_RESTRICTED   (1UL << 18)
#define FATTR4_WORD0_FILEHANDLE         (1UL << 19)
#define FATTR4_WORD0_FILEID             (1UL << 20)
#define FATTR4_WORD0_FILES_AVAIL        (1UL << 21)
#define FATTR4_WORD0_FILES_FREE         (1UL << 22)
#define FATTR4_WORD0_FILES_TOTAL        (1UL << 23)
#define FATTR4_WORD0_FS_LOCATIONS       (1UL << 24)
#define FATTR4_WORD0_HIDDEN             (1UL << 25)
#define FATTR4_WORD0_HOMOGENEOUS        (1UL << 26)
#define FATTR4_WORD0_MAXFILESIZE        (1UL << 27)
#define FATTR4_WORD0_MAXLINK            (1UL << 28)
#define FATTR4_WORD0_MAXNAME            (1UL << 29)
#define FATTR4_WORD0_MAXREAD            (1UL << 30)
#define FATTR4_WORD0_MAXWRITE           (1UL << 31)
#define FATTR4_WORD1_MIMETYPE           (1UL << 0)
#define FATTR4_WORD1_MODE               (1UL << 1)
#define FATTR4_WORD1_NO_TRUNC           (1UL << 2)
#define FATTR4_WORD1_NUMLINKS           (1UL << 3)
#define FATTR4_WORD1_OWNER              (1UL << 4)
#define FATTR4_WORD1_OWNER_GROUP        (1UL << 5)
#define FATTR4_WORD1_QUOTA_HARD         (1UL << 6)
#define FATTR4_WORD1_QUOTA_SOFT         (1UL << 7)
#define FATTR4_WORD1_QUOTA_USED         (1UL << 8)
#define FATTR4_WORD1_RAWDEV             (1UL << 9)
#define FATTR4_WORD1_SPACE_AVAIL        (1UL << 10)
#define FATTR4_WORD1_SPACE_FREE         (1UL << 11)
#define FATTR4_WORD1_SPACE_TOTAL        (1UL << 12)
#define FATTR4_WORD1_SPACE_USED         (1UL << 13)
#define FATTR4_WORD1_SYSTEM             (1UL << 14)
#define FATTR4_WORD1_TIME_ACCESS        (1UL << 15)
#define FATTR4_WORD1_TIME_ACCESS_SET    (1UL << 16)
#define FATTR4_WORD1_TIME_BACKUP        (1UL << 17)
#define FATTR4_WORD1_TIME_CREATE        (1UL << 18)
#define FATTR4_WORD1_TIME_DELTA         (1UL << 19)
#define FATTR4_WORD1_TIME_METADATA      (1UL << 20)
#define FATTR4_WORD1_TIME_MODIFY        (1UL << 21)
#define FATTR4_WORD1_TIME_MODIFY_SET    (1UL << 22)
#define FATTR4_WORD1_MOUNTED_ON_FILEID  (1UL << 23)
#define FATTR4_WORD1_FS_LAYOUT_TYPES    (1UL << 30)
#define FATTR4_WORD2_LAYOUT_BLKSIZE     (1UL << 1)

#define NFSPROC4_NULL 0
#define NFSPROC4_COMPOUND 1
#define NFS4_VERSION 4
#define NFS4_MINOR_VERSION 0

#if defined(CONFIG_NFS_V4_1)
#define NFS4_MAX_MINOR_VERSION 1
#else
#define NFS4_MAX_MINOR_VERSION 0
#endif /* CONFIG_NFS_V4_1 */

#define NFS4_DEBUG 1

/* Index of predefined Linux client operations */

enum {
	NFSPROC4_CLNT_NULL = 0,		/* Unused */
	NFSPROC4_CLNT_READ,
	NFSPROC4_CLNT_WRITE,
	NFSPROC4_CLNT_COMMIT,
	NFSPROC4_CLNT_OPEN,
	NFSPROC4_CLNT_OPEN_CONFIRM,
	NFSPROC4_CLNT_OPEN_NOATTR,
	NFSPROC4_CLNT_OPEN_DOWNGRADE,
	NFSPROC4_CLNT_CLOSE,
	NFSPROC4_CLNT_SETATTR,
	NFSPROC4_CLNT_FSINFO,
	NFSPROC4_CLNT_RENEW,
	NFSPROC4_CLNT_SETCLIENTID,
	NFSPROC4_CLNT_SETCLIENTID_CONFIRM,
	NFSPROC4_CLNT_LOCK,
	NFSPROC4_CLNT_LOCKT,
	NFSPROC4_CLNT_LOCKU,
	NFSPROC4_CLNT_ACCESS,
	NFSPROC4_CLNT_GETATTR,
	NFSPROC4_CLNT_LOOKUP,
	NFSPROC4_CLNT_LOOKUP_ROOT,
	NFSPROC4_CLNT_REMOVE,
	NFSPROC4_CLNT_RENAME,
	NFSPROC4_CLNT_LINK,
	NFSPROC4_CLNT_SYMLINK,
	NFSPROC4_CLNT_CREATE,
	NFSPROC4_CLNT_PATHCONF,
	NFSPROC4_CLNT_STATFS,
	NFSPROC4_CLNT_READLINK,
	NFSPROC4_CLNT_READDIR,
	NFSPROC4_CLNT_SERVER_CAPS,
	NFSPROC4_CLNT_DELEGRETURN,
	NFSPROC4_CLNT_GETACL,
	NFSPROC4_CLNT_SETACL,
	NFSPROC4_CLNT_FS_LOCATIONS,
	NFSPROC4_CLNT_RELEASE_LOCKOWNER,
	NFSPROC4_CLNT_SECINFO,

	/* nfs41 */
	NFSPROC4_CLNT_EXCHANGE_ID,
	NFSPROC4_CLNT_CREATE_SESSION,
	NFSPROC4_CLNT_DESTROY_SESSION,
	NFSPROC4_CLNT_SEQUENCE,
	NFSPROC4_CLNT_GET_LEASE_TIME,
	NFSPROC4_CLNT_RECLAIM_COMPLETE,
	NFSPROC4_CLNT_LAYOUTGET,
	NFSPROC4_CLNT_GETDEVICEINFO,
	NFSPROC4_CLNT_LAYOUTCOMMIT,
	NFSPROC4_CLNT_LAYOUTRETURN,
};

/* nfs41 types */
struct nfs4_sessionid {
	unsigned char data[NFS4_MAX_SESSIONID_LEN];
};

/* Create Session Flags */
#define SESSION4_PERSIST	 0x001
#define SESSION4_BACK_CHAN 	 0x002
#define SESSION4_RDMA		 0x004

enum state_protect_how4 {
	SP4_NONE	= 0,
	SP4_MACH_CRED	= 1,
	SP4_SSV		= 2
};

enum pnfs_layouttype {
	LAYOUT_NFSV4_1_FILES  = 1,
	LAYOUT_OSD2_OBJECTS = 2,
	LAYOUT_BLOCK_VOLUME = 3,
};

/* used for both layout return and recall */
enum pnfs_layoutreturn_type {
	RETURN_FILE = 1,
	RETURN_FSID = 2,
	RETURN_ALL  = 3
};

enum pnfs_iomode {
	IOMODE_READ = 1,
	IOMODE_RW = 2,
	IOMODE_ANY = 3,
};

enum pnfs_notify_deviceid_type4 {
	NOTIFY_DEVICEID4_CHANGE = 1 << 1,
	NOTIFY_DEVICEID4_DELETE = 1 << 2,
};

#define NFL4_UFLG_MASK			0x0000003F
#define NFL4_UFLG_DENSE			0x00000001
#define NFL4_UFLG_COMMIT_THRU_MDS	0x00000002
#define NFL4_UFLG_STRIPE_UNIT_SIZE_MASK	0xFFFFFFC0

/* Encoded in the loh_body field of type layouthint4 */
enum filelayout_hint_care4 {
	NFLH4_CARE_DENSE		= NFL4_UFLG_DENSE,
	NFLH4_CARE_COMMIT_THRU_MDS	= NFL4_UFLG_COMMIT_THRU_MDS,
	NFLH4_CARE_STRIPE_UNIT_SIZE	= 0x00000040,
	NFLH4_CARE_STRIPE_COUNT		= 0x00000080
};

#define NFS4_DEVICEID4_SIZE 16

struct nfs4_deviceid {
	char data[NFS4_DEVICEID4_SIZE];
};

#endif
#endif

/*
 * Local variables:
 *  c-basic-offset: 8
 * End:
 */
