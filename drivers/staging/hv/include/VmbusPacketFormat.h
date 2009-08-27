/*
 *
 * Copyright (c) 2009, Microsoft Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Authors:
 *   Haiyang Zhang <haiyangz@microsoft.com>
 *   Hank Janssen  <hjanssen@microsoft.com>
 *
 */

#ifndef _VMBUSPACKETFORMAT_H_

typedef struct {
	u16 Type;
	u16 DataOffset8;
	u16 Length8;
	u16 Flags;
	u64 TransactionId;
} __attribute__((packed)) VMPACKET_DESCRIPTOR, *PVMPACKET_DESCRIPTOR;

typedef u32 PREVIOUS_PACKET_OFFSET, *PPREVIOUS_PACKET_OFFSET;

typedef struct {
	PREVIOUS_PACKET_OFFSET PreviousPacketStartOffset;
	VMPACKET_DESCRIPTOR Descriptor;
} __attribute__((packed)) VMPACKET_HEADER, *PVMPACKET_HEADER;

typedef struct {
	u32 ByteCount;
	u32 ByteOffset;
} __attribute__((packed)) VMTRANSFER_PAGE_RANGE, *PVMTRANSFER_PAGE_RANGE;

typedef struct VMTRANSFER_PAGE_PACKET_HEADER {
	VMPACKET_DESCRIPTOR d;
	u16 TransferPageSetId;
	bool SenderOwnsSet;
	u8 Reserved;
	u32 RangeCount;
	VMTRANSFER_PAGE_RANGE Ranges[1];
} __attribute__((packed)) VMTRANSFER_PAGE_PACKET_HEADER, *PVMTRANSFER_PAGE_PACKET_HEADER;

typedef struct _VMGPADL_PACKET_HEADER {
	VMPACKET_DESCRIPTOR d;
	u32 Gpadl;
	u32 Reserved;
} __attribute__((packed)) VMGPADL_PACKET_HEADER, *PVMGPADL_PACKET_HEADER;

typedef struct _VMADD_REMOVE_TRANSFER_PAGE_SET {
	VMPACKET_DESCRIPTOR d;
	u32 Gpadl;
	u16 TransferPageSetId;
	u16 Reserved;
} __attribute__((packed)) VMADD_REMOVE_TRANSFER_PAGE_SET, *PVMADD_REMOVE_TRANSFER_PAGE_SET;

/*
 * This structure defines a range in guest physical space that can be made to
 * look virtually contiguous.
 */
typedef struct _GPA_RANGE {
	u32 ByteCount;
	u32 ByteOffset;
	u64 PfnArray[0];
} GPA_RANGE, *PGPA_RANGE;

/*
 * This is the format for an Establish Gpadl packet, which contains a handle by
 * which this GPADL will be known and a set of GPA ranges associated with it.
 * This can be converted to a MDL by the guest OS.  If there are multiple GPA
 * ranges, then the resulting MDL will be "chained," representing multiple VA
 * ranges.
 */
typedef struct _VMESTABLISH_GPADL {
	VMPACKET_DESCRIPTOR d;
	u32 Gpadl;
	u32 RangeCount;
	GPA_RANGE Range[1];
} __attribute__((packed)) VMESTABLISH_GPADL, *PVMESTABLISH_GPADL;

/*
 * This is the format for a Teardown Gpadl packet, which indicates that the
 * GPADL handle in the Establish Gpadl packet will never be referenced again.
 */
typedef struct _VMTEARDOWN_GPADL {
	VMPACKET_DESCRIPTOR d;
	u32 Gpadl;
	u32 Reserved;	/* for alignment to a 8-byte boundary */
} __attribute__((packed)) VMTEARDOWN_GPADL, *PVMTEARDOWN_GPADL;

/*
 * This is the format for a GPA-Direct packet, which contains a set of GPA
 * ranges, in addition to commands and/or data.
 */
typedef struct _VMDATA_GPA_DIRECT {
	VMPACKET_DESCRIPTOR d;
	u32 Reserved;
	u32 RangeCount;
	GPA_RANGE Range[1];
} __attribute__((packed)) VMDATA_GPA_DIRECT, *PVMDATA_GPA_DIRECT;

/* This is the format for a Additional Data Packet. */
typedef struct _VMADDITIONAL_DATA {
	VMPACKET_DESCRIPTOR d;
	u64 TotalBytes;
	u32 ByteOffset;
	u32 ByteCount;
	unsigned char Data[1];
} __attribute__((packed)) VMADDITIONAL_DATA, *PVMADDITIONAL_DATA;

typedef union {
	VMPACKET_DESCRIPTOR SimpleHeader;
	VMTRANSFER_PAGE_PACKET_HEADER TransferPageHeader;
	VMGPADL_PACKET_HEADER GpadlHeader;
	VMADD_REMOVE_TRANSFER_PAGE_SET AddRemoveTransferPageHeader;
	VMESTABLISH_GPADL EstablishGpadlHeader;
	VMTEARDOWN_GPADL TeardownGpadlHeader;
	VMDATA_GPA_DIRECT DataGpaDirectHeader;
} VMPACKET_LARGEST_POSSIBLE_HEADER, *PVMPACKET_LARGEST_POSSIBLE_HEADER;

#define VMPACKET_DATA_START_ADDRESS(__packet)	\
	(void *)(((unsigned char *)__packet) +	\
	 ((PVMPACKET_DESCRIPTOR)__packet)->DataOffset8 * 8)

#define VMPACKET_DATA_LENGTH(__packet)		\
	((((PVMPACKET_DESCRIPTOR)__packet)->Length8 -	\
	  ((PVMPACKET_DESCRIPTOR)__packet)->DataOffset8) * 8)

#define VMPACKET_TRANSFER_MODE(__packet)	\
	(((PVMPACKET_DESCRIPTOR)__packet)->Type)

typedef enum {
	VmbusServerEndpoint = 0,
	VmbusClientEndpoint,
	VmbusEndpointMaximum
} ENDPOINT_TYPE, *PENDPOINT_TYPE;

typedef enum {
	VmbusPacketTypeInvalid				= 0x0,
	VmbusPacketTypeSynch				= 0x1,
	VmbusPacketTypeAddTransferPageSet		= 0x2,
	VmbusPacketTypeRemoveTransferPageSet		= 0x3,
	VmbusPacketTypeEstablishGpadl			= 0x4,
	VmbusPacketTypeTearDownGpadl			= 0x5,
	VmbusPacketTypeDataInBand			= 0x6,
	VmbusPacketTypeDataUsingTransferPages		= 0x7,
	VmbusPacketTypeDataUsingGpadl			= 0x8,
	VmbusPacketTypeDataUsingGpaDirect		= 0x9,
	VmbusPacketTypeCancelRequest			= 0xa,
	VmbusPacketTypeCompletion			= 0xb,
	VmbusPacketTypeDataUsingAdditionalPackets	= 0xc,
	VmbusPacketTypeAdditionalData			= 0xd
} VMBUS_PACKET_TYPE, *PVMBUS_PACKET_TYPE;

#define VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED	1

#endif
