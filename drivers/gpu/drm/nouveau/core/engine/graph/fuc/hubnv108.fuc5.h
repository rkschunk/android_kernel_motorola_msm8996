uint32_t nv108_grhub_data[] = {
/* 0x0000: hub_mmio_list_head */
	0x00000300,
/* 0x0004: hub_mmio_list_tail */
	0x00000304,
/* 0x0008: gpc_count */
	0x00000000,
/* 0x000c: rop_count */
	0x00000000,
/* 0x0010: cmd_queue */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
/* 0x0058: ctx_current */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
/* 0x0100: chan_data */
/* 0x0100: chan_mmio_count */
	0x00000000,
/* 0x0104: chan_mmio_address */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
/* 0x0200: xfer_data */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
/* 0x0300: hub_mmio_list_base */
	0x0417e91c,
};

uint32_t nv108_grhub_code[] = {
	0x030e0ef5,
/* 0x0004: queue_put */
	0x9800d898,
	0x86f001d9,
	0xf489a408,
	0x020f0b1b,
	0x0002f87e,
/* 0x001a: queue_put_next */
	0x98c400f8,
	0x0384b607,
	0xb6008dbb,
	0x8eb50880,
	0x018fb500,
	0xf00190b6,
	0xd9b50f94,
/* 0x0037: queue_get */
	0xf400f801,
	0xd8980131,
	0x01d99800,
	0x0bf489a4,
	0x0789c421,
	0xbb0394b6,
	0x90b6009d,
	0x009e9808,
	0xb6019f98,
	0x84f00180,
	0x00d8b50f,
/* 0x0063: queue_get_done */
	0xf80132f4,
/* 0x0065: nv_rd32 */
	0xf0ecb200,
	0x00801fc9,
	0x0cf601ca,
/* 0x0073: nv_rd32_wait */
	0x8c04bd00,
	0xcf01ca00,
	0xccc800cc,
	0xf61bf41f,
	0xec7e060a,
	0x008f0000,
	0xffcf01cb,
/* 0x008f: nv_wr32 */
	0x8000f800,
	0xf601cc00,
	0x04bd000f,
	0xc9f0ecb2,
	0x1ec9f01f,
	0x01ca0080,
	0xbd000cf6,
/* 0x00a9: nv_wr32_wait */
	0xca008c04,
	0x00cccf01,
	0xf41fccc8,
	0x00f8f61b,
/* 0x00b8: wait_donez */
	0x99f094bd,
	0x37008000,
	0x0009f602,
	0x008004bd,
	0x0af60206,
/* 0x00cf: wait_donez_ne */
	0x8804bd00,
	0xcf010000,
	0x8aff0088,
	0xf61bf488,
	0x99f094bd,
	0x17008000,
	0x0009f602,
	0x00f804bd,
/* 0x00ec: wait_doneo */
	0x99f094bd,
	0x37008000,
	0x0009f602,
	0x008004bd,
	0x0af60206,
/* 0x0103: wait_doneo_e */
	0x8804bd00,
	0xcf010000,
	0x8aff0088,
	0xf60bf488,
	0x99f094bd,
	0x17008000,
	0x0009f602,
	0x00f804bd,
/* 0x0120: mmctx_size */
/* 0x0122: nv_mmctx_size_loop */
	0xe89894bd,
	0x1a85b600,
	0xb60180b6,
	0x98bb0284,
	0x04e0b600,
	0x1bf4efa4,
	0xf89fb2ec,
/* 0x013d: mmctx_xfer */
	0xf094bd00,
	0x00800199,
	0x09f60237,
	0xbd04bd00,
	0x05bbfd94,
	0x800f0bf4,
	0xf601c400,
	0x04bd000b,
/* 0x015f: mmctx_base_disabled */
	0xfd0099f0,
	0x0bf405ee,
	0xc6008018,
	0x000ef601,
	0x008004bd,
	0x0ff601c7,
	0xf004bd00,
/* 0x017a: mmctx_multi_disabled */
	0xabc80199,
	0x10b4b600,
	0xc80cb9f0,
	0xe4b601ae,
	0x05befd11,
	0x01c50080,
	0xbd000bf6,
/* 0x0195: mmctx_exec_loop */
/* 0x0195: mmctx_wait_free */
	0xc5008e04,
	0x00eecf01,
	0xf41fe4f0,
	0xce98f60b,
	0x05e9fd00,
	0x01c80080,
	0xbd000ef6,
	0x04c0b604,
	0x1bf4cda4,
	0x02abc8df,
/* 0x01bf: mmctx_fini_wait */
	0x8b1c1bf4,
	0xcf01c500,
	0xb4f000bb,
	0x10b4b01f,
	0x0af31bf4,
	0x00b87e05,
	0x250ef400,
/* 0x01d8: mmctx_stop */
	0xb600abc8,
	0xb9f010b4,
	0x12b9f00c,
	0x01c50080,
	0xbd000bf6,
/* 0x01ed: mmctx_stop_wait */
	0xc5008b04,
	0x00bbcf01,
	0xf412bbc8,
/* 0x01fa: mmctx_done */
	0x94bdf61b,
	0x800199f0,
	0xf6021700,
	0x04bd0009,
/* 0x020a: strand_wait */
	0xa0f900f8,
	0xb87e020a,
	0xa0fc0000,
/* 0x0216: strand_pre */
	0x0c0900f8,
	0x024afc80,
	0xbd0009f6,
	0x020a7e04,
/* 0x0227: strand_post */
	0x0900f800,
	0x4afc800d,
	0x0009f602,
	0x0a7e04bd,
	0x00f80002,
/* 0x0238: strand_set */
	0xfc800f0c,
	0x0cf6024f,
	0x0c04bd00,
	0x4afc800b,
	0x000cf602,
	0xfc8004bd,
	0x0ef6024f,
	0x0c04bd00,
	0x4afc800a,
	0x000cf602,
	0x0a7e04bd,
	0x00f80002,
/* 0x0268: strand_ctx_init */
	0x99f094bd,
	0x37008003,
	0x0009f602,
	0x167e04bd,
	0x030e0002,
	0x0002387e,
	0xfc80c4bd,
	0x0cf60247,
	0x0c04bd00,
	0x4afc8001,
	0x000cf602,
	0x0a7e04bd,
	0x0c920002,
	0x46fc8001,
	0x000cf602,
	0x020c04bd,
	0x024afc80,
	0xbd000cf6,
	0x020a7e04,
	0x02277e00,
	0x42008800,
	0x20008902,
	0x0099cf02,
/* 0x02c7: ctx_init_strand_loop */
	0xf608fe95,
	0x8ef6008e,
	0x808acf40,
	0xb606a5b6,
	0xeabb01a0,
	0x0480b600,
	0xf40192b6,
	0xe4b6e81b,
	0xf2efbc08,
	0x99f094bd,
	0x17008003,
	0x0009f602,
	0x00f804bd,
/* 0x02f8: error */
	0x02050080,
	0xbd000ff6,
	0x80010f04,
	0xf6030700,
	0x04bd000f,
/* 0x030e: init */
	0x04bd00f8,
	0x410007fe,
	0x11cf4200,
	0x0911e700,
	0x0814b601,
	0x020014fe,
	0x12004002,
	0xbd0002f6,
	0x05c94104,
	0xbd0010fe,
	0x07004024,
	0xbd0002f6,
	0x20034204,
	0x01010080,
	0xbd0002f6,
	0x20044204,
	0x01010480,
	0xbd0002f6,
	0x200b4204,
	0x01010880,
	0xbd0002f6,
	0x200c4204,
	0x01011c80,
	0xbd0002f6,
	0x01039204,
	0x03090080,
	0xbd0003f6,
	0x87044204,
	0xf6040040,
	0x04bd0002,
	0x00400402,
	0x0002f603,
	0x31f404bd,
	0x96048e10,
	0x00657e40,
	0xc7feb200,
	0x01b590f1,
	0x1ff4f003,
	0x01020fb5,
	0x041fbb01,
	0x800112b6,
	0xf6010300,
	0x04bd0001,
	0x01040080,
	0xbd0001f6,
	0x01004104,
	0x627e020f,
	0x717e0006,
	0x100f0006,
	0x0006b37e,
	0x98000e98,
	0x207e010f,
	0x14950001,
	0xc0008008,
	0x0004f601,
	0x008004bd,
	0x04f601c1,
	0xb704bd00,
	0xbb130030,
	0xf5b6001f,
	0xd3008002,
	0x000ff601,
	0x15b604bd,
	0x0110b608,
	0xb20814b6,
	0x02687e1f,
	0x001fbb00,
	0x84020398,
/* 0x041f: init_gpc */
	0xb8502000,
	0x0008044e,
	0x8f7e1fb2,
	0x4eb80000,
	0xbd00010c,
	0x008f7ef4,
	0x044eb800,
	0x8f7e0001,
	0x4eb80000,
	0x0f000100,
	0x008f7e02,
	0x004eb800,
/* 0x044e: init_gpc_wait */
	0x657e0008,
	0xffc80000,
	0xf90bf41f,
	0x08044eb8,
	0x00657e00,
	0x001fbb00,
	0x800040b7,
	0xf40132b6,
	0x000fb41b,
	0x0006b37e,
	0x627e000f,
	0x00800006,
	0x01f60201,
	0xbd04bd00,
	0x1f19f014,
	0x02300080,
	0xbd0001f6,
/* 0x0491: main */
	0x0031f404,
	0x0d0028f4,
	0x00377e10,
	0xf401f400,
	0x4001e4b1,
	0x00c71bf5,
	0x99f094bd,
	0x37008004,
	0x0009f602,
	0x008104bd,
	0x11cf02c0,
	0xc1008200,
	0x0022cf02,
	0xf41f13c8,
	0x23c8770b,
	0x550bf41f,
	0x12b220f9,
	0x99f094bd,
	0x37008007,
	0x0009f602,
	0x32f404bd,
	0x0231f401,
	0x0008367e,
	0x99f094bd,
	0x17008007,
	0x0009f602,
	0x20fc04bd,
	0x99f094bd,
	0x37008006,
	0x0009f602,
	0x31f404bd,
	0x08367e01,
	0xf094bd00,
	0x00800699,
	0x09f60217,
	0xf404bd00,
/* 0x0522: chsw_prev_no_next */
	0x20f92f0e,
	0x32f412b2,
	0x0232f401,
	0x0008367e,
	0x008020fc,
	0x02f602c0,
	0xf404bd00,
/* 0x053e: chsw_no_prev */
	0x23c8130e,
	0x0d0bf41f,
	0xf40131f4,
	0x367e0232,
/* 0x054e: chsw_done */
	0x01020008,
	0x02c30080,
	0xbd0002f6,
	0xf094bd04,
	0x00800499,
	0x09f60217,
	0xf504bd00,
/* 0x056b: main_not_ctx_switch */
	0xb0ff2a0e,
	0x1bf401e4,
	0x7ef2b20c,
	0xf40007d6,
/* 0x057a: main_not_ctx_chan */
	0xe4b0400e,
	0x2c1bf402,
	0x99f094bd,
	0x37008007,
	0x0009f602,
	0x32f404bd,
	0x0232f401,
	0x0008367e,
	0x99f094bd,
	0x17008007,
	0x0009f602,
	0x0ef404bd,
/* 0x05a9: main_not_ctx_save */
	0x10ef9411,
	0x7e01f5f0,
	0xf50002f8,
/* 0x05b7: main_done */
	0xbdfede0e,
	0x1f29f024,
	0x02300080,
	0xbd0002f6,
	0xcc0ef504,
/* 0x05c9: ih */
	0xfe80f9fe,
	0x80f90188,
	0xa0f990f9,
	0xd0f9b0f9,
	0xf0f9e0f9,
	0x004a04bd,
	0x00aacf02,
	0xf404abc4,
	0x100d230b,
	0xcf1a004e,
	0x004f00ee,
	0x00ffcf19,
	0x0000047e,
	0x0400b0b7,
	0x0040010e,
	0x000ef61d,
/* 0x060a: ih_no_fifo */
	0xabe404bd,
	0x0bf40100,
	0x4e100d0c,
	0x047e4001,
/* 0x061a: ih_no_ctxsw */
	0xabe40000,
	0x0bf40400,
	0x01004b10,
	0x448ebfb2,
	0x8f7e4001,
/* 0x062e: ih_no_fwmthd */
	0x044b0000,
	0xffb0bd01,
	0x0bf4b4ab,
	0x0700800c,
	0x000bf603,
/* 0x0642: ih_no_other */
	0x004004bd,
	0x000af601,
	0xf0fc04bd,
	0xd0fce0fc,
	0xa0fcb0fc,
	0x80fc90fc,
	0xfc0088fe,
	0x0032f480,
/* 0x0662: ctx_4170s */
	0xf5f001f8,
	0x8effb210,
	0x7e404170,
	0xf800008f,
/* 0x0671: ctx_4170w */
	0x41708e00,
	0x00657e40,
	0xf0ffb200,
	0x1bf410f4,
/* 0x0683: ctx_redswitch */
	0x4e00f8f3,
	0xe5f00200,
	0x20e5f040,
	0x8010e5f0,
	0xf6018500,
	0x04bd000e,
/* 0x069a: ctx_redswitch_delay */
	0xf2b6080f,
	0xfd1bf401,
	0x0400e5f1,
	0x0100e5f1,
	0x01850080,
	0xbd000ef6,
/* 0x06b3: ctx_86c */
	0x8000f804,
	0xf6022300,
	0x04bd000f,
	0x148effb2,
	0x8f7e408a,
	0xffb20000,
	0x41a88c8e,
	0x00008f7e,
/* 0x06d2: ctx_mem */
	0x008000f8,
	0x0ff60284,
/* 0x06db: ctx_mem_wait */
	0x8f04bd00,
	0xcf028400,
	0xfffd00ff,
	0xf61bf405,
/* 0x06ea: ctx_load */
	0x94bd00f8,
	0x800599f0,
	0xf6023700,
	0x04bd0009,
	0xb87e0c0a,
	0xf4bd0000,
	0x02890080,
	0xbd000ff6,
	0xc1008004,
	0x0002f602,
	0x008004bd,
	0x02f60283,
	0x0f04bd00,
	0x06d27e07,
	0xc0008000,
	0x0002f602,
	0x0bfe04bd,
	0x1f2af000,
	0xb60424b6,
	0x94bd0220,
	0x800899f0,
	0xf6023700,
	0x04bd0009,
	0x02810080,
	0xbd0002f6,
	0x0000d204,
	0x25f08000,
	0x88008002,
	0x0002f602,
	0x100104bd,
	0xf0020042,
	0x12fa0223,
	0xbd03f805,
	0x0899f094,
	0x02170080,
	0xbd0009f6,
	0x81019804,
	0x981814b6,
	0x25b68002,
	0x0512fd08,
	0xbd1601b5,
	0x0999f094,
	0x02370080,
	0xbd0009f6,
	0x81008004,
	0x0001f602,
	0x010204bd,
	0x02880080,
	0xbd0002f6,
	0x01004104,
	0xfa0613f0,
	0x03f80501,
	0x99f094bd,
	0x17008009,
	0x0009f602,
	0x94bd04bd,
	0x800599f0,
	0xf6021700,
	0x04bd0009,
/* 0x07d6: ctx_chan */
	0xea7e00f8,
	0x0c0a0006,
	0x0000b87e,
	0xd27e050f,
	0x00f80006,
/* 0x07e8: ctx_mmio_exec */
	0x80410398,
	0xf6028100,
	0x04bd0003,
/* 0x07f6: ctx_mmio_loop */
	0x34c434bd,
	0x0e1bf4ff,
	0xf0020045,
	0x35fa0653,
/* 0x0807: ctx_mmio_pull */
	0x9803f805,
	0x4f98804e,
	0x008f7e81,
	0x0830b600,
	0xf40112b6,
/* 0x081a: ctx_mmio_done */
	0x0398df1b,
	0x81008016,
	0x0003f602,
	0x00b504bd,
	0x01004140,
	0xfa0613f0,
	0x03f80601,
/* 0x0836: ctx_xfer */
	0x040e00f8,
	0x03020080,
	0xbd000ef6,
/* 0x0841: ctx_xfer_idle */
	0x00008e04,
	0x00eecf03,
	0x2000e4f1,
	0xf4f51bf4,
	0x02f40611,
/* 0x0855: ctx_xfer_pre */
	0x7e100f0c,
	0xf40006b3,
/* 0x085e: ctx_xfer_pre_load */
	0x020f1b11,
	0x0006627e,
	0x0006717e,
	0x0006837e,
	0x627ef4bd,
	0xea7e0006,
/* 0x0876: ctx_xfer_exec */
	0x01980006,
	0x8024bd16,
	0xf6010500,
	0x04bd0002,
	0x008e1fb2,
	0x8f7e41a5,
	0xfcf00000,
	0x022cf001,
	0xfd0124b6,
	0xffb205f2,
	0x41a5048e,
	0x00008f7e,
	0x0002167e,
	0xfc8024bd,
	0x02f60247,
	0xf004bd00,
	0x20b6012c,
	0x4afc8003,
	0x0002f602,
	0xacf004bd,
	0x06a5f001,
	0x0c98000b,
	0x010d9800,
	0x3d7e000e,
	0x080a0001,
	0x0000ec7e,
	0x00020a7e,
	0x0a1201f4,
	0x00b87e0c,
	0x7e050f00,
	0xf40006d2,
/* 0x08f2: ctx_xfer_post */
	0x020f2d02,
	0x0006627e,
	0xb37ef4bd,
	0x277e0006,
	0x717e0002,
	0xf4bd0006,
	0x0006627e,
	0x981011f4,
	0x11fd4001,
	0x070bf405,
	0x0007e87e,
/* 0x091c: ctx_xfer_no_post_mmio */
/* 0x091c: ctx_xfer_done */
	0x000000f8,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};
