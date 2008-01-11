/*******************************************************************
 * This file is part of the Emulex Linux Device Driver for         *
 * Fibre Channel Host Bus Adapters.                                *
 * Copyright (C) 2004-2007 Emulex.  All rights reserved.           *
 * EMULEX and SLI are trademarks of Emulex.                        *
 * www.emulex.com                                                  *
 *                                                                 *
 * This program is free software; you can redistribute it and/or   *
 * modify it under the terms of version 2 of the GNU General       *
 * Public License as published by the Free Software Foundation.    *
 * This program is distributed in the hope that it will be useful. *
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND          *
 * WARRANTIES, INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY,  *
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE      *
 * DISCLAIMED, EXCEPT TO THE EXTENT THAT SUCH DISCLAIMERS ARE HELD *
 * TO BE LEGALLY INVALID.  See the GNU General Public License for  *
 * more details, a copy of which can be found in the file COPYING  *
 * included with this package.                                     *
 *******************************************************************/

/* forward declaration for LPFC_IOCB_t's use */
struct lpfc_hba;
struct lpfc_vport;

/* Define the context types that SLI handles for abort and sums. */
typedef enum _lpfc_ctx_cmd {
	LPFC_CTX_LUN,
	LPFC_CTX_TGT,
	LPFC_CTX_HOST
} lpfc_ctx_cmd;

/* This structure is used to handle IOCB requests / responses */
struct lpfc_iocbq {
	/* lpfc_iocbqs are used in double linked lists */
	struct list_head list;
	uint16_t iotag;         /* pre-assigned IO tag */
	uint16_t rsvd1;

	IOCB_t iocb;		/* IOCB cmd */
	uint8_t retry;		/* retry counter for IOCB cmd - if needed */
	uint8_t iocb_flag;
#define LPFC_IO_LIBDFC		1	/* libdfc iocb */
#define LPFC_IO_WAKE		2	/* High Priority Queue signal flag */
#define LPFC_IO_FCP		4	/* FCP command -- iocbq in scsi_buf */
#define LPFC_DRIVER_ABORTED	8	/* driver aborted this request */
#define LPFC_IO_FABRIC		0x10	/* Iocb send using fabric scheduler */
#define LPFC_DELAY_MEM_FREE	0x20    /* Defer free'ing of FC data */

	uint8_t abort_count;
	uint8_t rsvd2;
	uint32_t drvrTimeout;	/* driver timeout in seconds */
	struct lpfc_vport *vport;/* virtual port pointer */
	void *context1;		/* caller context information */
	void *context2;		/* caller context information */
	void *context3;		/* caller context information */
	union {
		wait_queue_head_t    *wait_queue;
		struct lpfc_iocbq    *rsp_iocb;
		struct lpfcMboxq     *mbox;
		struct lpfc_nodelist *ndlp;
	} context_un;

	void (*fabric_iocb_cmpl) (struct lpfc_hba *, struct lpfc_iocbq *,
			   struct lpfc_iocbq *);
	void (*iocb_cmpl) (struct lpfc_hba *, struct lpfc_iocbq *,
			   struct lpfc_iocbq *);

};

#define SLI_IOCB_RET_IOCB      1	/* Return IOCB if cmd ring full */

#define IOCB_SUCCESS        0
#define IOCB_BUSY           1
#define IOCB_ERROR          2
#define IOCB_TIMEDOUT       3

#define LPFC_MBX_WAKE		1
#define LPFC_MBX_IMED_UNREG	2

typedef struct lpfcMboxq {
	/* MBOXQs are used in single linked lists */
	struct list_head list;	/* ptr to next mailbox command */
	MAILBOX_t mb;		/* Mailbox cmd */
	struct lpfc_vport *vport;/* virutal port pointer */
	void *context1;		/* caller context information */
	void *context2;		/* caller context information */

	void (*mbox_cmpl) (struct lpfc_hba *, struct lpfcMboxq *);
	uint8_t mbox_flag;

} LPFC_MBOXQ_t;

#define MBX_POLL        1	/* poll mailbox till command done, then
				   return */
#define MBX_NOWAIT      2	/* issue command then return immediately */

#define LPFC_MAX_RING_MASK  4	/* max num of rctl/type masks allowed per
				   ring */
#define LPFC_MAX_RING       4	/* max num of SLI rings used by driver */

struct lpfc_sli_ring;

struct lpfc_sli_ring_mask {
	uint8_t profile;	/* profile associated with ring */
	uint8_t rctl;	/* rctl / type pair configured for ring */
	uint8_t type;	/* rctl / type pair configured for ring */
	uint8_t rsvd;
	/* rcv'd unsol event */
	void (*lpfc_sli_rcv_unsol_event) (struct lpfc_hba *,
					 struct lpfc_sli_ring *,
					 struct lpfc_iocbq *);
};


/* Structure used to hold SLI statistical counters and info */
struct lpfc_sli_ring_stat {
	uint64_t iocb_event;	 /* IOCB event counters */
	uint64_t iocb_cmd;	 /* IOCB cmd issued */
	uint64_t iocb_rsp;	 /* IOCB rsp received */
	uint64_t iocb_cmd_delay; /* IOCB cmd ring delay */
	uint64_t iocb_cmd_full;	 /* IOCB cmd ring full */
	uint64_t iocb_cmd_empty; /* IOCB cmd ring is now empty */
	uint64_t iocb_rsp_full;	 /* IOCB rsp ring full */
};

/* Structure used to hold SLI ring information */
struct lpfc_sli_ring {
	uint16_t flag;		/* ring flags */
#define LPFC_DEFERRED_RING_EVENT 0x001	/* Deferred processing a ring event */
#define LPFC_CALL_RING_AVAILABLE 0x002	/* indicates cmd was full */
#define LPFC_STOP_IOCB_EVENT     0x020	/* Stop processing IOCB cmds event */
	uint16_t abtsiotag;	/* tracks next iotag to use for ABTS */

	uint32_t local_getidx;   /* last available cmd index (from cmdGetInx) */
	uint32_t next_cmdidx;    /* next_cmd index */
	uint32_t rspidx;	/* current index in response ring */
	uint32_t cmdidx;	/* current index in command ring */
	uint8_t rsvd;
	uint8_t ringno;		/* ring number */
	uint16_t numCiocb;	/* number of command iocb's per ring */
	uint16_t numRiocb;	/* number of rsp iocb's per ring */
	uint16_t sizeCiocb;	/* Size of command iocb's in this ring */
	uint16_t sizeRiocb;	/* Size of response iocb's in this ring */

	uint32_t fast_iotag;	/* max fastlookup based iotag           */
	uint32_t iotag_ctr;	/* keeps track of the next iotag to use */
	uint32_t iotag_max;	/* max iotag value to use               */
	struct list_head txq;
	uint16_t txq_cnt;	/* current length of queue */
	uint16_t txq_max;	/* max length */
	struct list_head txcmplq;
	uint16_t txcmplq_cnt;	/* current length of queue */
	uint16_t txcmplq_max;	/* max length */
	uint32_t *cmdringaddr;	/* virtual address for cmd rings */
	uint32_t *rspringaddr;	/* virtual address for rsp rings */
	uint32_t missbufcnt;	/* keep track of buffers to post */
	struct list_head postbufq;
	uint16_t postbufq_cnt;	/* current length of queue */
	uint16_t postbufq_max;	/* max length */
	struct list_head iocb_continueq;
	uint16_t iocb_continueq_cnt;	/* current length of queue */
	uint16_t iocb_continueq_max;	/* max length */

	struct lpfc_sli_ring_mask prt[LPFC_MAX_RING_MASK];
	uint32_t num_mask;	/* number of mask entries in prt array */
	void (*lpfc_sli_rcv_async_status) (struct lpfc_hba *,
		struct lpfc_sli_ring *, struct lpfc_iocbq *);

	struct lpfc_sli_ring_stat stats;	/* SLI statistical info */

	/* cmd ring available */
	void (*lpfc_sli_cmd_available) (struct lpfc_hba *,
					struct lpfc_sli_ring *);
};

/* Structure used for configuring rings to a specific profile or rctl / type */
struct lpfc_hbq_init {
	uint32_t rn;		/* Receive buffer notification */
	uint32_t entry_count;	/* max # of entries in HBQ */
	uint32_t headerLen;	/* 0 if not profile 4 or 5 */
	uint32_t logEntry;	/* Set to 1 if this HBQ used for LogEntry */
	uint32_t profile;	/* Selection profile 0=all, 7=logentry */
	uint32_t ring_mask;	/* Binds HBQ to a ring e.g. Ring0=b0001,
				 * ring2=b0100 */
	uint32_t hbq_index;	/* index of this hbq in ring .HBQs[] */

	uint32_t seqlenoff;
	uint32_t maxlen;
	uint32_t seqlenbcnt;
	uint32_t cmdcodeoff;
	uint32_t cmdmatch[8];
	uint32_t mask_count;	/* number of mask entries in prt array */
	struct hbq_mask hbqMasks[6];

	/* Non-config rings fields to keep track of buffer allocations */
	uint32_t buffer_count;	/* number of buffers allocated */
	uint32_t init_count;	/* number to allocate when initialized */
	uint32_t add_count;	/* number to allocate when starved */
} ;

/* Structure used to hold SLI statistical counters and info */
struct lpfc_sli_stat {
	uint64_t mbox_stat_err;  /* Mbox cmds completed status error */
	uint64_t mbox_cmd;       /* Mailbox commands issued */
	uint64_t sli_intr;       /* Count of Host Attention interrupts */
	uint32_t err_attn_event; /* Error Attn event counters */
	uint32_t link_event;     /* Link event counters */
	uint32_t mbox_event;     /* Mailbox event counters */
	uint32_t mbox_busy;	 /* Mailbox cmd busy */
};

/* Structure to store link status values when port stats are reset */
struct lpfc_lnk_stat {
	uint32_t link_failure_count;
	uint32_t loss_of_sync_count;
	uint32_t loss_of_signal_count;
	uint32_t prim_seq_protocol_err_count;
	uint32_t invalid_tx_word_count;
	uint32_t invalid_crc_count;
	uint32_t error_frames;
	uint32_t link_events;
};

/* Structure used to hold SLI information */
struct lpfc_sli {
	uint32_t num_rings;
	uint32_t sli_flag;

	/* Additional sli_flags */
#define LPFC_SLI_MBOX_ACTIVE      0x100	/* HBA mailbox is currently active */
#define LPFC_SLI2_ACTIVE          0x200	/* SLI2 overlay in firmware is active */
#define LPFC_PROCESS_LA           0x400	/* Able to process link attention */
#define LPFC_BLOCK_MGMT_IO        0x800	/* Don't allow mgmt mbx or iocb cmds */

	struct lpfc_sli_ring ring[LPFC_MAX_RING];
	int fcp_ring;		/* ring used for FCP initiator commands */
	int next_ring;

	int extra_ring;		/* extra ring used for other protocols */

	struct lpfc_sli_stat slistat;	/* SLI statistical info */
	struct list_head mboxq;
	uint16_t mboxq_cnt;	/* current length of queue */
	uint16_t mboxq_max;	/* max length */
	LPFC_MBOXQ_t *mbox_active;	/* active mboxq information */
	struct list_head mboxq_cmpl;

	struct timer_list mbox_tmo;	/* Hold clk to timeout active mbox
					   cmd */

#define LPFC_IOCBQ_LOOKUP_INCREMENT  1024
	struct lpfc_iocbq ** iocbq_lookup; /* array to lookup IOCB by IOTAG */
	size_t iocbq_lookup_len;           /* current lengs of the array */
	uint16_t  last_iotag;              /* last allocated IOTAG */
	unsigned long  stats_start;        /* in seconds */
	struct lpfc_lnk_stat lnk_stat_offsets;
};

#define LPFC_MBOX_TMO           30	/* Sec tmo for outstanding mbox
					   command */
#define LPFC_MBOX_TMO_FLASH_CMD 300     /* Sec tmo for outstanding FLASH write
					 * or erase cmds. This is especially
					 * long because of the potential of
					 * multiple flash erases that can be
					 * spawned.
					 */
