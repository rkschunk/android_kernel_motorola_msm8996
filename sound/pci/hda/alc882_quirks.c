/*
 * ALC882/ALC883/ALC888/ALC889 quirk models
 * included by patch_realtek.c
 */

/* ALC882 models */
enum {
	ALC882_AUTO,
	ALC885_MACPRO,
	ALC885_MBA21,
	ALC885_MBP3,
	ALC885_MB5,
	ALC885_MACMINI3,
	ALC885_IMAC24,
	ALC885_IMAC91,
	ALC888_ACER_ASPIRE_4930G,
	ALC888_ACER_ASPIRE_6530G,
	ALC888_ACER_ASPIRE_8930G,
	ALC888_ACER_ASPIRE_7730G,
	ALC889A_MB31,
	ALC882_MODEL_LAST,
};

/*
 * ALC888 Acer Aspire 4930G model
 */

static const struct hda_verb alc888_acer_aspire_4930g_verbs[] = {
/* Front Mic: set to PIN_IN (empty by default) */
	{0x12, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},
/* Unselect Front Mic by default in input mixer 3 */
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0xb)},
/* Enable unsolicited event for HP jack */
	{0x15, AC_VERB_SET_UNSOLICITED_ENABLE, ALC_HP_EVENT | AC_USRSP_EN},
/* Connect Internal HP to front */
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x14, AC_VERB_SET_CONNECT_SEL, 0x00},
/* Connect HP out to front */
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x00},
	{0x15, AC_VERB_SET_EAPD_BTLENABLE, 2},
	{ }
};

/*
 * ALC888 Acer Aspire 6530G model
 */

static const struct hda_verb alc888_acer_aspire_6530g_verbs[] = {
/* Route to built-in subwoofer as well as speakers */
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(1)},
	{0x0f, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x0f, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(1)},
/* Bias voltage on for external mic port */
	{0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN | PIN_VREF80},
/* Front Mic: set to PIN_IN (empty by default) */
	{0x12, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},
/* Unselect Front Mic by default in input mixer 3 */
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0xb)},
/* Enable unsolicited event for HP jack */
	{0x15, AC_VERB_SET_UNSOLICITED_ENABLE, ALC_HP_EVENT | AC_USRSP_EN},
/* Enable speaker output */
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x14, AC_VERB_SET_EAPD_BTLENABLE, 2},
/* Enable headphone output */
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT | PIN_HP},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x00},
	{0x15, AC_VERB_SET_EAPD_BTLENABLE, 2},
	{ }
};

/*
 *ALC888 Acer Aspire 7730G model
 */

static const struct hda_verb alc888_acer_aspire_7730G_verbs[] = {
/* Bias voltage on for external mic port */
	{0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN | PIN_VREF80},
/* Front Mic: set to PIN_IN (empty by default) */
	{0x12, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},
/* Unselect Front Mic by default in input mixer 3 */
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0xb)},
/* Enable unsolicited event for HP jack */
	{0x15, AC_VERB_SET_UNSOLICITED_ENABLE, ALC_HP_EVENT | AC_USRSP_EN},
/* Enable speaker output */
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x14, AC_VERB_SET_EAPD_BTLENABLE, 2},
/* Enable headphone output */
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT | PIN_HP},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x00},
	{0x15, AC_VERB_SET_EAPD_BTLENABLE, 2},
/*Enable internal subwoofer */
	{0x17, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x17, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x17, AC_VERB_SET_CONNECT_SEL, 0x02},
	{0x17, AC_VERB_SET_EAPD_BTLENABLE, 2},
	{ }
};

/*
 * ALC889 Acer Aspire 8930G model
 */

static const struct hda_verb alc889_acer_aspire_8930g_verbs[] = {
/* Front Mic: set to PIN_IN (empty by default) */
	{0x12, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},
/* Unselect Front Mic by default in input mixer 3 */
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0xb)},
/* Enable unsolicited event for HP jack */
	{0x15, AC_VERB_SET_UNSOLICITED_ENABLE, ALC_HP_EVENT | AC_USRSP_EN},
/* Connect Internal Front to Front */
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x14, AC_VERB_SET_CONNECT_SEL, 0x00},
/* Connect Internal Rear to Rear */
	{0x1b, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x1b, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x1b, AC_VERB_SET_CONNECT_SEL, 0x01},
/* Connect Internal CLFE to CLFE */
	{0x16, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x16, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x16, AC_VERB_SET_CONNECT_SEL, 0x02},
/* Connect HP out to Front */
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT | PIN_HP},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x00},
/* Enable all DACs */
/*  DAC DISABLE/MUTE 1? */
/*  setting bits 1-5 disables DAC nids 0x02-0x06 apparently. Init=0x38 */
	{0x20, AC_VERB_SET_COEF_INDEX, 0x03},
	{0x20, AC_VERB_SET_PROC_COEF, 0x0000},
/*  DAC DISABLE/MUTE 2? */
/*  some bit here disables the other DACs. Init=0x4900 */
	{0x20, AC_VERB_SET_COEF_INDEX, 0x08},
	{0x20, AC_VERB_SET_PROC_COEF, 0x0000},
/* DMIC fix
 * This laptop has a stereo digital microphone. The mics are only 1cm apart
 * which makes the stereo useless. However, either the mic or the ALC889
 * makes the signal become a difference/sum signal instead of standard
 * stereo, which is annoying. So instead we flip this bit which makes the
 * codec replicate the sum signal to both channels, turning it into a
 * normal mono mic.
 */
/*  DMIC_CONTROL? Init value = 0x0001 */
	{0x20, AC_VERB_SET_COEF_INDEX, 0x0b},
	{0x20, AC_VERB_SET_PROC_COEF, 0x0003},
	{ }
};

static const struct hda_input_mux alc888_2_capture_sources[2] = {
	/* Front mic only available on one ADC */
	{
		.num_items = 4,
		.items = {
			{ "Mic", 0x0 },
			{ "Line", 0x2 },
			{ "CD", 0x4 },
			{ "Front Mic", 0xb },
		},
	},
	{
		.num_items = 3,
		.items = {
			{ "Mic", 0x0 },
			{ "Line", 0x2 },
			{ "CD", 0x4 },
		},
	}
};

static const struct hda_input_mux alc888_acer_aspire_6530_sources[2] = {
	/* Interal mic only available on one ADC */
	{
		.num_items = 5,
		.items = {
			{ "Mic", 0x0 },
			{ "Line In", 0x2 },
			{ "CD", 0x4 },
			{ "Input Mix", 0xa },
			{ "Internal Mic", 0xb },
		},
	},
	{
		.num_items = 4,
		.items = {
			{ "Mic", 0x0 },
			{ "Line In", 0x2 },
			{ "CD", 0x4 },
			{ "Input Mix", 0xa },
		},
	}
};

static const struct hda_input_mux alc889_capture_sources[3] = {
	/* Digital mic only available on first "ADC" */
	{
		.num_items = 5,
		.items = {
			{ "Mic", 0x0 },
			{ "Line", 0x2 },
			{ "CD", 0x4 },
			{ "Front Mic", 0xb },
			{ "Input Mix", 0xa },
		},
	},
	{
		.num_items = 4,
		.items = {
			{ "Mic", 0x0 },
			{ "Line", 0x2 },
			{ "CD", 0x4 },
			{ "Input Mix", 0xa },
		},
	},
	{
		.num_items = 4,
		.items = {
			{ "Mic", 0x0 },
			{ "Line", 0x2 },
			{ "CD", 0x4 },
			{ "Input Mix", 0xa },
		},
	}
};

static const struct snd_kcontrol_new alc888_base_mixer[] = {
	HDA_CODEC_VOLUME("Front Playback Volume", 0x0c, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE("Front Playback Switch", 0x0c, 2, HDA_INPUT),
	HDA_CODEC_VOLUME("Surround Playback Volume", 0x0d, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE("Surround Playback Switch", 0x0d, 2, HDA_INPUT),
	HDA_CODEC_VOLUME_MONO("Center Playback Volume", 0x0e, 1, 0x0,
		HDA_OUTPUT),
	HDA_CODEC_VOLUME_MONO("LFE Playback Volume", 0x0e, 2, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE_MONO("Center Playback Switch", 0x0e, 1, 2, HDA_INPUT),
	HDA_BIND_MUTE_MONO("LFE Playback Switch", 0x0e, 2, 2, HDA_INPUT),
	HDA_CODEC_VOLUME("Side Playback Volume", 0x0f, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE("Side Playback Switch", 0x0f, 2, HDA_INPUT),
	HDA_CODEC_VOLUME("CD Playback Volume", 0x0b, 0x04, HDA_INPUT),
	HDA_CODEC_MUTE("CD Playback Switch", 0x0b, 0x04, HDA_INPUT),
	HDA_CODEC_VOLUME("Line Playback Volume", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_MUTE("Line Playback Switch", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Playback Volume", 0x0b, 0x0, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Boost Volume", 0x18, 0, HDA_INPUT),
	HDA_CODEC_MUTE("Mic Playback Switch", 0x0b, 0x0, HDA_INPUT),
	{ } /* end */
};

static const struct snd_kcontrol_new alc888_acer_aspire_4930g_mixer[] = {
	HDA_CODEC_VOLUME("Front Playback Volume", 0x0c, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE("Front Playback Switch", 0x0c, 2, HDA_INPUT),
	HDA_CODEC_VOLUME("Surround Playback Volume", 0x0d, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE("Surround Playback Switch", 0x0d, 2, HDA_INPUT),
	HDA_CODEC_VOLUME_MONO("Center Playback Volume", 0x0e, 1, 0x0,
		HDA_OUTPUT),
	HDA_BIND_MUTE_MONO("Center Playback Switch", 0x0e, 1, 2, HDA_INPUT),
	HDA_CODEC_VOLUME_MONO("LFE Playback Volume", 0x0e, 2, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE_MONO("LFE Playback Switch", 0x0e, 2, 2, HDA_INPUT),
	HDA_CODEC_VOLUME_MONO("Internal LFE Playback Volume", 0x0f, 1, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE_MONO("Internal LFE Playback Switch", 0x0f, 1, 2, HDA_INPUT),
	HDA_CODEC_VOLUME("CD Playback Volume", 0x0b, 0x04, HDA_INPUT),
	HDA_CODEC_MUTE("CD Playback Switch", 0x0b, 0x04, HDA_INPUT),
	HDA_CODEC_VOLUME("Line Playback Volume", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_MUTE("Line Playback Switch", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Playback Volume", 0x0b, 0x0, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Boost Volume", 0x18, 0, HDA_INPUT),
	HDA_CODEC_MUTE("Mic Playback Switch", 0x0b, 0x0, HDA_INPUT),
	{ } /* end */
};

static const struct snd_kcontrol_new alc889_acer_aspire_8930g_mixer[] = {
	HDA_CODEC_VOLUME("Front Playback Volume", 0x0c, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE("Front Playback Switch", 0x0c, 2, HDA_INPUT),
	HDA_CODEC_VOLUME("Surround Playback Volume", 0x0d, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE("Surround Playback Switch", 0x0d, 2, HDA_INPUT),
	HDA_CODEC_VOLUME_MONO("Center Playback Volume", 0x0e, 1, 0x0,
		HDA_OUTPUT),
	HDA_CODEC_VOLUME_MONO("LFE Playback Volume", 0x0e, 2, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE_MONO("Center Playback Switch", 0x0e, 1, 2, HDA_INPUT),
	HDA_BIND_MUTE_MONO("LFE Playback Switch", 0x0e, 2, 2, HDA_INPUT),
	HDA_CODEC_VOLUME("Line Playback Volume", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_MUTE("Line Playback Switch", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Playback Volume", 0x0b, 0x0, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Boost Volume", 0x18, 0, HDA_INPUT),
	HDA_CODEC_MUTE("Mic Playback Switch", 0x0b, 0x0, HDA_INPUT),
	{ } /* end */
};


static void alc888_acer_aspire_4930g_setup(struct hda_codec *codec)
{
	struct alc_spec *spec = codec->spec;

	spec->autocfg.hp_pins[0] = 0x15;
	spec->autocfg.speaker_pins[0] = 0x14;
	spec->autocfg.speaker_pins[1] = 0x16;
	spec->autocfg.speaker_pins[2] = 0x17;
	alc_simple_setup_automute(spec, ALC_AUTOMUTE_AMP);
}

static void alc888_acer_aspire_6530g_setup(struct hda_codec *codec)
{
	struct alc_spec *spec = codec->spec;

	spec->autocfg.hp_pins[0] = 0x15;
	spec->autocfg.speaker_pins[0] = 0x14;
	spec->autocfg.speaker_pins[1] = 0x16;
	spec->autocfg.speaker_pins[2] = 0x17;
	alc_simple_setup_automute(spec, ALC_AUTOMUTE_AMP);
}

static void alc888_acer_aspire_7730g_setup(struct hda_codec *codec)
{
	struct alc_spec *spec = codec->spec;

	spec->autocfg.hp_pins[0] = 0x15;
	spec->autocfg.speaker_pins[0] = 0x14;
	spec->autocfg.speaker_pins[1] = 0x16;
	spec->autocfg.speaker_pins[2] = 0x17;
	alc_simple_setup_automute(spec, ALC_AUTOMUTE_AMP);
}

static void alc889_acer_aspire_8930g_setup(struct hda_codec *codec)
{
	struct alc_spec *spec = codec->spec;

	spec->autocfg.hp_pins[0] = 0x15;
	spec->autocfg.speaker_pins[0] = 0x14;
	spec->autocfg.speaker_pins[1] = 0x16;
	spec->autocfg.speaker_pins[2] = 0x1b;
	alc_simple_setup_automute(spec, ALC_AUTOMUTE_AMP);
}

#define ALC882_DIGOUT_NID	0x06
#define ALC882_DIGIN_NID	0x0a
#define ALC883_DIGOUT_NID	ALC882_DIGOUT_NID
#define ALC883_DIGIN_NID	ALC882_DIGIN_NID
#define ALC1200_DIGOUT_NID	0x10


static const struct hda_channel_mode alc882_ch_modes[1] = {
	{ 8, NULL }
};

/* DACs */
static const hda_nid_t alc882_dac_nids[4] = {
	/* front, rear, clfe, rear_surr */
	0x02, 0x03, 0x04, 0x05
};
#define alc883_dac_nids		alc882_dac_nids

/* ADCs */
#define alc882_adc_nids		alc880_adc_nids
#define alc882_adc_nids_alt	alc880_adc_nids_alt
#define alc883_adc_nids		alc882_adc_nids_alt
static const hda_nid_t alc883_adc_nids_alt[1] = { 0x08 };
static const hda_nid_t alc883_adc_nids_rev[2] = { 0x09, 0x08 };
#define alc889_adc_nids		alc880_adc_nids

static const hda_nid_t alc882_capsrc_nids[3] = { 0x24, 0x23, 0x22 };
static const hda_nid_t alc882_capsrc_nids_alt[2] = { 0x23, 0x22 };
#define alc883_capsrc_nids	alc882_capsrc_nids_alt
static const hda_nid_t alc883_capsrc_nids_rev[2] = { 0x22, 0x23 };
#define alc889_capsrc_nids	alc882_capsrc_nids

/* input MUX */
/* FIXME: should be a matrix-type input source selection */

static const struct hda_input_mux alc882_capture_source = {
	.num_items = 4,
	.items = {
		{ "Mic", 0x0 },
		{ "Front Mic", 0x1 },
		{ "Line", 0x2 },
		{ "CD", 0x4 },
	},
};

#define alc883_capture_source	alc882_capture_source

static const struct hda_input_mux alc889_capture_source = {
	.num_items = 3,
	.items = {
		{ "Front Mic", 0x0 },
		{ "Mic", 0x3 },
		{ "Line", 0x2 },
	},
};

static const struct hda_input_mux mb5_capture_source = {
	.num_items = 3,
	.items = {
		{ "Mic", 0x1 },
		{ "Line", 0x7 },
		{ "CD", 0x4 },
	},
};

static const struct hda_input_mux macmini3_capture_source = {
	.num_items = 2,
	.items = {
		{ "Line", 0x2 },
		{ "CD", 0x4 },
	},
};

static const struct hda_input_mux alc883_3stack_6ch_intel = {
	.num_items = 4,
	.items = {
		{ "Mic", 0x1 },
		{ "Front Mic", 0x0 },
		{ "Line", 0x2 },
		{ "CD", 0x4 },
	},
};

static const struct hda_input_mux alc889A_mb31_capture_source = {
	.num_items = 2,
	.items = {
		{ "Mic", 0x0 },
		/* Front Mic (0x01) unused */
		{ "Line", 0x2 },
		/* Line 2 (0x03) unused */
		/* CD (0x04) unused? */
	},
};

static const struct hda_input_mux alc889A_imac91_capture_source = {
	.num_items = 2,
	.items = {
		{ "Mic", 0x01 },
		{ "Line", 0x2 }, /* Not sure! */
	},
};

/*
 * 2ch mode
 */
static const struct hda_channel_mode alc883_3ST_2ch_modes[1] = {
	{ 2, NULL }
};

/*
 * 2ch mode
 */
static const struct hda_verb alc882_3ST_ch2_init[] = {
	{ 0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_VREF80 },
	{ 0x18, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE },
	{ 0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN },
	{ 0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE },
	{ } /* end */
};

/*
 * 4ch mode
 */
static const struct hda_verb alc882_3ST_ch4_init[] = {
	{ 0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_VREF80 },
	{ 0x18, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE },
	{ 0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT },
	{ 0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE },
	{ 0x1a, AC_VERB_SET_CONNECT_SEL, 0x01 },
	{ } /* end */
};

/*
 * 6ch mode
 */
static const struct hda_verb alc882_3ST_ch6_init[] = {
	{ 0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT },
	{ 0x18, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE },
	{ 0x18, AC_VERB_SET_CONNECT_SEL, 0x02 },
	{ 0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT },
	{ 0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE },
	{ 0x1a, AC_VERB_SET_CONNECT_SEL, 0x01 },
	{ } /* end */
};

static const struct hda_channel_mode alc882_3ST_6ch_modes[3] = {
	{ 2, alc882_3ST_ch2_init },
	{ 4, alc882_3ST_ch4_init },
	{ 6, alc882_3ST_ch6_init },
};

#define alc883_3ST_6ch_modes	alc882_3ST_6ch_modes


/* Macbook Air 2,1 */

static const struct hda_channel_mode alc885_mba21_ch_modes[1] = {
      { 2, NULL },
};

/*
 * macbook pro ALC885 can switch LineIn to LineOut without losing Mic
 */

/*
 * 2ch mode
 */
static const struct hda_verb alc885_mbp_ch2_init[] = {
	{ 0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN },
	{ 0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{ 0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	{ } /* end */
};

/*
 * 4ch mode
 */
static const struct hda_verb alc885_mbp_ch4_init[] = {
	{ 0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT },
	{ 0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{ 0x1a, AC_VERB_SET_CONNECT_SEL, 0x01 },
	{ 0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{ 0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(1)},
	{ } /* end */
};

static const struct hda_channel_mode alc885_mbp_4ch_modes[2] = {
	{ 2, alc885_mbp_ch2_init },
	{ 4, alc885_mbp_ch4_init },
};

/*
 * 2ch
 * Speakers/Woofer/HP = Front
 * LineIn = Input
 */
static const struct hda_verb alc885_mb5_ch2_init[] = {
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	{ } /* end */
};

/*
 * 6ch mode
 * Speakers/HP = Front
 * Woofer = LFE
 * LineIn = Surround
 */
static const struct hda_verb alc885_mb5_ch6_init[] = {
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x01},
	{ } /* end */
};

static const struct hda_channel_mode alc885_mb5_6ch_modes[2] = {
	{ 2, alc885_mb5_ch2_init },
	{ 6, alc885_mb5_ch6_init },
};

#define alc885_macmini3_6ch_modes	alc885_mb5_6ch_modes

/* Macbook Air 2,1 same control for HP and internal Speaker */

static const struct snd_kcontrol_new alc885_mba21_mixer[] = {
      HDA_CODEC_VOLUME("Speaker Playback Volume", 0x0c, 0x00, HDA_OUTPUT),
      HDA_BIND_MUTE("Speaker Playback Switch", 0x0c, 0x02, HDA_OUTPUT),
     { }
};


static const struct snd_kcontrol_new alc885_mbp3_mixer[] = {
	HDA_CODEC_VOLUME("Speaker Playback Volume", 0x0c, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE   ("Speaker Playback Switch", 0x0c, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Headphone Playback Volume", 0x0e, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE   ("Headphone Playback Switch", 0x0e, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Surround Playback Volume", 0x0d, 0x00, HDA_OUTPUT),
	HDA_CODEC_VOLUME("Line Playback Volume", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_MUTE  ("Line Playback Switch", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Playback Volume", 0x0b, 0x00, HDA_INPUT),
	HDA_CODEC_MUTE  ("Mic Playback Switch", 0x0b, 0x00, HDA_INPUT),
	HDA_CODEC_VOLUME("Line Boost Volume", 0x1a, 0x00, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Boost Volume", 0x18, 0x00, HDA_INPUT),
	{ } /* end */
};

static const struct snd_kcontrol_new alc885_mb5_mixer[] = {
	HDA_CODEC_VOLUME("Front Playback Volume", 0x0c, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE   ("Front Playback Switch", 0x0c, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Surround Playback Volume", 0x0d, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE   ("Surround Playback Switch", 0x0d, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("LFE Playback Volume", 0x0e, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE   ("LFE Playback Switch", 0x0e, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Headphone Playback Volume", 0x0f, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE   ("Headphone Playback Switch", 0x0f, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Line Playback Volume", 0x0b, 0x07, HDA_INPUT),
	HDA_CODEC_MUTE  ("Line Playback Switch", 0x0b, 0x07, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Playback Volume", 0x0b, 0x01, HDA_INPUT),
	HDA_CODEC_MUTE  ("Mic Playback Switch", 0x0b, 0x01, HDA_INPUT),
	HDA_CODEC_VOLUME("Line Boost Volume", 0x15, 0x00, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Boost Volume", 0x19, 0x00, HDA_INPUT),
	{ } /* end */
};

static const struct snd_kcontrol_new alc885_macmini3_mixer[] = {
	HDA_CODEC_VOLUME("Front Playback Volume", 0x0c, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE   ("Front Playback Switch", 0x0c, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Surround Playback Volume", 0x0d, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE   ("Surround Playback Switch", 0x0d, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("LFE Playback Volume", 0x0e, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE   ("LFE Playback Switch", 0x0e, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Headphone Playback Volume", 0x0f, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE   ("Headphone Playback Switch", 0x0f, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Line Playback Volume", 0x0b, 0x07, HDA_INPUT),
	HDA_CODEC_MUTE  ("Line Playback Switch", 0x0b, 0x07, HDA_INPUT),
	HDA_CODEC_VOLUME("Line Boost Volume", 0x15, 0x00, HDA_INPUT),
	{ } /* end */
};

static const struct snd_kcontrol_new alc885_imac91_mixer[] = {
	HDA_CODEC_VOLUME("Speaker Playback Volume", 0x0c, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE("Speaker Playback Switch", 0x0c, 0x02, HDA_INPUT),
	{ } /* end */
};


static const struct snd_kcontrol_new alc882_chmode_mixer[] = {
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "Channel Mode",
		.info = alc_ch_mode_info,
		.get = alc_ch_mode_get,
		.put = alc_ch_mode_put,
	},
	{ } /* end */
};

static const struct hda_verb alc882_base_init_verbs[] = {
	/* Front mixer: unmute input/output amp left and right (volume = 0) */
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* Rear mixer */
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* CLFE mixer */
	{0x0e, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0e, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* Side mixer */
	{0x0f, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0f, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},

	/* Front Pin: output 0 (0x0c) */
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x14, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* Rear Pin: output 1 (0x0d) */
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x01},
	/* CLFE Pin: output 2 (0x0e) */
	{0x16, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x16, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x16, AC_VERB_SET_CONNECT_SEL, 0x02},
	/* Side Pin: output 3 (0x0f) */
	{0x17, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x17, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x17, AC_VERB_SET_CONNECT_SEL, 0x03},
	/* Mic (rear) pin: input vref at 80% */
	{0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_VREF80},
	{0x18, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	/* Front Mic pin: input vref at 80% */
	{0x19, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_VREF80},
	{0x19, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	/* Line In pin: input */
	{0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},
	{0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	/* Line-2 In: Headphone output (output 0 - 0x0c) */
	{0x1b, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_HP},
	{0x1b, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x1b, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* CD pin widget for input */
	{0x1c, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},

	/* FIXME: use matrix-type input source selection */
	/* Mixer elements: 0x18, 19, 1a, 1b, 1c, 1d, 14, 15, 16, 17, 0b */
	/* Input mixer2 */
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	/* Input mixer3 */
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	/* ADC2: mute amp left and right */
	{0x08, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x08, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* ADC3: mute amp left and right */
	{0x09, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x09, AC_VERB_SET_CONNECT_SEL, 0x00},

	{ }
};

static const struct hda_verb alc882_adc1_init_verbs[] = {
	/* Input mixer1: unmute Mic, F-Mic, Line, CD inputs */
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	/* ADC1: mute amp left and right */
	{0x07, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x07, AC_VERB_SET_CONNECT_SEL, 0x00},
	{ }
};

static const struct hda_verb alc889_eapd_verbs[] = {
	{0x14, AC_VERB_SET_EAPD_BTLENABLE, 2},
	{0x15, AC_VERB_SET_EAPD_BTLENABLE, 2},
	{ }
};

#define alc883_init_verbs	alc882_base_init_verbs

/* Mac Pro test */
static const struct snd_kcontrol_new alc882_macpro_mixer[] = {
	HDA_CODEC_VOLUME("Front Playback Volume", 0x0c, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE("Front Playback Switch", 0x0c, 2, HDA_INPUT),
	HDA_CODEC_MUTE("Headphone Playback Switch", 0x18, 0x0, HDA_OUTPUT),
	HDA_CODEC_VOLUME("Line Playback Volume", 0x0b, 0x01, HDA_INPUT),
	HDA_CODEC_MUTE("Line Playback Switch", 0x0b, 0x01, HDA_INPUT),
	/* FIXME: this looks suspicious...
	HDA_CODEC_VOLUME("Beep Playback Volume", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_MUTE("Beep Playback Switch", 0x0b, 0x02, HDA_INPUT),
	*/
	{ } /* end */
};

static const struct hda_verb alc882_macpro_init_verbs[] = {
	/* Front mixer: unmute input/output amp left and right (volume = 0) */
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* Front Pin: output 0 (0x0c) */
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* Front Mic pin: input vref at 80% */
	{0x19, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_VREF80},
	{0x19, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	/* Speaker:  output */
	{0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x1a, AC_VERB_SET_CONNECT_SEL, 0x04},
	/* Headphone output (output 0 - 0x0c) */
	{0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_HP},
	{0x18, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x18, AC_VERB_SET_CONNECT_SEL, 0x00},

	/* FIXME: use matrix-type input source selection */
	/* Mixer elements: 0x18, 19, 1a, 1b, 1c, 1d, 14, 15, 16, 17, 0b */
	/* Input mixer1: unmute Mic, F-Mic, Line, CD inputs */
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	/* Input mixer2 */
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	/* Input mixer3 */
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	/* ADC1: mute amp left and right */
	{0x07, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x07, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* ADC2: mute amp left and right */
	{0x08, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x08, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* ADC3: mute amp left and right */
	{0x09, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x09, AC_VERB_SET_CONNECT_SEL, 0x00},

	{ }
};

/* Macbook 5,1 */
static const struct hda_verb alc885_mb5_init_verbs[] = {
	/* DACs */
	{0x02, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x03, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x04, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x05, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	/* Front mixer */
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* Surround mixer */
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* LFE mixer */
	{0x0e, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0e, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0e, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* HP mixer */
	{0x0f, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0f, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0f, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* Front Pin (0x0c) */
	{0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT | 0x01},
	{0x18, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x18, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* LFE Pin (0x0e) */
	{0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT | 0x01},
	{0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x1a, AC_VERB_SET_CONNECT_SEL, 0x02},
	/* HP Pin (0x0f) */
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x14, AC_VERB_SET_CONNECT_SEL, 0x03},
	{0x14, AC_VERB_SET_UNSOLICITED_ENABLE, ALC_HP_EVENT | AC_USRSP_EN},
	/* Front Mic pin: input vref at 80% */
	{0x19, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_VREF80},
	{0x19, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	/* Line In pin */
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},

	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0x1)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0x7)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0x4)},
	{ }
};

/* Macmini 3,1 */
static const struct hda_verb alc885_macmini3_init_verbs[] = {
	/* DACs */
	{0x02, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x03, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x04, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x05, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	/* Front mixer */
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* Surround mixer */
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* LFE mixer */
	{0x0e, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0e, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0e, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* HP mixer */
	{0x0f, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0f, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0f, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* Front Pin (0x0c) */
	{0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT | 0x01},
	{0x18, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x18, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* LFE Pin (0x0e) */
	{0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT | 0x01},
	{0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x1a, AC_VERB_SET_CONNECT_SEL, 0x02},
	/* HP Pin (0x0f) */
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x14, AC_VERB_SET_CONNECT_SEL, 0x03},
	{0x14, AC_VERB_SET_UNSOLICITED_ENABLE, ALC_HP_EVENT | AC_USRSP_EN},
	/* Line In pin */
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},

	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(1)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	{ }
};


static const struct hda_verb alc885_mba21_init_verbs[] = {
	/*Internal and HP Speaker Mixer*/
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(1)},
	/*Internal Speaker Pin (0x0c)*/
	{0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, (PIN_OUT | AC_PINCTL_VREF_50) },
	{0x18, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x18, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* HP Pin: output 0 (0x0e) */
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, 0xc4},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x14, AC_VERB_SET_CONNECT_SEL, 0x00},
	{0x14, AC_VERB_SET_UNSOLICITED_ENABLE, (ALC_HP_EVENT | AC_USRSP_EN)},
	/* Line in (is hp when jack connected)*/
	{0x19, AC_VERB_SET_PIN_WIDGET_CONTROL, AC_PINCTL_VREF_50},
	{0x19, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},

	{ }
 };


/* Macbook Pro rev3 */
static const struct hda_verb alc885_mbp3_init_verbs[] = {
	/* Front mixer: unmute input/output amp left and right (volume = 0) */
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* Rear mixer */
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* HP mixer */
	{0x0e, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0e, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0e, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* Front Pin: output 0 (0x0c) */
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x14, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* HP Pin: output 0 (0x0e) */
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, 0xc4},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x02},
	{0x15, AC_VERB_SET_UNSOLICITED_ENABLE, ALC_HP_EVENT | AC_USRSP_EN},
	/* Mic (rear) pin: input vref at 80% */
	{0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_VREF80},
	{0x18, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	/* Front Mic pin: input vref at 80% */
	{0x19, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_VREF80},
	{0x19, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	/* Line In pin: use output 1 when in LineOut mode */
	{0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},
	{0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	{0x1a, AC_VERB_SET_CONNECT_SEL, 0x01},

	/* FIXME: use matrix-type input source selection */
	/* Mixer elements: 0x18, 19, 1a, 1b, 1c, 1d, 14, 15, 16, 17, 0b */
	/* Input mixer1: unmute Mic, F-Mic, Line, CD inputs */
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	/* Input mixer2 */
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	/* Input mixer3 */
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	/* ADC1: mute amp left and right */
	{0x07, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x07, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* ADC2: mute amp left and right */
	{0x08, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x08, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* ADC3: mute amp left and right */
	{0x09, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x09, AC_VERB_SET_CONNECT_SEL, 0x00},

	{ }
};

/* iMac 9,1 */
static const struct hda_verb alc885_imac91_init_verbs[] = {
	/* Internal Speaker Pin (0x0c) */
	{0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, (PIN_OUT | AC_PINCTL_VREF_50) },
	{0x18, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x18, AC_VERB_SET_CONNECT_SEL, 0x00},
	{0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, (PIN_OUT | AC_PINCTL_VREF_50) },
	{0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x1a, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* HP Pin: Rear */
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_HP},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x14, AC_VERB_SET_CONNECT_SEL, 0x00},
	{0x14, AC_VERB_SET_UNSOLICITED_ENABLE, (ALC_HP_EVENT | AC_USRSP_EN)},
	/* Line in Rear */
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, AC_PINCTL_VREF_50},
	{0x15, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	/* Front Mic pin: input vref at 80% */
	{0x19, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_VREF80},
	{0x19, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	/* Rear mixer */
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0d, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* Line-Out mixer: unmute input/output amp left and right (volume = 0) */
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_ZERO},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x0c, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(1)},
	/* 0x24 [Audio Mixer] wcaps 0x20010b: Stereo Amp-In */
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x24, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	/* 0x23 [Audio Mixer] wcaps 0x20010b: Stereo Amp-In */
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x23, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	/* 0x22 [Audio Mixer] wcaps 0x20010b: Stereo Amp-In */
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_UNMUTE(0)},
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(3)},
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(2)},
	{0x22, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(4)},
	/* 0x07 [Audio Input] wcaps 0x10011b: Stereo Amp-In */
	{0x07, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x07, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* 0x08 [Audio Input] wcaps 0x10011b: Stereo Amp-In */
	{0x08, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x08, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* 0x09 [Audio Input] wcaps 0x10011b: Stereo Amp-In */
	{0x09, AC_VERB_SET_AMP_GAIN_MUTE, AMP_IN_MUTE(0)},
	{0x09, AC_VERB_SET_CONNECT_SEL, 0x00},
	{ }
};

/* iMac 24 mixer. */
static const struct snd_kcontrol_new alc885_imac24_mixer[] = {
	HDA_CODEC_VOLUME("Master Playback Volume", 0x0c, 0x00, HDA_OUTPUT),
	HDA_CODEC_MUTE("Master Playback Switch", 0x0c, 0x00, HDA_INPUT),
	{ } /* end */
};

/* iMac 24 init verbs. */
static const struct hda_verb alc885_imac24_init_verbs[] = {
	/* Internal speakers: output 0 (0x0c) */
	{0x18, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x18, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x18, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* Internal speakers: output 0 (0x0c) */
	{0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},
	{0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x1a, AC_VERB_SET_CONNECT_SEL, 0x00},
	/* Headphone: output 0 (0x0c) */
	{0x14, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_HP},
	{0x14, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE},
	{0x14, AC_VERB_SET_CONNECT_SEL, 0x00},
	{0x14, AC_VERB_SET_UNSOLICITED_ENABLE, ALC_HP_EVENT | AC_USRSP_EN},
	/* Front Mic: input vref at 80% */
	{0x19, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_VREF80},
	{0x19, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},
	{ }
};

/* Toggle speaker-output according to the hp-jack state */
static void alc885_imac24_setup(struct hda_codec *codec)
{
	struct alc_spec *spec = codec->spec;

	spec->autocfg.hp_pins[0] = 0x14;
	spec->autocfg.speaker_pins[0] = 0x18;
	spec->autocfg.speaker_pins[1] = 0x1a;
	alc_simple_setup_automute(spec, ALC_AUTOMUTE_AMP);
}

#define alc885_mb5_setup	alc885_imac24_setup
#define alc885_macmini3_setup	alc885_imac24_setup

/* Macbook Air 2,1 */
static void alc885_mba21_setup(struct hda_codec *codec)
{
	struct alc_spec *spec = codec->spec;

	spec->autocfg.hp_pins[0] = 0x14;
	spec->autocfg.speaker_pins[0] = 0x18;
	alc_simple_setup_automute(spec, ALC_AUTOMUTE_AMP);
}



static void alc885_mbp3_setup(struct hda_codec *codec)
{
	struct alc_spec *spec = codec->spec;

	spec->autocfg.hp_pins[0] = 0x15;
	spec->autocfg.speaker_pins[0] = 0x14;
	alc_simple_setup_automute(spec, ALC_AUTOMUTE_AMP);
}

static void alc885_imac91_setup(struct hda_codec *codec)
{
	struct alc_spec *spec = codec->spec;

	spec->autocfg.hp_pins[0] = 0x14;
	spec->autocfg.speaker_pins[0] = 0x18;
	spec->autocfg.speaker_pins[1] = 0x1a;
	alc_simple_setup_automute(spec, ALC_AUTOMUTE_AMP);
}

/* toggle speaker-output according to the hp-jack state */
static void alc882_gpio_mute(struct hda_codec *codec, int pin, int muted)
{
	unsigned int gpiostate, gpiomask, gpiodir;

	gpiostate = snd_hda_codec_read(codec, codec->afg, 0,
				       AC_VERB_GET_GPIO_DATA, 0);

	if (!muted)
		gpiostate |= (1 << pin);
	else
		gpiostate &= ~(1 << pin);

	gpiomask = snd_hda_codec_read(codec, codec->afg, 0,
				      AC_VERB_GET_GPIO_MASK, 0);
	gpiomask |= (1 << pin);

	gpiodir = snd_hda_codec_read(codec, codec->afg, 0,
				     AC_VERB_GET_GPIO_DIRECTION, 0);
	gpiodir |= (1 << pin);


	snd_hda_codec_write(codec, codec->afg, 0,
			    AC_VERB_SET_GPIO_MASK, gpiomask);
	snd_hda_codec_write(codec, codec->afg, 0,
			    AC_VERB_SET_GPIO_DIRECTION, gpiodir);

	msleep(1);

	snd_hda_codec_write(codec, codec->afg, 0,
			    AC_VERB_SET_GPIO_DATA, gpiostate);
}

/* set up GPIO at initialization */
static void alc885_macpro_init_hook(struct hda_codec *codec)
{
	alc882_gpio_mute(codec, 0, 0);
	alc882_gpio_mute(codec, 1, 0);
}

/* set up GPIO and update auto-muting at initialization */
static void alc885_imac24_init_hook(struct hda_codec *codec)
{
	alc885_macpro_init_hook(codec);
	alc_hp_automute(codec);
}

/* 2ch mode (Speaker:front, Subwoofer:CLFE, Line:input, Headphones:front) */
static const struct hda_verb alc889A_mb31_ch2_init[] = {
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x00},             /* HP as front */
	{0x16, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE}, /* Subwoofer on */
	{0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},    /* Line as input */
	{0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},   /* Line off */
	{ } /* end */
};

/* 4ch mode (Speaker:front, Subwoofer:CLFE, Line:CLFE, Headphones:front) */
static const struct hda_verb alc889A_mb31_ch4_init[] = {
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x00},             /* HP as front */
	{0x16, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE}, /* Subwoofer on */
	{0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},   /* Line as output */
	{0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE}, /* Line on */
	{ } /* end */
};

/* 5ch mode (Speaker:front, Subwoofer:CLFE, Line:input, Headphones:rear) */
static const struct hda_verb alc889A_mb31_ch5_init[] = {
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x01},             /* HP as rear */
	{0x16, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE}, /* Subwoofer on */
	{0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},    /* Line as input */
	{0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},   /* Line off */
	{ } /* end */
};

/* 6ch mode (Speaker:front, Subwoofer:off, Line:CLFE, Headphones:Rear) */
static const struct hda_verb alc889A_mb31_ch6_init[] = {
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x01},             /* HP as front */
	{0x16, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE},   /* Subwoofer off */
	{0x1a, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_OUT},   /* Line as output */
	{0x1a, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_UNMUTE}, /* Line on */
	{ } /* end */
};

static const struct hda_channel_mode alc889A_mb31_6ch_modes[4] = {
	{ 2, alc889A_mb31_ch2_init },
	{ 4, alc889A_mb31_ch4_init },
	{ 5, alc889A_mb31_ch5_init },
	{ 6, alc889A_mb31_ch6_init },
};

static const struct snd_kcontrol_new alc883_3ST_6ch_mixer[] = {
	HDA_CODEC_VOLUME("Front Playback Volume", 0x0c, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE("Front Playback Switch", 0x0c, 2, HDA_INPUT),
	HDA_CODEC_VOLUME("Surround Playback Volume", 0x0d, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE("Surround Playback Switch", 0x0d, 2, HDA_INPUT),
	HDA_CODEC_VOLUME_MONO("Center Playback Volume", 0x0e, 1, 0x0, HDA_OUTPUT),
	HDA_CODEC_VOLUME_MONO("LFE Playback Volume", 0x0e, 2, 0x0, HDA_OUTPUT),
	HDA_BIND_MUTE_MONO("Center Playback Switch", 0x0e, 1, 2, HDA_INPUT),
	HDA_BIND_MUTE_MONO("LFE Playback Switch", 0x0e, 2, 2, HDA_INPUT),
	HDA_CODEC_MUTE("Headphone Playback Switch", 0x1b, 0x0, HDA_OUTPUT),
	HDA_CODEC_VOLUME("CD Playback Volume", 0x0b, 0x04, HDA_INPUT),
	HDA_CODEC_MUTE("CD Playback Switch", 0x0b, 0x04, HDA_INPUT),
	HDA_CODEC_VOLUME("Line Playback Volume", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_MUTE("Line Playback Switch", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Playback Volume", 0x0b, 0x0, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Boost Volume", 0x18, 0, HDA_INPUT),
	HDA_CODEC_MUTE("Mic Playback Switch", 0x0b, 0x0, HDA_INPUT),
	HDA_CODEC_VOLUME("Front Mic Playback Volume", 0x0b, 0x1, HDA_INPUT),
	HDA_CODEC_VOLUME("Front Mic Boost Volume", 0x19, 0, HDA_INPUT),
	HDA_CODEC_MUTE("Front Mic Playback Switch", 0x0b, 0x1, HDA_INPUT),
	{ } /* end */
};

static const struct snd_kcontrol_new alc888_acer_aspire_6530_mixer[] = {
	HDA_CODEC_VOLUME("Front Playback Volume", 0x0c, 0x0, HDA_OUTPUT),
	HDA_CODEC_VOLUME("LFE Playback Volume", 0x0f, 0x0, HDA_OUTPUT),
	HDA_CODEC_VOLUME("Line Playback Volume", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_MUTE("Line Playback Switch", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("CD Playback Volume", 0x0b, 0x04, HDA_INPUT),
	HDA_CODEC_MUTE("CD Playback Switch", 0x0b, 0x04, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Playback Volume", 0x0b, 0x0, HDA_INPUT),
	HDA_CODEC_VOLUME("Mic Boost Volume", 0x18, 0, HDA_INPUT),
	HDA_CODEC_MUTE("Mic Playback Switch", 0x0b, 0x0, HDA_INPUT),
	{ } /* end */
};

static const struct snd_kcontrol_new alc889A_mb31_mixer[] = {
	/* Output mixers */
	HDA_CODEC_VOLUME("Front Playback Volume", 0x0c, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE("Front Playback Switch", 0x0c, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME("Surround Playback Volume", 0x0d, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE("Surround Playback Switch", 0x0d, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME_MONO("Center Playback Volume", 0x0e, 1, 0x00,
		HDA_OUTPUT),
	HDA_BIND_MUTE_MONO("Center Playback Switch", 0x0e, 1, 0x02, HDA_INPUT),
	HDA_CODEC_VOLUME_MONO("LFE Playback Volume", 0x0e, 2, 0x00, HDA_OUTPUT),
	HDA_BIND_MUTE_MONO("LFE Playback Switch", 0x0e, 2, 0x02, HDA_INPUT),
	/* Output switches */
	HDA_CODEC_MUTE("Enable Speaker", 0x14, 0x00, HDA_OUTPUT),
	HDA_CODEC_MUTE("Enable Headphones", 0x15, 0x00, HDA_OUTPUT),
	HDA_CODEC_MUTE_MONO("Enable LFE", 0x16, 2, 0x00, HDA_OUTPUT),
	/* Boost mixers */
	HDA_CODEC_VOLUME("Mic Boost Volume", 0x18, 0x00, HDA_INPUT),
	HDA_CODEC_VOLUME("Line Boost Volume", 0x1a, 0x00, HDA_INPUT),
	/* Input mixers */
	HDA_CODEC_VOLUME("Mic Playback Volume", 0x0b, 0x00, HDA_INPUT),
	HDA_CODEC_MUTE("Mic Playback Switch", 0x0b, 0x00, HDA_INPUT),
	HDA_CODEC_VOLUME("Line Playback Volume", 0x0b, 0x02, HDA_INPUT),
	HDA_CODEC_MUTE("Line Playback Switch", 0x0b, 0x02, HDA_INPUT),
	{ } /* end */
};

static const struct snd_kcontrol_new alc883_chmode_mixer[] = {
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "Channel Mode",
		.info = alc_ch_mode_info,
		.get = alc_ch_mode_get,
		.put = alc_ch_mode_put,
	},
	{ } /* end */
};

static const struct hda_verb alc889A_mb31_verbs[] = {
	/* Init rear pin (used as headphone output) */
	{0x15, AC_VERB_SET_PIN_WIDGET_CONTROL, 0xc4},    /* Apple Headphones */
	{0x15, AC_VERB_SET_CONNECT_SEL, 0x00},           /* Connect to front */
	{0x15, AC_VERB_SET_UNSOLICITED_ENABLE, ALC_HP_EVENT | AC_USRSP_EN},
	/* Init line pin (used as output in 4ch and 6ch mode) */
	{0x1a, AC_VERB_SET_CONNECT_SEL, 0x02},           /* Connect to CLFE */
	/* Init line 2 pin (used as headphone out by default) */
	{0x1b, AC_VERB_SET_PIN_WIDGET_CONTROL, PIN_IN},  /* Use as input */
	{0x1b, AC_VERB_SET_AMP_GAIN_MUTE, AMP_OUT_MUTE}, /* Mute output */
	{ } /* end */
};

/* Mute speakers according to the headphone jack state */
static void alc889A_mb31_automute(struct hda_codec *codec)
{
	unsigned int present;

	/* Mute only in 2ch or 4ch mode */
	if (snd_hda_codec_read(codec, 0x15, 0, AC_VERB_GET_CONNECT_SEL, 0)
	    == 0x00) {
		present = snd_hda_jack_detect(codec, 0x15);
		snd_hda_codec_amp_stereo(codec, 0x14,  HDA_OUTPUT, 0,
			HDA_AMP_MUTE, present ? HDA_AMP_MUTE : 0);
		snd_hda_codec_amp_stereo(codec, 0x16, HDA_OUTPUT, 0,
			HDA_AMP_MUTE, present ? HDA_AMP_MUTE : 0);
	}
}

static void alc889A_mb31_unsol_event(struct hda_codec *codec, unsigned int res)
{
	if ((res >> 26) == ALC_HP_EVENT)
		alc889A_mb31_automute(codec);
}

/*
 * configuration and preset
 */
static const char * const alc882_models[ALC882_MODEL_LAST] = {
	[ALC885_MACPRO]		= "macpro",
	[ALC885_MB5]		= "mb5",
	[ALC885_MACMINI3]	= "macmini3",
	[ALC885_MBA21]		= "mba21",
	[ALC885_MBP3]		= "mbp3",
	[ALC885_IMAC24]		= "imac24",
	[ALC885_IMAC91]		= "imac91",
	[ALC888_ACER_ASPIRE_4930G]	= "acer-aspire-4930g",
	[ALC888_ACER_ASPIRE_6530G]	= "acer-aspire-6530g",
	[ALC888_ACER_ASPIRE_8930G]	= "acer-aspire-8930g",
	[ALC888_ACER_ASPIRE_7730G]	= "acer-aspire-7730g",
	[ALC889A_MB31]		= "mb31",
	[ALC882_AUTO]		= "auto",
};

static const struct snd_pci_quirk alc882_cfg_tbl[] = {
	SND_PCI_QUIRK(0x1025, 0x013e, "Acer Aspire 4930G",
		ALC888_ACER_ASPIRE_4930G),
	SND_PCI_QUIRK(0x1025, 0x013f, "Acer Aspire 5930G",
		ALC888_ACER_ASPIRE_4930G),
	SND_PCI_QUIRK(0x1025, 0x0145, "Acer Aspire 8930G",
		ALC888_ACER_ASPIRE_8930G),
	SND_PCI_QUIRK(0x1025, 0x0146, "Acer Aspire 6935G",
		ALC888_ACER_ASPIRE_8930G),
	SND_PCI_QUIRK(0x1025, 0x015e, "Acer Aspire 6930G",
		ALC888_ACER_ASPIRE_6530G),
	SND_PCI_QUIRK(0x1025, 0x0166, "Acer Aspire 6530G",
		ALC888_ACER_ASPIRE_6530G),
	SND_PCI_QUIRK(0x1025, 0x0142, "Acer Aspire 7730G",
		ALC888_ACER_ASPIRE_7730G),
	{}
};

/* codec SSID table for Intel Mac */
static const struct snd_pci_quirk alc882_ssid_cfg_tbl[] = {
	SND_PCI_QUIRK(0x106b, 0x00a0, "MacBookPro 3,1", ALC885_MBP3),
	SND_PCI_QUIRK(0x106b, 0x00a1, "Macbook", ALC885_MBP3),
	SND_PCI_QUIRK(0x106b, 0x00a4, "MacbookPro 4,1", ALC885_MBP3),
	SND_PCI_QUIRK(0x106b, 0x0c00, "Mac Pro", ALC885_MACPRO),
	SND_PCI_QUIRK(0x106b, 0x1000, "iMac 24", ALC885_IMAC24),
	SND_PCI_QUIRK(0x106b, 0x2800, "AppleTV", ALC885_IMAC24),
	SND_PCI_QUIRK(0x106b, 0x2c00, "MacbookPro rev3", ALC885_MBP3),
	SND_PCI_QUIRK(0x106b, 0x3000, "iMac", ALC889A_MB31),
	SND_PCI_QUIRK(0x106b, 0x3400, "MacBookAir 1,1", ALC885_MBP3),
	SND_PCI_QUIRK(0x106b, 0x3500, "MacBookAir 2,1", ALC885_MBA21),
	SND_PCI_QUIRK(0x106b, 0x3600, "Macbook 3,1", ALC889A_MB31),
	SND_PCI_QUIRK(0x106b, 0x3800, "MacbookPro 4,1", ALC885_MBP3),
	SND_PCI_QUIRK(0x106b, 0x3e00, "iMac 24 Aluminum", ALC885_IMAC24),
	SND_PCI_QUIRK(0x106b, 0x4900, "iMac 9,1 Aluminum", ALC885_IMAC91),
	SND_PCI_QUIRK(0x106b, 0x3f00, "Macbook 5,1", ALC885_MB5),
	SND_PCI_QUIRK(0x106b, 0x4a00, "Macbook 5,2", ALC885_MB5),
	/* FIXME: HP jack sense seems not working for MBP 5,1 or 5,2,
	 * so apparently no perfect solution yet
	 */
	SND_PCI_QUIRK(0x106b, 0x4000, "MacbookPro 5,1", ALC885_MB5),
	SND_PCI_QUIRK(0x106b, 0x4600, "MacbookPro 5,2", ALC885_MB5),
	SND_PCI_QUIRK(0x106b, 0x4100, "Macmini 3,1", ALC885_MACMINI3),
	{} /* terminator */
};

static const struct alc_config_preset alc882_presets[] = {
	   [ALC885_MBA21] = {
			.mixers = { alc885_mba21_mixer },
			.init_verbs = { alc885_mba21_init_verbs, alc880_gpio1_init_verbs },
			.num_dacs = 2,
			.dac_nids = alc882_dac_nids,
			.channel_mode = alc885_mba21_ch_modes,
			.num_channel_mode = ARRAY_SIZE(alc885_mba21_ch_modes),
			.input_mux = &alc882_capture_source,
			.unsol_event = alc_sku_unsol_event,
			.setup = alc885_mba21_setup,
			.init_hook = alc_hp_automute,
       },
	[ALC885_MBP3] = {
		.mixers = { alc885_mbp3_mixer, alc882_chmode_mixer },
		.init_verbs = { alc885_mbp3_init_verbs,
				alc880_gpio1_init_verbs },
		.num_dacs = 2,
		.dac_nids = alc882_dac_nids,
		.hp_nid = 0x04,
		.channel_mode = alc885_mbp_4ch_modes,
		.num_channel_mode = ARRAY_SIZE(alc885_mbp_4ch_modes),
		.input_mux = &alc882_capture_source,
		.dig_out_nid = ALC882_DIGOUT_NID,
		.dig_in_nid = ALC882_DIGIN_NID,
		.unsol_event = alc_sku_unsol_event,
		.setup = alc885_mbp3_setup,
		.init_hook = alc_hp_automute,
	},
	[ALC885_MB5] = {
		.mixers = { alc885_mb5_mixer, alc882_chmode_mixer },
		.init_verbs = { alc885_mb5_init_verbs,
				alc880_gpio1_init_verbs },
		.num_dacs = ARRAY_SIZE(alc882_dac_nids),
		.dac_nids = alc882_dac_nids,
		.channel_mode = alc885_mb5_6ch_modes,
		.num_channel_mode = ARRAY_SIZE(alc885_mb5_6ch_modes),
		.input_mux = &mb5_capture_source,
		.dig_out_nid = ALC882_DIGOUT_NID,
		.dig_in_nid = ALC882_DIGIN_NID,
		.unsol_event = alc_sku_unsol_event,
		.setup = alc885_mb5_setup,
		.init_hook = alc_hp_automute,
	},
	[ALC885_MACMINI3] = {
		.mixers = { alc885_macmini3_mixer, alc882_chmode_mixer },
		.init_verbs = { alc885_macmini3_init_verbs,
				alc880_gpio1_init_verbs },
		.num_dacs = ARRAY_SIZE(alc882_dac_nids),
		.dac_nids = alc882_dac_nids,
		.channel_mode = alc885_macmini3_6ch_modes,
		.num_channel_mode = ARRAY_SIZE(alc885_macmini3_6ch_modes),
		.input_mux = &macmini3_capture_source,
		.dig_out_nid = ALC882_DIGOUT_NID,
		.dig_in_nid = ALC882_DIGIN_NID,
		.unsol_event = alc_sku_unsol_event,
		.setup = alc885_macmini3_setup,
		.init_hook = alc_hp_automute,
	},
	[ALC885_MACPRO] = {
		.mixers = { alc882_macpro_mixer },
		.init_verbs = { alc882_macpro_init_verbs },
		.num_dacs = ARRAY_SIZE(alc882_dac_nids),
		.dac_nids = alc882_dac_nids,
		.dig_out_nid = ALC882_DIGOUT_NID,
		.dig_in_nid = ALC882_DIGIN_NID,
		.num_channel_mode = ARRAY_SIZE(alc882_ch_modes),
		.channel_mode = alc882_ch_modes,
		.input_mux = &alc882_capture_source,
		.init_hook = alc885_macpro_init_hook,
	},
	[ALC885_IMAC24] = {
		.mixers = { alc885_imac24_mixer },
		.init_verbs = { alc885_imac24_init_verbs },
		.num_dacs = ARRAY_SIZE(alc882_dac_nids),
		.dac_nids = alc882_dac_nids,
		.dig_out_nid = ALC882_DIGOUT_NID,
		.dig_in_nid = ALC882_DIGIN_NID,
		.num_channel_mode = ARRAY_SIZE(alc882_ch_modes),
		.channel_mode = alc882_ch_modes,
		.input_mux = &alc882_capture_source,
		.unsol_event = alc_sku_unsol_event,
		.setup = alc885_imac24_setup,
		.init_hook = alc885_imac24_init_hook,
	},
	[ALC885_IMAC91] = {
		.mixers = {alc885_imac91_mixer},
		.init_verbs = { alc885_imac91_init_verbs,
				alc880_gpio1_init_verbs },
		.num_dacs = ARRAY_SIZE(alc882_dac_nids),
		.dac_nids = alc882_dac_nids,
		.channel_mode = alc885_mba21_ch_modes,
		.num_channel_mode = ARRAY_SIZE(alc885_mba21_ch_modes),
		.input_mux = &alc889A_imac91_capture_source,
		.dig_out_nid = ALC882_DIGOUT_NID,
		.dig_in_nid = ALC882_DIGIN_NID,
		.unsol_event = alc_sku_unsol_event,
		.setup = alc885_imac91_setup,
		.init_hook = alc_hp_automute,
	},
	[ALC888_ACER_ASPIRE_4930G] = {
		.mixers = { alc888_acer_aspire_4930g_mixer,
				alc883_chmode_mixer },
		.init_verbs = { alc883_init_verbs, alc880_gpio1_init_verbs,
				alc888_acer_aspire_4930g_verbs },
		.num_dacs = ARRAY_SIZE(alc883_dac_nids),
		.dac_nids = alc883_dac_nids,
		.num_adc_nids = ARRAY_SIZE(alc883_adc_nids_rev),
		.adc_nids = alc883_adc_nids_rev,
		.capsrc_nids = alc883_capsrc_nids_rev,
		.dig_out_nid = ALC883_DIGOUT_NID,
		.num_channel_mode = ARRAY_SIZE(alc883_3ST_6ch_modes),
		.channel_mode = alc883_3ST_6ch_modes,
		.need_dac_fix = 1,
		.const_channel_count = 6,
		.num_mux_defs =
			ARRAY_SIZE(alc888_2_capture_sources),
		.input_mux = alc888_2_capture_sources,
		.unsol_event = alc_sku_unsol_event,
		.setup = alc888_acer_aspire_4930g_setup,
		.init_hook = alc_hp_automute,
	},
	[ALC888_ACER_ASPIRE_6530G] = {
		.mixers = { alc888_acer_aspire_6530_mixer },
		.init_verbs = { alc883_init_verbs, alc880_gpio1_init_verbs,
				alc888_acer_aspire_6530g_verbs },
		.num_dacs = ARRAY_SIZE(alc883_dac_nids),
		.dac_nids = alc883_dac_nids,
		.num_adc_nids = ARRAY_SIZE(alc883_adc_nids_rev),
		.adc_nids = alc883_adc_nids_rev,
		.capsrc_nids = alc883_capsrc_nids_rev,
		.dig_out_nid = ALC883_DIGOUT_NID,
		.num_channel_mode = ARRAY_SIZE(alc883_3ST_2ch_modes),
		.channel_mode = alc883_3ST_2ch_modes,
		.num_mux_defs =
			ARRAY_SIZE(alc888_2_capture_sources),
		.input_mux = alc888_acer_aspire_6530_sources,
		.unsol_event = alc_sku_unsol_event,
		.setup = alc888_acer_aspire_6530g_setup,
		.init_hook = alc_hp_automute,
	},
	[ALC888_ACER_ASPIRE_8930G] = {
		.mixers = { alc889_acer_aspire_8930g_mixer,
				alc883_chmode_mixer },
		.init_verbs = { alc883_init_verbs, alc880_gpio1_init_verbs,
				alc889_acer_aspire_8930g_verbs,
				alc889_eapd_verbs},
		.num_dacs = ARRAY_SIZE(alc883_dac_nids),
		.dac_nids = alc883_dac_nids,
		.num_adc_nids = ARRAY_SIZE(alc889_adc_nids),
		.adc_nids = alc889_adc_nids,
		.capsrc_nids = alc889_capsrc_nids,
		.dig_out_nid = ALC883_DIGOUT_NID,
		.num_channel_mode = ARRAY_SIZE(alc883_3ST_6ch_modes),
		.channel_mode = alc883_3ST_6ch_modes,
		.need_dac_fix = 1,
		.const_channel_count = 6,
		.num_mux_defs =
			ARRAY_SIZE(alc889_capture_sources),
		.input_mux = alc889_capture_sources,
		.unsol_event = alc_sku_unsol_event,
		.setup = alc889_acer_aspire_8930g_setup,
		.init_hook = alc_hp_automute,
#ifdef CONFIG_SND_HDA_POWER_SAVE
		.power_hook = alc_power_eapd,
#endif
	},
	[ALC888_ACER_ASPIRE_7730G] = {
		.mixers = { alc883_3ST_6ch_mixer,
				alc883_chmode_mixer },
		.init_verbs = { alc883_init_verbs, alc880_gpio1_init_verbs,
				alc888_acer_aspire_7730G_verbs },
		.num_dacs = ARRAY_SIZE(alc883_dac_nids),
		.dac_nids = alc883_dac_nids,
		.num_adc_nids = ARRAY_SIZE(alc883_adc_nids_rev),
		.adc_nids = alc883_adc_nids_rev,
		.capsrc_nids = alc883_capsrc_nids_rev,
		.dig_out_nid = ALC883_DIGOUT_NID,
		.num_channel_mode = ARRAY_SIZE(alc883_3ST_6ch_modes),
		.channel_mode = alc883_3ST_6ch_modes,
		.need_dac_fix = 1,
		.const_channel_count = 6,
		.input_mux = &alc883_capture_source,
		.unsol_event = alc_sku_unsol_event,
		.setup = alc888_acer_aspire_7730g_setup,
		.init_hook = alc_hp_automute,
	},
	[ALC889A_MB31] = {
		.mixers = { alc889A_mb31_mixer, alc883_chmode_mixer},
		.init_verbs = { alc883_init_verbs, alc889A_mb31_verbs,
			alc880_gpio1_init_verbs },
		.adc_nids = alc883_adc_nids,
		.num_adc_nids = ARRAY_SIZE(alc883_adc_nids),
		.capsrc_nids = alc883_capsrc_nids,
		.dac_nids = alc883_dac_nids,
		.num_dacs = ARRAY_SIZE(alc883_dac_nids),
		.channel_mode = alc889A_mb31_6ch_modes,
		.num_channel_mode = ARRAY_SIZE(alc889A_mb31_6ch_modes),
		.input_mux = &alc889A_mb31_capture_source,
		.dig_out_nid = ALC883_DIGOUT_NID,
		.unsol_event = alc889A_mb31_unsol_event,
		.init_hook = alc889A_mb31_automute,
	},
};


