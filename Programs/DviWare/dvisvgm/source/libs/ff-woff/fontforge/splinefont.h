/* Copyright (C) 2000-2012 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _SPLINEFONT_H
#define _SPLINEFONT_H

#include <basics.h>
#include <dlist.h>
#include "configure-fontforge.h"
#ifdef HAVE_ICONV
# include <iconv.h>
#else
# include <gwwiconv.h>
#endif
#include "locale.h"
#include <gnetwork.h>

#ifdef FONTFORGE_CONFIG_USE_DOUBLE
# define real		double
# define bigreal	double
#else
# define real		float
# define bigreal	double
#endif

#define extended	double
	/* Solaris wants to define extended to be unsigned [3] unless we do this*/
#define _EXTENDED

#define CHR(ch1,ch2,ch3,ch4) (((ch1)<<24)|((ch2)<<16)|((ch3)<<8)|(ch4))

#define MmMax		16	/* PS says at most this many instances for type1/2 mm fonts */
#define AppleMmMax	26	/* Apple sort of has a limit of 4095, but we only support this many */

typedef struct ipoint {
    int x;
    int y;
} IPoint;

#define IPOINT_EMPTY { 0, 0 }


typedef struct basepoint {
    real x;
    real y;
} BasePoint;

#define BASEPOINT_EMPTY { (real)0.0, (real)0.0 }


typedef struct dbasepoint {
    bigreal x;
    bigreal y;
} DBasePoint;

#define DBASEPOINT_EMPTY { (bigreal)0.0, (bigreal)0.0 }


typedef struct tpoint {
    real x;
    real y;
    real t;
} TPoint;

#define TPOINT_EMPTY { (real)0.0, (real)0.0, (real)0.0 }


typedef struct dbounds {
    real minx, maxx;
    real miny, maxy;
} DBounds;

#define DBOUNDS_EMPTY { (real)0.0, (real)0.0, (real)0.0, (real)0.0 }


typedef struct ibounds {
    int minx, maxx;
    int miny, maxy;
} IBounds;

#define IBOUNDS_EMPTY { 0, 0, 0, 0 }


enum val_type { v_int, v_real, v_str, v_unicode, v_lval, v_arr, v_arrfree,
		v_int32pt, v_int16pt, v_int8pt, v_void };

typedef struct val {
    enum val_type type;
    union {
	int ival;
	real fval;
	char *sval;
	struct val *lval;
	struct array *aval;
	uint32 *u32ptval;
	uint16 *u16ptval;
	uint8  *u8ptval;
    } u;
} Val;		/* Used by scripting */

struct psdict {
    int cnt;		/* *key[0..cnt] and *values[0..cnt] currently available */
    int next;		/* **key[0..next] and **values[0..next] currently used  */
    char **keys;
    char **values;
};

struct pschars {
    int cnt, next;
    char **keys;
    uint8 **values;
    int *lens;
    int bias;		/* for type2 strings */
};

enum linejoin {
    lj_miter,		/* Extend lines until they meet */
    lj_round,		/* circle centered at the join of expand radius */
    lj_bevel,		/* Straight line between the ends of next and prev */
    lj_inherited
};
enum linecap {
    lc_butt,		/* equiv to lj_bevel, straight line extends from one side to other */
    lc_round,		/* semi-circle */
    lc_square,		/* Extend lines by radius, then join them */
    lc_inherited
};
enum spreadMethod {
    sm_pad, sm_reflect, sm_repeat
};

#define COLOR_INHERITED	0xfffffffe

struct grad_stops {
    real offset;
    uint32 col;
    real opacity;
};

struct gradient {
    BasePoint start;	/* focal of a radial gradient, start of a linear */
    BasePoint stop;	/* center of a radial gradient, end of a linear */
    real radius;	/* 0=>linear gradient, else radius of a radial gradient */
    enum spreadMethod sm;
    int stop_cnt;
    struct grad_stops *grad_stops;
};

struct pattern {
    char *pattern;
    real width, height;		/* Pattern is scaled to be repeated every width/height (in user coordinates) */
    real transform[6];
    /* Used during rasterization process */
    struct bdfchar *pat;
    real invtrans[6];
    int bminx, bminy, bwidth, bheight;	/* of the pattern at bdfchar scale */
};

struct brush {
    uint32 col;
    float opacity;		/* number between [0,1], only for svg/pdf */
    struct pattern *pattern;	/* A pattern to be tiled */
    struct gradient *gradient;	/* A gradient fill */
};
#define WIDTH_INHERITED	(-1)
#define DASH_INHERITED	255	/* if the dashes[0]==0 && dashes[1]==DASH_INHERITED */
#define DASH_MAX	8
typedef unsigned char DashType;
struct pen {
    struct brush brush;
    uint8 linejoin;
    uint8 linecap;
    float width;
    real trans[4];
    DashType dashes[DASH_MAX];
};

struct spline;
enum si_type { si_std, si_caligraphic, si_poly, si_centerline };
/* If you change this structure you may need to update MakeStrokeDlg */
/*  and cvpalettes.c both contain statically initialized StrokeInfos */
typedef struct strokeinfo {
    real radius;			/* or major axis of pen */
    enum linejoin join;
    enum linecap cap;
    enum si_type stroke_type;
    unsigned int removeinternal: 1;
    unsigned int removeexternal: 1;
    unsigned int leave_users_center: 1;			/* Don't move the pen so its center is at the origin */
    real penangle;
    real minorradius;
    struct splinepointlist *poly;
    real resolution;
/* For freehand tool */
    real radius2;
    int pressure1, pressure2;
/* End freehand tool */
    void *data;
    bigreal (*factor)(void *data,struct spline *spline,real t);
} StrokeInfo;

enum PolyType { Poly_Convex, Poly_Concave, Poly_PointOnEdge,
    Poly_TooFewPoints, Poly_Line };


enum overlap_type { over_remove, over_rmselected, over_intersect, over_intersel,
	over_exclude, over_findinter, over_fisel };

enum simpify_flags { sf_cleanup=-1, sf_normal=0, sf_ignoreslopes=1,
	sf_ignoreextremum=2, sf_smoothcurves=4, sf_choosehv=8,
	sf_forcelines=0x10, sf_nearlyhvlines=0x20,
	sf_mergelines=0x40, sf_setstart2extremum=0x80,
	sf_rmsingletonpoints=0x100 };

struct hsquash { double lsb_percent, stem_percent, counter_percent, rsb_percent; };

enum serif_type { srf_flat, srf_simpleslant, srf_complexslant };
/* |    | (flat)    |   | (simple)     |    | (complex) */
/* |    |           |  /               |   /            */
/* |    |           | /                |  /             */
/* +----+           |/                 \ /              */

typedef struct italicinfo {
    double italic_angle;
    double xheight_percent;
    struct hsquash lc, uc, neither;
    enum serif_type secondary_serif;

    unsigned int transform_bottom_serifs: 1;
    unsigned int transform_top_xh_serifs: 1;	/* Those at x-height */
    unsigned int transform_top_as_serifs: 1;	/* Those at ascender-height */
    unsigned int transform_diagon_serifs: 1;	/* Those at baseline/xheight */

    unsigned int a_from_d: 1;		/* replace the "a" glyph with the variant which looks like a "d" without an ascender */
  /* When I say "f" I also mean "f_f" ligature, "longs", cyrillic phi and other things shaped like "f" */
    unsigned int f_long_tail: 1;	/* Some Italic fonts have the "f" grow an extension of the main stem below the baseline */
    unsigned int f_rotate_top: 1;	/* Most Italic fonts take the top curve of the "f", rotate it 180 and attach to the bottom */
    unsigned int pq_deserif: 1;		/* Remove a serif from the descender of p or q and replace with a secondary serif as above */

  /* Unsupported */
    /* e becomes rounder, cross bar slightly slanted */
    /* g closed counter at bottom */
    /* k closed counter at top */
    /* v-z diagonal stems become more curvatious */

    unsigned int cyrl_phi: 1;		/* Gains an "f" like top, bottom treated like "f" */
    unsigned int cyrl_i: 1;		/* Turns into a latin u */
    unsigned int cyrl_pi: 1;		/* Turns into a latin n */
    unsigned int cyrl_te: 1;		/* Turns into a latin m */
    unsigned int cyrl_sha: 1;		/* Turns into a latin m rotated 180 */
    unsigned int cyrl_dje: 1;		/* Turns into a latin smallcaps T */
    unsigned int cyrl_dzhe: 1;		/* Turns into a latin u */
		    /* Is there a difference between dzhe and i? both look like u to me */

  /* Unsupported */
    /* u432 curved B */
    /* u433 strange gamma */
    /* u434 normal delta */
    /* u436 */
    /* u43b lambda ? */
    /* u43c */
    /* u446 */
    /* u449 */
    /* u449 */
    /* u44a */

/* This half of the structure gets filled in later - see ITALICINFO_REMAINDER */
    double tan_ia;
    double x_height;
    double pq_depth;
    double ascender_height;
    double emsize;
    int order2;
    struct splinefont *sf;
    int layer;
    double serif_extent, serif_height;
    struct splinepoint *f_start, *f_end;		/* start has next pointing into the f head and up */
    struct splinepoint *ff_start1, *ff_end1, *ff_start2, *ff_end2;
    double f_height, ff_height;
} ItalicInfo;

#define ITALICINFO_REMAINDER 0, 0, 0, 0, 0, 0, NULL, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0


typedef struct bluedata {
    real xheight, xheighttop;		/* height of "x" and "o" (u,v,w,x,y,z) */
    real caph, caphtop;			/* height of "I" and "O" */
    real base, basebelow;		/* bottom of "I" and "O" */
    real ascent;			/* height of "l" */
    real descent;			/* depth of "p" */
    real numh, numhtop;			/* height of "7" and "8" */ /* numbers with ascenders */
    int bluecnt;			/* If the private dica contains bluevalues... */
    real blues[12][2];			/* 7 pairs from bluevalues, 5 from otherblues */
} BlueData;

#define BLUEDATA_EMPTY { \
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, \
    { { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, \
      { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 0.0 }  \
    } \
}


typedef struct bdffloat {
    int16 xmin,xmax,ymin,ymax;
    int16 bytes_per_line;
    unsigned int byte_data:1;
    uint8 depth;
    uint8 *bitmap;
} BDFFloat;

/* OpenType does not document 'dflt' as a language, but we'll use it anyway. */
/* (Adobe uses it too) we'll turn it into a default entry when we output it. */
#define DEFAULT_LANG		CHR('d','f','l','t')
/* The OpenType spec says in one place that the default script is 'dflt' and */
/*  in another that it is 'DFLT'. 'DFLT' is correct */
#define DEFAULT_SCRIPT		CHR('D','F','L','T')
#define REQUIRED_FEATURE	CHR(' ','R','Q','D')

enum otlookup_type {
    ot_undef = 0,			/* Not a lookup type */
    gsub_start	       = 0x000,		/* Not a lookup type */
    gsub_single        = 0x001,
    gsub_multiple      = 0x002,
    gsub_alternate     = 0x003,
    gsub_ligature      = 0x004,
    gsub_context       = 0x005,
    gsub_contextchain  = 0x006,
     /* GSUB extension 7 */
    gsub_reversecchain = 0x008,
    /* mac state machines */
    morx_indic	       = 0x0fd,
    morx_context       = 0x0fe,
    morx_insert        = 0x0ff,
    /* ********************* */
    gpos_start         = 0x100,		/* Not a lookup type */

    gpos_single        = 0x101,
    gpos_pair          = 0x102,
    gpos_cursive       = 0x103,
    gpos_mark2base     = 0x104,
    gpos_mark2ligature = 0x105,
    gpos_mark2mark     = 0x106,
    gpos_context       = 0x107,
    gpos_contextchain  = 0x108,
    /* GPOS extension 9 */
    kern_statemachine  = 0x1ff

    /* otlookup&0xff == lookup type for the appropriate table */
    /* otlookup>>8:     0=>GSUB, 1=>GPOS */
};

enum otlookup_typemasks {
    gsub_single_mask        = 0x00001,
    gsub_multiple_mask      = 0x00002,
    gsub_alternate_mask     = 0x00004,
    gsub_ligature_mask      = 0x00008,
    gsub_context_mask       = 0x00010,
    gsub_contextchain_mask  = 0x00020,
    gsub_reversecchain_mask = 0x00040,
    morx_indic_mask         = 0x00080,
    morx_context_mask       = 0x00100,
    morx_insert_mask        = 0x00200,
    /* ********************* */
    gpos_single_mask        = 0x00400,
    gpos_pair_mask          = 0x00800,
    gpos_cursive_mask       = 0x01000,
    gpos_mark2base_mask     = 0x02000,
    gpos_mark2ligature_mask = 0x04000,
    gpos_mark2mark_mask     = 0x08000,
    gpos_context_mask       = 0x10000,
    gpos_contextchain_mask  = 0x20000,
    kern_statemachine_mask  = 0x40000
};

#define MAX_LANG 		4	/* If more than this we allocate more_langs in chunks of MAX_LANG */
struct scriptlanglist {
    uint32 script;
    uint32 langs[MAX_LANG];
    uint32 *morelangs;
    int lang_cnt;
    struct scriptlanglist *next;
};

#define OPENTYPE_FEATURE_FRIENDLYNAMES_EMPTY { 0, NULL, NULL, 0 }


typedef struct featurescriptlanglist {
    uint32 featuretag;
    struct scriptlanglist *scripts;
    struct featurescriptlanglist *next;
    unsigned int ismac: 1;	/* treat the featuretag as a mac feature/setting */
} FeatureScriptLangList;

enum pst_flags { pst_r2l=1, pst_ignorebaseglyphs=2, pst_ignoreligatures=4,
	pst_ignorecombiningmarks=8, pst_usemarkfilteringset=0x10,
	pst_markclass=0xff00, pst_markset=0xffff0000 };

struct lookup_subtable {
    char *subtable_name;
    char *suffix;			/* for gsub_single, used to find a default replacement */
    int16 separation, minkern;	/* for gpos_pair, used to guess default kerning values */
    struct otlookup *lookup;
    unsigned int unused: 1;
    unsigned int per_glyph_pst_or_kern: 1;
    unsigned int anchor_classes: 1;
    unsigned int vertical_kerning: 1;
    unsigned int ticked: 1;
    unsigned int kerning_by_touch: 1;	/* for gpos_pair, calculate kerning so that glyphs will touch */
    unsigned int onlyCloser: 1;		/* for kerning classes */
    unsigned int dontautokern: 1;		/* for kerning classes */
    struct kernclass *kc;
    struct generic_fpst *fpst;
    struct generic_asm  *sm;
    /* Each time an item is added to a lookup we must place it into a */
    /*  subtable. If it's a kerning class, fpst or state machine it has */
    /*  a subtable all to itself. If it's an anchor class it can share */
    /*  a subtable with other anchor classes (merge with). If it's a glyph */
    /*  PST it may share a subtable with other PSTs */
    /* Note items may only be placed in lookups in which they fit. Can't */
    /*  put kerning data in a gpos_single lookup, etc. */
    struct lookup_subtable *next;
    int32 subtable_offset;
    int32 *extra_subtables;
    /* If a kerning subtable has too much stuff in it, we are prepared to */
    /*  break it up into several smaller subtables, each of which has */
    /*  an offset in this list (extra-subtables[0]==subtable_offset) */
    /*  the list is terminated by an entry of -1 */
};

typedef struct otlookup {
    struct otlookup *next;
    enum otlookup_type lookup_type;
    uint32 lookup_flags;		/* Low order: traditional flags, High order: markset index, only meaningful if pst_usemarkfilteringset set */
    char *lookup_name;
    FeatureScriptLangList *features;
    struct lookup_subtable *subtables;
    unsigned int unused: 1;	/* No subtable is used (call SFFindUnusedLookups before examining) */
    unsigned int empty: 1;	/* No subtable is used, and no anchor classes are used */
    unsigned int store_in_afm: 1;	/* Used for ligatures, some get stored */
    					/*  'liga' generally does, but 'frac' doesn't */
    unsigned int needs_extension: 1;	/* Used during opentype generation */
    unsigned int temporary_kern: 1;	/* Used when decomposing kerning classes into kern pairs for older formats */
    unsigned int def_lang_checked: 1;
    unsigned int def_lang_found: 1;
    unsigned int ticked: 1;
    unsigned int in_gpos: 1;
    unsigned int in_jstf: 1;
    unsigned int only_jstf: 1;
    int16 subcnt;		/* Actual number of subtables we will output */
				/* Some of our subtables may contain no data */
			        /* Some may be too big and need to be broken up.*/
			        /* So this field may be different than just counting the subtables */
    int lookup_index;		/* used during opentype generation */
    uint32 lookup_offset;
    uint32 lookup_length;
    char *tempname;
} OTLookup;

#define LOOKUP_SUBTABLE_EMPTY { NULL, NULL, 0, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, 0, NULL }
#define OTLOOKUP_EMPTY { NULL, 0, 0, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL }


typedef struct devicetab {
    uint16 first_pixel_size, last_pixel_size;		/* A range of point sizes to which this table applies */
    int8 *corrections;					/* a set of pixel corrections, one for each point size */
} DeviceTable;

typedef struct valdev {		/* Value records can have four associated device tables */
    DeviceTable xadjust;
    DeviceTable yadjust;
    DeviceTable xadv;
    DeviceTable yadv;
} ValDevTab;

enum anchorclass_type { act_mark, act_mkmk, act_curs, act_mklg, act_unknown };
typedef struct anchorclass {
    char *name;			/* in utf8 */
    struct lookup_subtable *subtable;
    uint8 type;		/* anchorclass_type */
    uint8 has_base;
    uint8 processed, has_mark, matches, ac_num;
    uint8 ticked;
    struct anchorclass *next;
} AnchorClass;

enum anchor_type { at_mark, at_basechar, at_baselig, at_basemark, at_centry, at_cexit, at_max };
typedef struct anchorpoint {
    AnchorClass *anchor;
    BasePoint me;
    DeviceTable xadjust, yadjust;
    unsigned int type: 4;
    unsigned int selected: 1;
    unsigned int ticked: 1;
    unsigned int has_ttf_pt: 1;
    uint16 ttf_pt_index;
    int16  lig_index;
    struct anchorpoint *next;
} AnchorPoint;

typedef struct kernpair {
    // Note that the left character in the pair has the reference to the kerning pair, which in turn references the right character.
    struct lookup_subtable *subtable;
    struct splinechar *sc;
    int16 off;
    uint16 kcid;			/* temporary value */
    DeviceTable *adjust;		/* Only adjustment in one dimen, if more needed use pst */
    struct kernpair *next;
} KernPair;

#define FF_KERNCLASS_FLAG_NATIVE 2 // If set, the class goes into groups.plist or kerning.plist.
#define FF_KERNCLASS_FLAG_FEATURE 4 // If set, the class or rule goes into the feature file. In the present configuration, this ought to be zero always.
#define FF_KERNCLASS_FLAG_NAMETYPE 8 // If unset (default), the class has a standard name, which translates to a U. F. O. name starting in public.kern, which may be illegal in the feature file. If set, it has a name like @MMK_.
#define FF_KERNCLASS_FLAG_NAMELEGACY 16 // If set, the class has a U. F. O. name starting in @kc as FontForge liked to do in the past.
#define FF_KERNCLASS_FLAG_VIRTUAL 32 // If unset (default), the class is a real character class and does not conflict with same-sided classes. If set, FontForge mostly ignores the class except for U. F. O. input/output.
#define FF_KERNCLASS_FLAG_FLATTEN 64 // If unset (default), the class gets exported as a class. If set, it gets exported as its first member (in order to support class-character kerns).
#define FF_KERNCLASS_FLAG_SINGLECHAR (FF_KERNCLASS_FLAG_VIRTUAL | FF_KERNCLASS_FLAG_FLATTEN) // We expect to see these used together.

typedef struct kernclass {
    int first_cnt, second_cnt;		/* Count of classes for first and second chars */
    char **firsts;			/* list of a space separated list of char names */
    char **seconds;			/*  one entry for each class. Entry 0 is null */
    					/*  and means everything not specified elsewhere */
    char **firsts_names; // We need to track the names of the classes in order to round-trip U. F. O. data.
    char **seconds_names;
    int *firsts_flags; // This tracks the storage format of the class in U. F. O. (groups.plist or features.fea) and whether it's a single-character class.
    int *seconds_flags; // We also track the name format (@MMK or public.kern).
    struct lookup_subtable *subtable;
    uint16 kcid;			/* Temporary value, used for many things briefly */
    int16 *offsets;			/* array of first_cnt*second_cnt entries with 0 representing no data */
    int *offsets_flags;
    DeviceTable *adjusts;		/* array of first_cnt*second_cnt entries representing resolution-specific adjustments */
    struct kernclass *next;		// Note that, in most cases, a typeface needs only one struct kernclass since it can contain all classes.
    int feature; // This indicates whether the kerning class came from a feature file. This is important during export.
} KernClass;

enum possub_type { pst_null, pst_position, pst_pair,
	pst_substitution, pst_alternate,
	pst_multiple, pst_ligature,
	pst_lcaret /* must be pst_max-1, see charinfo.c*/,
	pst_max,
	/* These are not psts but are related so it's handly to have values for them */
	pst_kerning = pst_max, pst_vkerning, pst_anchors,
	/* And these are fpsts */
	pst_contextpos, pst_contextsub, pst_chainpos, pst_chainsub,
	pst_reversesub, fpst_max,
	/* And these are used to specify a kerning pair where the current */
	/*  char is the final glyph rather than the initial one */
	/* A kludge used when cutting and pasting features */
	pst_kernback, pst_vkernback
	};

struct vr {
    int16 xoff, yoff, h_adv_off, v_adv_off;
    ValDevTab *adjust;
};

typedef struct generic_pst {
    unsigned int ticked: 1;
    unsigned int temporary: 1;		/* Used in afm ligature closure */
    /* enum possub_type*/ uint8 type;
    struct lookup_subtable *subtable;
    struct generic_pst *next;
    union {
	struct vr pos;
	struct { char *paired; struct vr *vr; } pair;
	struct { char *variant; } subs;
	struct { char *components; } mult, alt;
	struct { char *components; struct splinechar *lig; } lig;
	struct { int16 *carets; int cnt; } lcaret;	/* Ligature caret positions */
    } u;
} PST;

typedef struct liglist {
    PST *lig;
    struct splinechar *first;		/* First component */
    struct splinecharlist *components;	/* Other than the first */
    struct liglist *next;
    int ccnt;				/* Component count. (includes first component) */
} LigList;

enum fpossub_format { pst_glyphs, pst_class, pst_coverage,
		    pst_reversecoverage, pst_formatmax };

struct seqlookup {
    int seq;
    struct otlookup *lookup;
};

struct fpg { char *names, *back, *fore; };
struct fpc { int ncnt, bcnt, fcnt; uint16 *nclasses, *bclasses, *fclasses, *allclasses; };
struct fpv { int ncnt, bcnt, fcnt; char **ncovers, **bcovers, **fcovers; };
struct fpr { int always1, bcnt, fcnt; char **ncovers, **bcovers, **fcovers; char *replacements; };

struct fpst_rule {
    union {
	/* Note: Items in backtrack area are in reverse order because that's how the OT wants them */
	/*  they need to be reversed again to be displayed to the user */
	struct fpg glyph;
	struct fpc class;
	struct fpv coverage;
	struct fpr rcoverage;
    } u;
    int lookup_cnt;
    struct seqlookup *lookups;
};

typedef struct generic_fpst {
    uint16 /*enum possub_type*/ type;
    uint16 /*enum fpossub_format*/ format;
    struct lookup_subtable *subtable;
    struct generic_fpst *next;
    uint16 nccnt, bccnt, fccnt;
    uint16 rule_cnt;
    char **nclass, **bclass, **fclass;
    struct fpst_rule *rules;
    uint8 ticked;
    uint8 effectively_by_glyphs;
    char **nclassnames, **bclassnames, **fclassnames;
} FPST;

enum asm_type { asm_indic, asm_context, asm_lig, asm_simple=4, asm_insert,
	asm_kern=0x11 };
enum asm_flags { asm_vert=0x8000, asm_descending=0x4000, asm_always=0x2000 };

struct asm_state {
    uint16 next_state;
    uint16 flags;
    union {
	struct {
	    struct otlookup *mark_lookup;	/* for contextual glyph subs (tag of a nested lookup) */
	    struct otlookup *cur_lookup;	/* for contextual glyph subs */
	} context;
	struct {
	    char *mark_ins;
	    char *cur_ins;
	} insert;
	struct {
	    int16 *kerns;
	    int kcnt;
	} kern;
    } u;
};

typedef struct generic_asm {		/* Apple State Machine */
    struct generic_asm *next;
    uint16 /*enum asm_type*/ type;
    struct lookup_subtable *subtable;	/* Lookup contains feature setting info */
    uint16 flags;	/* 0x8000=>vert, 0x4000=>r2l, 0x2000=>hor&vert */
    uint8 ticked;

    uint16 class_cnt, state_cnt;
    char **classes;
    struct asm_state *state;
} ASM;
/* State Flags:
 Indic:
	0x8000	mark current glyph as first in rearrangement
	0x4000	don't advance to next glyph
	0x2000	mark current glyph as last
	0x000f	verb
		0 = no change		8 = AxCD => CDxA
		1 = Ax => xA		9 = AxCD => DCxA
		2 = xD => Dx		a = ABxD => DxAB
		3 = AxD => DxA		b = ABxD => DxBA
		4 = ABx => xAB		c = ABxCD => CDxAB
		5 = ABx => xBA		d = ABxCD => CDxBA
		6 = xCD => CDx		e = ABxCD => DCxAB
		7 = xCD => DCx		f = ABxCD => DCxBA
 Contextual:
	0x8000	mark current glyph
	0x4000	don't advance to next glyph
 Insert:
	0x8000	mark current glyph
	0x4000	don't advance to next glyph
	0x2000	current is Kashida like
	0x1000	mark is Kashida like
	0x0800	current insert before
	0x0400	mark insert before
	0x03e0	count of chars to be inserted at current (31 max)
	0x001f	count of chars to be inserted at mark (31 max)
 Kern:
	0x8000	add current glyph to kerning stack
	0x4000	don't advance to next glyph
	0x3fff	value offset
*/

struct jstf_prio {
    OTLookup **enableShrink;	/* Points to an array of lookups (GSUB or GPOS)*/
    OTLookup **disableShrink;	/* NULL terminated */
    OTLookup **maxShrink;	/* Array of GPOS like lookups */
    OTLookup **enableExtend;
    OTLookup **disableExtend;
    OTLookup **maxExtend;
};

struct jstf_lang {
    uint32 lang;
    struct jstf_lang *next;
    int cnt;
    struct jstf_prio *prios;
};

typedef struct jstf_script {
    uint32 script;
    struct jstf_script *next;
    char *extenders;		/* list of glyph names */
    struct jstf_lang *langs;
} Justify;

struct macname {
    struct macname *next;
    uint16 enc;		/* Platform specific encoding. 0=>mac roman, 1=>sjis, 7=>russian */
    uint16 lang;	/* Mac languages 0=>english, 1=>french, 2=>german */
    char *name;		/* Not a unicode string, uninterpreted mac encoded string */
};

/* Wow, the GPOS 'size' feature stores a string in the name table just as mac */
/*  features do */
/* And now (OTF 1.6) GSUB 'ss01'-'ss20' do too */
struct otfname {
    struct otfname *next;
    uint16 lang;	/* windows language code */
    char *name;		/* utf8 */
};

struct otffeatname {
    uint32 tag;			/* Feature tag */
    struct otfname *names;
    struct otffeatname *next;
    uint16 nid;			/* temporary value */
};

struct macsetting {
    struct macsetting *next;
    uint16 setting;
    uint16 strid;
    struct macname *setname;
    unsigned int initially_enabled: 1;
};

typedef struct macfeat {
    struct macfeat *next;
    uint16 feature;
    uint8 ismutex;
    uint8 default_setting;		/* Apple's docs say both that this is a byte and a short. It's a byte */
    uint16 strid;			/* Temporary value, used when reading in */
    struct macname *featname;
    struct macsetting *settings;
} MacFeat;

typedef struct refbdfc {
    unsigned int checked: 1;
    unsigned int selected: 1;
    int8 xoff;
    int8 yoff;
    uint16 gid;
    struct refbdfc *next;
    struct bdfchar *bdfc;
} BDFRefChar;

struct bdfcharlist {
    struct bdfchar *bc;
    struct bdfcharlist *next;
};

typedef struct bdfchar {
    struct splinechar *sc;
    int16 xmin,xmax,ymin,ymax;
    int16 width;
    int16 bytes_per_line;
    uint8 *bitmap;
    struct refbdfc *refs;
    int orig_pos;
    int16 pixelsize;                    /* for undoes */
    struct bitmapview *views;
    struct undoes *undoes;
    struct undoes *redoes;
    unsigned int changed: 1;
    unsigned int byte_data: 1;	/* for anti-aliased chars entries are grey-scale bytes not bw bits */
    unsigned int widthgroup: 1;	/* for ttf bitmap output */
    unsigned int isreference: 1;	/* for ttf bitmap input, */
    unsigned int ticked: 1;
    uint8 depth;			/* for ttf bitmap output */
    uint16 vwidth;
    BDFFloat *selection;
    BDFFloat *backup;
    struct bdfcharlist *dependents;
} BDFChar;

enum undotype { ut_none=0, ut_state, ut_tstate, ut_statehint, ut_statename,
		ut_statelookup,
		ut_anchors,
		ut_width, ut_vwidth, ut_lbearing, ut_rbearing, ut_possub,
		ut_hints, ut_bitmap, ut_bitmapsel, ut_composit, ut_multiple, ut_layers,
		ut_noop };

#define UNDO_LAYER_UNKNOWN -1


enum sfundotype
{
    sfut_none=0,
    sfut_lookups,
    sfut_lookups_kerns,
    sfut_fontinfo,
    sfut_noop
};

/**
 * A spline font level undo stack. undoes are doubly linked using the
 * 'ln' member and carry some user presentable description of what the
 * undo relates to in 'msg'.
 *
 * The sfdchunk is a pointer to an SFD fragment which will apply the
 * undo to the current state. For example, it might contain
 * information about the old value of kerning pairs which can be used
 * to restore state to how it was. Note that the sfdchunk might only
 * be partial, containing only enough information to restore the state
 * which changed when the undo was created.
 */

typedef struct enc {
    char *enc_name;
    int char_cnt;	/* Size of the next two arrays */
    int32 *unicode;	/* unicode value for each encoding point */
    char **psnames;	/* optional postscript name for each encoding point */
    struct enc *next;
    unsigned int builtin: 1;
    unsigned int hidden: 1;
    unsigned int only_1byte: 1;
    unsigned int has_1byte: 1;
    unsigned int has_2byte: 1;
    unsigned int is_unicodebmp: 1;
    unsigned int is_unicodefull: 1;
    unsigned int is_custom: 1;
    unsigned int is_original: 1;
    unsigned int is_compact: 1;
    unsigned int is_japanese: 1;
    unsigned int is_korean: 1;
    unsigned int is_tradchinese: 1;
    unsigned int is_simplechinese: 1;
    char iso_2022_escape[8];
    int iso_2022_escape_len;
    int low_page, high_page;
    char *iconv_name;	/* For compatibility to old versions we might use a different name from that used by iconv. */
    iconv_t *tounicode;
    iconv_t *fromunicode;
    int (*tounicode_func)(int);
    int (*fromunicode_func)(int);
    unsigned int is_temporary: 1;	/* freed when the map gets freed */
    int char_max;			/* Used by temporary encodings */
} Encoding;

struct renames { char *from; char *to; };

typedef struct namelist {
    struct namelist *basedon;
    char *title;
    const char ***unicode[17];
    struct namelist *next;
    struct renames *renames;
    int uses_unicode;
    char *a_utf8_name;
} NameList;

enum uni_interp { ui_unset= -1, ui_none, ui_adobe, ui_greek, ui_japanese,
	ui_trad_chinese, ui_simp_chinese, ui_korean, ui_ams };

struct remap { uint32 firstenc, lastenc; int32 infont; };

typedef struct encmap {		/* A per-font map of encoding to glyph id */
    int32 *map;			/* Map from encoding to glyphid */
    int32 *backmap;		/* Map from glyphid to encoding */
    int enccount;		/* used size of the map array */
    				/*  strictly speaking this might include */
			        /*  glyphs that are not encoded, but which */
			        /*  are displayed after the proper encoding */
    int encmax;			/* allocated size of the map array */
    int backmax;		/* allocated size of the backmap array */
    struct remap *remap;
    Encoding *enc;
    unsigned int ticked: 1;
} EncMap;

enum property_type { prt_string, prt_atom, prt_int, prt_uint, prt_property=0x10 };

typedef struct bdfprops {
    char *name;		/* These include both properties (like SLANT) and non-properties (like FONT) */
    int type;
    union {
	char *str;
	char *atom;
	int val;
    } u;
} BDFProperties;

typedef struct bdffont {
    struct splinefont *sf;
    int glyphcnt, glyphmax;	/* used & allocated sizes of glyphs array */
    BDFChar **glyphs;		/* an array of charcnt entries */
    int16 pixelsize;
    int16 ascent, descent;
    int16 layer;		/* for piecemeal fonts */
    unsigned int piecemeal: 1;
    unsigned int bbsized: 1;
    unsigned int ticked: 1;
    unsigned int unhinted_freetype: 1;
    unsigned int recontext_freetype: 1;
    struct bdffont *next;
    struct clut *clut;
    char *foundry;
    int res;
    void *freetype_context;
    uint16 truesize;		/* for bbsized fonts */
    int16 prop_cnt;
    int16 prop_max;		/* only used within bdfinfo dlg */
    BDFProperties *props;
    uint16 ptsize, dpi;		/* for piecemeal fonts */
} BDFFont;

#define HntMax	96		/* PS says at most 96 hints */
typedef uint8 HintMask[HntMax/8];

enum pointtype { pt_curve, pt_corner, pt_tangent, pt_hvcurve };
typedef struct splinepoint {
    BasePoint me;
    BasePoint nextcp;		/* control point */
    BasePoint prevcp;		/* control point */
    unsigned int nonextcp:1;
    unsigned int noprevcp:1;
    unsigned int nextcpdef:1;
    unsigned int prevcpdef:1;
    unsigned int selected:1;	/* for UI */
    unsigned int nextcpselected: 2; /* Is the next BCP selected */
    unsigned int prevcpselected: 2; /* Is the prev BCP selected */
    unsigned int pointtype:2;
    unsigned int isintersection: 1;
    unsigned int flexy: 1;	/* When "freetype_markup" is on in charview.c:DrawPoint */
    unsigned int flexx: 1;	/* flexy means select nextcp, and flexx means draw circle around nextcp */
    unsigned int roundx: 1;	/* For true type hinting */
    unsigned int roundy: 1;	/* For true type hinting */
    unsigned int dontinterpolate: 1;	/* in ttf, don't imply point by interpolating between cps */
    unsigned int ticked: 1;
    unsigned int watched: 1;
	/* 1 bits left... */
    uint16 ptindex;		/* Temporary value used by metafont routine */
    uint16 ttfindex;		/* Truetype point index */
	/* Special values 0xffff => point implied by averaging control points */
	/*		  0xfffe => point created with no real number yet */
	/* (or perhaps point in context where no number is possible as in a glyph with points & refs) */
    uint16 nextcpindex;		/* Truetype point index */
    struct spline *next;
    struct spline *prev;
    HintMask *hintmask;
	char* name;
} SplinePoint;

enum linelist_flags { cvli_onscreen=0x1, cvli_clipped=0x2 };

typedef struct linelist {
    IPoint here;
    struct linelist *next;
    /* The first two fields are constant for the linelist, the next ones */
    /*  refer to a particular screen. If some portion of the line from */
    /*  this point to the next one is on the screen then set cvli_onscreen */
    /*  if this point needs to be clipped then set cvli_clipped */
    /*  asend and asstart are the actual screen locations where this point */
    /*  intersects the clip edge. */
    enum linelist_flags flags;
    IPoint asend, asstart;
} LineList;

typedef struct linearapprox {
    real scale;
    unsigned int oneline: 1;
    unsigned int onepoint: 1;
    unsigned int any: 1;		/* refers to a particular screen */
    struct linelist *lines;
    struct linearapprox *next;
} LinearApprox;

typedef struct spline1d {
    real a, b, c, d;
} Spline1D;

/**
 *
 * 2013Note: If you are altering from->me.x and y then you will
 *           probably have to modify splines[] to match your change.
 *           eg, moving both ends of a spline up/down by changing their
 *           to/from will also probably need an update to splines[ 0 | 1 ].d to
 *           match.
 */
typedef struct spline {
    unsigned int islinear: 1;		/* No control points */
    unsigned int isquadratic: 1;	/* probably read in from ttf */
    unsigned int isticked: 1;
    unsigned int isneeded: 1;		/* Used in remove overlap */
    unsigned int isunneeded: 1;		/* Used in remove overlap */
    unsigned int exclude: 1;		/* Used in remove overlap varient: exclude */
    unsigned int ishorvert: 1;
    unsigned int knowncurved: 1;	/* We know that it curves */
    unsigned int knownlinear: 1;	/* it might have control points, but still traces out a line */
	/* If neither knownlinear nor curved then we haven't checked */
    unsigned int order2: 1;		/* It's a bezier curve with only one cp */
    unsigned int touched: 1;
    unsigned int leftedge: 1;
    unsigned int rightedge: 1;
    unsigned int acceptableextrema: 1;	/* This spline has extrema, but we don't care */
    SplinePoint *from;
    SplinePoint *to;
    Spline1D splines[2];		/* splines[0] is the x spline, splines[1] is y */
    struct linearapprox *approx;
    /* Posible optimizations:
	Precalculate bounding box
	Precalculate min/max/ points of inflection
    */
} Spline;

#ifndef _NO_LIBSPIRO
# include "spiroentrypoints.h"
#else
# define SPIRO_OPEN_CONTOUR	'{'
# define SPIRO_CORNER		'v'
# define SPIRO_G4		'o'
# define SPIRO_G2		'c'
# define SPIRO_LEFT		'['
# define SPIRO_RIGHT		']'
# define SPIRO_END		'z'
typedef struct {			/* Taken from spiro.h because I want */
    double x;				/*  to be able to compile for spiro */
    double y;				/*  even on a system without it */
    char ty;
} spiro_cp;
#endif
#define SPIRO_SELECTED(cp)	((cp)->ty&0x80)
#define SPIRO_DESELECT(cp)	((cp)->ty&=~0x80)
#define SPIRO_SELECT(cp)	((cp)->ty|=0x80)
#define SPIRO_SPL_OPEN(spl)	((spl)->spiro_cnt>1 && ((spl)->spiros[0].ty&0x7f)==SPIRO_OPEN_CONTOUR)

#define SPIRO_NEXT_CONSTRAINT	SPIRO_RIGHT	/* The curve is on the next side of the constraint point */
#define SPIRO_PREV_CONSTRAINT	SPIRO_LEFT	/* The curve is on the prev side of the constraint point */

typedef struct splinepointlist {
    SplinePoint *first, *last;
    struct splinepointlist *next;
    spiro_cp *spiros;
    uint16 spiro_cnt, spiro_max;
	/* These could be bit fields, but bytes are easier to access and we */
	/*  don't need the space (yet) */
    uint8 ticked;
    uint8 beziers_need_optimizer;	/* If the spiros have changed in spiro mode, then reverting to bezier mode might, someday, run a simplifier */
    uint8 is_clip_path;			/* In type3/svg fonts */
    int start_offset; // This indicates which point is the canonical first for purposes of outputting to U. F. O..
    char *contour_name;
} SplinePointList, SplineSet;


struct reflayer {
    unsigned int background: 1;
    unsigned int order2: 1;
    unsigned int anyflexes: 1;
    unsigned int dofill: 1;
    unsigned int dostroke: 1;
    unsigned int fillfirst: 1;
    struct brush fill_brush;
    struct pen stroke_pen;
    SplinePointList *splines;
};

typedef struct refchar {
    unsigned int checked: 1;
    unsigned int selected: 1;
    unsigned int point_match: 1;	/* match_pt* are point indexes */
					/*  and need to be converted to a */
			                /*  translation after truetype readin */
    unsigned int encoded: 1;		/* orig_pos is actually an encoded value, used for old sfd files */
    unsigned int justtranslated: 1;	/* The transformation matrix specifies a translation (or is identity) */
    unsigned int use_my_metrics: 1;	/* Retain the ttf "use_my_metrics" info. */
	/* important for glyphs with instructions which change the width used */
	/* inside composites */
    unsigned int round_translation_to_grid: 1;	/* Retain the ttf "round_to_grid" info. */
    unsigned int point_match_out_of_date: 1;	/* Someone has edited a base glyph */
    int16 adobe_enc;
    int orig_pos;
    int unicode_enc;		/* used by paste */
    real transform[6];		/* transformation matrix (first 2 rows of a 3x3 matrix, missing row is 0,0,1) */
    struct reflayer *layers;
    int layer_cnt;
    struct refchar *next;
    DBounds bb;
    struct splinechar *sc;
    BasePoint top;
    uint16 match_pt_base, match_pt_ref;
} RefChar;

/* Some stems may appear, disappear, reapear several times */
/* Serif stems on I which appear at 0, disappear, reappear at top */
/* Or the major vertical stems on H which disappear at the cross bar */
typedef struct hintinstance {
    real begin;			/* location in the non-major direction*/
    real end;				/* width/height in non-major direction*/
    unsigned int closed: 1;
    short int counternumber;
    struct hintinstance *next;
} HintInstance;

enum hinttypes { ht_unspecified=0, ht_h, ht_v, ht_d };
typedef real _MMArray[2][MmMax];

typedef struct steminfo {
    struct steminfo *next;
    unsigned int hinttype: 2;	/* Only used by undoes */
    unsigned int ghost: 1;	/* this is a ghost stem hint. As such truetype should ignore it, type2 output should negate it, and type1 should use as is */
		    /* stored width will be either 20 or 21 */
		    /* Type2 says: -20 is "width" of top edge, -21 is "width" of bottom edge, type1 accepts either */
    unsigned int haspointleft:1;
    unsigned int haspointright:1;
    unsigned int hasconflicts:1;/* Does this stem have conflicts within its cluster? */
    unsigned int used: 1;	/* Temporary for counter hints or hint substitution */
    unsigned int tobeused: 1;	/* Temporary for counter hints or hint substitution */
    unsigned int active: 1;	/* Currently active hint in Review Hints dlg */
				/*  displayed differently in char display */
    unsigned int enddone: 1;	/* Used by ttf instructing, indicates a prev */
				/*  hint had the same end as this one (so */
			        /*  the points on the end line have been */
			        /*  instructed already */
    unsigned int startdone: 1;	/* Used by ttf instructing */
    /*unsigned int backwards: 1;*/	/* If we think this hint is better done with a negative width */
    unsigned int reordered: 1;	/* In AutoHinting. Means we changed the start of the hint, need to test for out of order */
    unsigned int pendingpt: 1;	/* A pending stem creation, not a true stem */
    unsigned int linearedges: 1;/* If we have a nice rectangle then we aren't */
				/*  interested in the orientation which is */
			        /*  wider than long */
    int16 hintnumber;		/* when dumping out hintmasks we need to know */
				/*  what bit to set for this hint */
    union {
	int mask;		/* Mask of all references that use this hint */
				/*  in type2 output */
	_MMArray *unblended /*[2][MmMax]*/;	/* Used when reading in type1 mm hints */
    } u;
    real start;			/* location at which the stem starts */
    real width;			/* or height */
    HintInstance *where;	/* location(s) in the other coord */
} StemInfo;

typedef struct dsteminfo {
    struct dsteminfo *next;	/* First two fields match those in steminfo */
    unsigned int hinttype: 2;	/* Only used by undoes */
    unsigned int used: 1;	/* used only by tottf.c:gendinstrs, metafont.c to mark a hint that has been dealt with */
    BasePoint left, right, unit;
    HintInstance *where;	/* location(s) along the unit vector */
} DStemInfo;

typedef struct minimumdistance {
    /* If either point is NULL it will be assumed to mean either the origin */
    /*  or the width point (depending on which is closer). This allows user */
    /*  to control metrics... */
    SplinePoint *sp1, *sp2;
    unsigned int x: 1;
    unsigned int done: 1;
    struct minimumdistance *next;
} MinimumDistance;

typedef struct layer /* : reflayer */{
    unsigned int background: 1;
    unsigned int order2: 1;
    unsigned int anyflexes: 1;
    unsigned int dofill: 1;
    unsigned int dostroke: 1;
    unsigned int fillfirst: 1;
    struct brush fill_brush;
    struct pen stroke_pen;
    SplinePointList *splines;
    RefChar *refs;			/* Only in foreground layer(s) */
    uint32 validation_state;
    uint32 old_vs;
    void *python_persistent;		/* If python this will hold a python object, if not python this will hold a string containing a pickled object. We do nothing with it (if not python) except save it back out unchanged */
    int python_persistent_has_lists;
} Layer;

enum layer_type { ly_all=-2, ly_grid= -1, ly_back=0, ly_fore=1,
    /* Possibly other foreground layers for type3 things */
    /* Possibly other background layers for normal fonts */
	ly_none = -3
    };

struct gv_part {
    char *component;
    unsigned int is_extender: 1;	/* This component may be skipped or repeated */
    uint16 startConnectorLength;
    uint16 endConnectorLength;
    uint16 fullAdvance;
};

/* For the 'MATH' table (and for TeX) */
struct glyphvariants {
    char *variants;	/* Space separated list of glyph names */
/* Glyph assembly */
    int16 italic_correction;	/* Of the composed glyph */
    DeviceTable *italic_adjusts;
    int part_cnt;
    struct gv_part *parts;
};

struct mathkerndata {
    int16 height,kern;
    DeviceTable *height_adjusts;
    DeviceTable *kern_adjusts;
};

/* For the 'MATH' table */
struct mathkernvertex {
    int cnt;		/* There is one more kern entry than height entry */
	    /* So the last mkd should have its height ignored */
	    /* The MATH table stores the height count, I think the kern count */
	    /*  is more useful (and that's what I use here). They differ by 1 */
    struct mathkerndata *mkd;
};

struct mathkern {
    struct mathkernvertex top_right;
    struct mathkernvertex top_left;
    struct mathkernvertex bottom_right;
    struct mathkernvertex bottom_left;
};

enum privatedict_state {
    pds_odd        = 0x1,	/* Odd number of entries */
    pds_outoforder = 0x2,	/* Bluevalues should be listed in order */
    pds_toomany    = 0x4,	/* arrays are of limited sizes */
    pds_tooclose   = 0x8,	/* adjacent zones must not be within 2*bluefuzz+1 (or 3, if bluefuzz omitted) */
    pds_notintegral= 0x10,	/* Must be integers */
    pds_toobig     = 0x20,	/* within pair difference have some relation to BlueScale but the docs make no sense to me */
    pds_shift	   = 8,		/* BlueValues/OtherBlues, unshifted, FamilyBlues/FamilyOtherBlues shifted once */

    pds_missingblue  = 0x010000,
    pds_badbluefuzz  = 0x020000,
    pds_badbluescale = 0x040000,
    pds_badstdhw     = 0x080000,
    pds_badstdvw     = 0x100000,
    pds_badstemsnaph = 0x200000,
    pds_badstemsnapv = 0x400000,
    pds_stemsnapnostdh = 0x0800000,
    pds_stemsnapnostdv = 0x1000000,
    pds_badblueshift   = 0x2000000

};

enum validation_state { vs_unknown = 0,
	vs_known=0x01,				/* It has been validated */
	vs_opencontour=0x02,
	vs_selfintersects=0x04,
	vs_wrongdirection=0x08,
	vs_flippedreferences=0x10,		/* special case of wrong direction */
	vs_missingextrema=0x20,
	vs_missingglyphnameingsub=0x40,
	    /* Next few are postscript only */
	vs_toomanypoints=0x80,
	vs_toomanyhints=0x100,
	vs_badglyphname=0x200,
	    /* Next few are only for fontlint */
	    /* These are relative to maxp values which ff would fix on generating a font */
	vs_maxp_toomanypoints    =0x400,
	vs_maxp_toomanypaths     =0x800,
	vs_maxp_toomanycomppoints=0x1000,
	vs_maxp_toomanycomppaths =0x2000,
	vs_maxp_instrtoolong     =0x4000,
	vs_maxp_toomanyrefs      =0x8000,
	vs_maxp_refstoodeep      =0x10000,
	/* vs_maxp_prepfpgmtoolong=0x20000, */	/* I think I was wrong about this "error" */
	    /* Oops, we need another one, two, for the glyphs */
	vs_pointstoofarapart	= 0x40000,
	vs_nonintegral		= 0x80000,	/* This will never be interesting in a real font, but might be in an sfd file */
	vs_missinganchor	= 0x100000,
	vs_dupname		= 0x200000,
	vs_dupunicode		= 0x400000,
	vs_overlappedhints	= 0x800000,

	vs_last = vs_overlappedhints,
	vs_maskps = 0x3fe | vs_pointstoofarapart | vs_missinganchor | vs_dupname | vs_dupunicode | vs_overlappedhints,
	vs_maskcid = 0x1fe | vs_pointstoofarapart | vs_missinganchor | vs_dupname | vs_overlappedhints,
	vs_maskttf = 0x7e | vs_pointstoofarapart | vs_nonintegral | vs_missinganchor | vs_dupunicode,
	vs_maskfindproblems = 0x1be | vs_pointstoofarapart | vs_nonintegral | vs_missinganchor | vs_overlappedhints
	};

struct splinecharlist { struct splinechar *sc; struct splinecharlist *next;};

struct altuni { struct altuni *next; int32 unienc, vs; uint32 fid; };
	/* vs is the "variation selector" a unicode codepoint which modifieds */
	/*  the code point before it. If vs is -1 then unienc is just an */
	/*  alternate encoding (greek Alpha and latin A), but if vs is one */
	/*  of unicode's variation selectors then this glyph is somehow a */
	/*  variant shape. The specifics depend on the selector and script */
	/*  fid is currently unused, but may, someday, be used to do ttcs */
	/* NOTE: GlyphInfo displays vs==-1 as vs==0, and fixes things up */

typedef struct splinechar {
    char *name;
    int unicodeenc;
    int orig_pos;		/* Original position in the glyph list */
    int16 width, vwidth;
    int16 lsidebearing;		/* only used when reading in a type1 font */
				/*  Or an otf font where it is the subr number of a refered character */
			        /*  or a ttf font without bit 1 of head.flags set */
			        /*  or (once upon a time, but no longer) a ttf font with vert metrics where it is the ymax value when we had a font-wide vertical offset */
			        /*  or when generating morx where it is the mask of tables in which the glyph occurs */
				/* Always a temporary value */
    int ttf_glyph;		/* only used when writing out a ttf or otf font */
    Layer *layers;		/* layer[0] is background, layer[1] foreground */
	/* In type3 fonts 2-n are also foreground, otherwise also background */
    int layer_cnt;
    StemInfo *hstem;		/* hstem hints have a vertical offset but run horizontally */
    StemInfo *vstem;		/* vstem hints have a horizontal offset but run vertically */
    DStemInfo *dstem;		/* diagonal hints for ttf */
    MinimumDistance *md;

    struct charinfo *charinfo;
    struct splinefont *parent;
    unsigned int changed: 1;
    unsigned int changedsincelasthinted: 1;
    unsigned int manualhints: 1;
    unsigned int ticked: 1;	/* For reference character processing */
				/* And fontview processing */
    unsigned int changed_since_autosave: 1;
    unsigned int widthset: 1;	/* needed so an emspace char doesn't disappear */
    unsigned int vconflicts: 1;	/* Any hint overlaps in the vstem list? */
    unsigned int hconflicts: 1;	/* Any hint overlaps in the hstem list? */
    unsigned int searcherdummy: 1;
    unsigned int changed_since_search: 1;
    unsigned int wasopen: 1;
    unsigned int namechanged: 1;
    unsigned int blended: 1;	/* An MM blended character */
    unsigned int ticked2: 1;
    unsigned int glyph_class: 3; /* 0=> fontforge determines class automagically, else one more than the class value in gdef so 2+1=>lig, 3+1=>mark */
    unsigned int numberpointsbackards: 1;
    unsigned int instructions_out_of_date: 1;
    unsigned int complained_about_ptnums: 1;
    unsigned int vs_open: 1;
    unsigned int unlink_rm_ovrlp_save_undo: 1;
    unsigned int inspiro: 1;
    unsigned int lig_caret_cnt_fixed: 1;
    unsigned int suspendMetricsViewEventPropagation: 1; /* rect tool might do this while drawing */
    /* 5 bits left (one more if we ignore compositionunit below) */
#if HANYANG
    unsigned int compositionunit: 1;
    int16 jamo, varient;
#endif
    struct splinecharlist *dependents;
	    /* The dependents list is a list of all characters which refenence*/
	    /*  the current character directly */
    KernPair *kerns; // Note that the left character in the pair has the reference to the kerning pair, which in turn references the right character.
    KernPair *vkerns;
    PST *possub;		/* If we are a ligature then this tells us what */
				/*  It may also contain a bunch of other stuff now */
    LigList *ligofme;		/* If this is the first character of a ligature then this gives us the list of possible ones */
				/*  this field must be regenerated before the font is saved */
    char *comment;			/* in utf8 */
    uint32 /*Color*/ color;
    AnchorPoint *anchor;
    uint8 *ttf_instrs;
    int16 ttf_instrs_len;
    int16 countermask_cnt;
    HintMask *countermasks;
    struct altuni *altuni;
/* for TeX */
    int16 tex_height, tex_depth;
/* TeX also uses italic_correction and glyph variants below */
/* For the 'MATH' table (and for TeX) */
    unsigned int is_extended_shape: 1;
    int16 italic_correction;
    int16 top_accent_horiz;		/* MATH table allows you to specific a*/
		/* horizontal anchor for accent attachments, vertical */
		/* positioning is done elsewhere */
    DeviceTable *italic_adjusts;
    DeviceTable *top_accent_adjusts;
    struct glyphvariants *vert_variants;
    struct glyphvariants *horiz_variants;
    struct mathkern *mathkern;
/* End of MATH/TeX fields */
#ifndef _NO_PYTHON
    void *python_sc_object;
    void *python_temporary;
#endif
#if 0
    // Python persistent data is now in the layers.
    void *python_persistent;		/* If python this will hold a python object, if not python this will hold a string containing a pickled object. We do nothing with it (if not python) except save it back out unchanged */
    int python_persistent_has_lists;
#endif // 0
	/* If the glyph is used as a tile pattern, then the next two values */
	/*  determine the amount of white space around the tile. If extra is*/
	/*  non-zero then we add it to the max components of the bbox and   */
	/*  subtract it from the min components. If extra is 0 then tile_bounds*/
	/*  will be used. If tile_bounds is all zeros then the glyph's bbox */
	/*  will be used. */
    real tile_margin;			/* If the glyph is used as a tile */
    DBounds tile_bounds;
    char * glif_name; // This stores the base name of the glyph when saved to U. F. O..
} SplineChar;

#define TEX_UNDEF 0x7fff

enum ttfnames { ttf_copyright=0, ttf_family, ttf_subfamily, ttf_uniqueid,
    ttf_fullname, ttf_version, ttf_postscriptname, ttf_trademark,
    ttf_manufacturer, ttf_designer, ttf_descriptor, ttf_venderurl,
    ttf_designerurl, ttf_license, ttf_licenseurl, ttf_idontknow/*reserved*/,
    ttf_preffamilyname, ttf_prefmodifiers, ttf_compatfull, ttf_sampletext,
    ttf_cidfindfontname, ttf_wwsfamily, ttf_wwssubfamily, ttf_namemax };
struct ttflangname {
    int lang;
    char *names[ttf_namemax];			/* in utf8 */
    int frommac[(ttf_namemax+31)/32];		/* Used when parsing the 'name' table */
    struct ttflangname *next;
};

struct MATH {
/* From the MATH Constants subtable (constants for positioning glyphs. Not PI)*/
    int16 ScriptPercentScaleDown;
    int16 ScriptScriptPercentScaleDown;
    uint16 DelimitedSubFormulaMinHeight;
    uint16 DisplayOperatorMinHeight;
    int16 MathLeading;
    DeviceTable *MathLeading_adjust;
    int16 AxisHeight;
    DeviceTable *AxisHeight_adjust;
    int16 AccentBaseHeight;
    DeviceTable *AccentBaseHeight_adjust;
    int16 FlattenedAccentBaseHeight;
    DeviceTable *FlattenedAccentBaseHeight_adjust;
    int16 SubscriptShiftDown;
    DeviceTable *SubscriptShiftDown_adjust;
    int16 SubscriptTopMax;
    DeviceTable *SubscriptTopMax_adjust;
    int16 SubscriptBaselineDropMin;
    DeviceTable *SubscriptBaselineDropMin_adjust;
    int16 SuperscriptShiftUp;
    DeviceTable *SuperscriptShiftUp_adjust;
    int16 SuperscriptShiftUpCramped;
    DeviceTable *SuperscriptShiftUpCramped_adjust;
    int16 SuperscriptBottomMin;
    DeviceTable *SuperscriptBottomMin_adjust;
    int16 SuperscriptBaselineDropMax;
    DeviceTable *SuperscriptBaselineDropMax_adjust;
    int16 SubSuperscriptGapMin;
    DeviceTable *SubSuperscriptGapMin_adjust;
    int16 SuperscriptBottomMaxWithSubscript;
    DeviceTable *SuperscriptBottomMaxWithSubscript_adjust;
    int16 SpaceAfterScript;
    DeviceTable *SpaceAfterScript_adjust;
    int16 UpperLimitGapMin;
    DeviceTable *UpperLimitGapMin_adjust;
    int16 UpperLimitBaselineRiseMin;
    DeviceTable *UpperLimitBaselineRiseMin_adjust;
    int16 LowerLimitGapMin;
    DeviceTable *LowerLimitGapMin_adjust;
    int16 LowerLimitBaselineDropMin;
    DeviceTable *LowerLimitBaselineDropMin_adjust;
    int16 StackTopShiftUp;
    DeviceTable *StackTopShiftUp_adjust;
    int16 StackTopDisplayStyleShiftUp;
    DeviceTable *StackTopDisplayStyleShiftUp_adjust;
    int16 StackBottomShiftDown;
    DeviceTable *StackBottomShiftDown_adjust;
    int16 StackBottomDisplayStyleShiftDown;
    DeviceTable *StackBottomDisplayStyleShiftDown_adjust;
    int16 StackGapMin;
    DeviceTable *StackGapMin_adjust;
    int16 StackDisplayStyleGapMin;
    DeviceTable *StackDisplayStyleGapMin_adjust;
    int16 StretchStackTopShiftUp;
    DeviceTable *StretchStackTopShiftUp_adjust;
    int16 StretchStackBottomShiftDown;
    DeviceTable *StretchStackBottomShiftDown_adjust;
    int16 StretchStackGapAboveMin;
    DeviceTable *StretchStackGapAboveMin_adjust;
    int16 StretchStackGapBelowMin;
    DeviceTable *StretchStackGapBelowMin_adjust;
    int16 FractionNumeratorShiftUp;
    DeviceTable *FractionNumeratorShiftUp_adjust;
    int16 FractionNumeratorDisplayStyleShiftUp;
    DeviceTable *FractionNumeratorDisplayStyleShiftUp_adjust;
    int16 FractionDenominatorShiftDown;
    DeviceTable *FractionDenominatorShiftDown_adjust;
    int16 FractionDenominatorDisplayStyleShiftDown;
    DeviceTable *FractionDenominatorDisplayStyleShiftDown_adjust;
    int16 FractionNumeratorGapMin;
    DeviceTable *FractionNumeratorGapMin_adjust;
    int16 FractionNumeratorDisplayStyleGapMin;
    DeviceTable *FractionNumeratorDisplayStyleGapMin_adjust;
    int16 FractionRuleThickness;
    DeviceTable *FractionRuleThickness_adjust;
    int16 FractionDenominatorGapMin;
    DeviceTable *FractionDenominatorGapMin_adjust;
    int16 FractionDenominatorDisplayStyleGapMin;
    DeviceTable *FractionDenominatorDisplayStyleGapMin_adjust;
    int16 SkewedFractionHorizontalGap;
    DeviceTable *SkewedFractionHorizontalGap_adjust;
    int16 SkewedFractionVerticalGap;
    DeviceTable *SkewedFractionVerticalGap_adjust;
    int16 OverbarVerticalGap;
    DeviceTable *OverbarVerticalGap_adjust;
    int16 OverbarRuleThickness;
    DeviceTable *OverbarRuleThickness_adjust;
    int16 OverbarExtraAscender;
    DeviceTable *OverbarExtraAscender_adjust;
    int16 UnderbarVerticalGap;
    DeviceTable *UnderbarVerticalGap_adjust;
    int16 UnderbarRuleThickness;
    DeviceTable *UnderbarRuleThickness_adjust;
    int16 UnderbarExtraDescender;
    DeviceTable *UnderbarExtraDescender_adjust;
    int16 RadicalVerticalGap;
    DeviceTable *RadicalVerticalGap_adjust;
    int16 RadicalDisplayStyleVerticalGap;
    DeviceTable *RadicalDisplayStyleVerticalGap_adjust;
    int16 RadicalRuleThickness;
    DeviceTable *RadicalRuleThickness_adjust;
    int16 RadicalExtraAscender;
    DeviceTable *RadicalExtraAscender_adjust;
    int16 RadicalKernBeforeDegree;
    DeviceTable *RadicalKernBeforeDegree_adjust;
    int16 RadicalKernAfterDegree;
    DeviceTable *RadicalKernAfterDegree_adjust;
    uint16 RadicalDegreeBottomRaisePercent;
/* Global constants from other subtables */
    uint16 MinConnectorOverlap;			/* in the math variants sub-table */
};

enum backedup_state { bs_dontknow=0, bs_not=1, bs_backedup=2 };
enum loadvalidation_state {
	lvs_bad_ps_fontname    = 0x001,
	lvs_bad_glyph_table    = 0x002,
	lvs_bad_cff_table      = 0x004,
	lvs_bad_metrics_table  = 0x008,
	lvs_bad_cmap_table     = 0x010,
	lvs_bad_bitmaps_table  = 0x020,
	lvs_bad_gx_table       = 0x040,
	lvs_bad_ot_table       = 0x080,
	lvs_bad_os2_version    = 0x100,
	lvs_bad_sfnt_header    = 0x200
    };

typedef struct layerinfo {
    char *name;
    unsigned int background: 1;			/* Layer is to be treated as background: No width, images, not worth outputting */
    unsigned int order2: 1;			/* Layer's data are order 2 bezier splines (truetype) rather than order 3 (postscript) */
						/* In all glyphs in the font */
    unsigned int ticked: 1;
    char * ufo_path;
} LayerInfo;

/* Baseline data from the 'BASE' table */
struct baselangextent {
    uint32 lang;		/* also used for feature tag */
    struct baselangextent *next;
    int16 ascent, descent;
    struct baselangextent *features;
};

struct basescript {
    uint32 script;
    struct basescript *next;
    int    def_baseline;	/* index [0-baseline_cnt) */
    int16 *baseline_pos;	/* baseline_cnt of these */
    struct baselangextent *langs;	/* Language specific extents (may be NULL) */
				/* The default one has the tag DEFAULT_LANG */
};

struct Base {
    int baseline_cnt;
    uint32 *baseline_tags;
    /* A font does not need to provide info on all baselines, but if one script */
    /*  talks about a baseline, then all must. So the set of baselines is global*/
    struct basescript *scripts;
};

struct pfminfo {		/* A misnomer now. OS/2 info would be more accurate, but that's stuff in here from all over ttf files */
    unsigned int pfmset: 1;
    unsigned int winascent_add: 1;
    unsigned int windescent_add: 1;
    unsigned int hheadascent_add: 1;
    unsigned int hheaddescent_add: 1;
    unsigned int typoascent_add: 1;
    unsigned int typodescent_add: 1;
    unsigned int subsuper_set: 1;
    unsigned int panose_set: 1;
    unsigned int hheadset: 1;
    unsigned int vheadset: 1;
    unsigned int hascodepages: 1;
    unsigned int hasunicoderanges: 1;
    unsigned char pfmfamily;
    int16 weight;
    int16 width;
    char panose[10];
    /* A subset of OS/2 fsSelection, used for style mapping. */
    /* Must agree with macStyle per otspec, takes precedence. */
    /* Can't use macStyle because it doesn't have a "regular" bit unlike the OS/2 component. */
    int16 stylemap;
    int16 fstype;
    int16 linegap;		/* from hhea */
    int16 vlinegap;		/* from vhea */
    int16 hhead_ascent, hhead_descent;
    int16 os2_typoascent, os2_typodescent, os2_typolinegap;
    int16 os2_winascent, os2_windescent;
    int16 os2_subxsize, os2_subysize, os2_subxoff, os2_subyoff;
    int16 os2_supxsize, os2_supysize, os2_supxoff, os2_supyoff;
    int16 os2_strikeysize, os2_strikeypos;
    int16 os2_capheight, os2_xheight;
    char os2_vendor[4];
    int16 os2_family_class;
    uint32 codepages[2];
    uint32 unicoderanges[4];
};

struct ttf_table {
    uint32 tag;
    uint32 len, maxlen;
    uint8 *data;
    struct ttf_table *next;
    FILE *temp;	/* Temporary storage used during generation */
};

enum texdata_type { tex_unset, tex_text, tex_math, tex_mathext };

struct texdata {
    enum texdata_type type;
    int32 params[22];		/* param[6] has different meanings in normal and math fonts */
};

struct gasp {
    uint16 ppem;
    uint16 flags;
};

struct ff_glyphclasses {
    // This matches struct glyphclasses from featurefile.c for now. We may make the references numeric in the future.
    // There may be a matching entry as a class elsewhere. For now, the output driver is responsible for eliminating duplicates.
    // In the interest of preserving orderings, we shall output from here, checking for value overrides from kerning classes on each kerning group entry.
    char *classname, *glyphs;
    struct ff_glyphclasses *next;
};

struct ff_rawoffsets {
    // This stores raw offsets as read from kerning.plist.
    // FontForge shall output these after native data and shall output only those for which it has not emitted native data.
    char *left;
    char *right;
    int offset;
    struct ff_rawoffsets *next;
};

typedef struct splinefont {
    char *fontname, *fullname, *familyname, *weight;
    char *familyname_with_timestamp;
    char *copyright;
    char *filename;				/* sfd name. NULL if we open a font, that's origname */
    char *defbasefilename;
    char *version;
    real italicangle, upos, uwidth;		/* In font info */
    int ascent, descent, invalidem; // If invalidem, then we use the format-specific ascent and descent on export.
    int uniqueid;				/* Not copied when reading in!!!! */
    int glyphcnt, glyphmax;			/* allocated size of glyphs array */
    SplineChar **glyphs;
    unsigned int changed: 1;
    unsigned int changed_since_autosave: 1;
    unsigned int changed_since_xuidchanged: 1;
    unsigned int display_antialias: 1;
    unsigned int display_bbsized: 1;
    unsigned int dotlesswarn: 1;		/* User warned that font doesn't have a dotless i character */
    unsigned int serifcheck: 1;			/* Have we checked to see if we have serifs? */
    unsigned int issans: 1;			/* We have no serifs */
    unsigned int isserif: 1;			/* We have serifs. If neither set then we don't know. */
    unsigned int hasvmetrics: 1;		/* We've got vertical metric data and should output vhea/vmtx/VORG tables */
    unsigned int loading_cid_map: 1;
    unsigned int dupnamewarn: 1;		/* Warn about duplicate names when loading bdf font */
    unsigned int encodingchanged: 1;		/* Font's encoding has changed since it was loaded */
    unsigned int multilayer: 1;			/* only applies if TYPE3 is set, means this font can contain strokes & fills */
						/*  I leave it in so as to avoid cluttering up code with #ifdefs */
    unsigned int strokedfont: 1;
    unsigned int new: 1;			/* A new and unsaved font */
    unsigned int compacted: 1;			/* only used when opening a font */
    unsigned int backedup: 2;			/* 0=>don't know, 1=>no, 2=>yes */
    unsigned int use_typo_metrics: 1;		/* The standard says to. But MS */
    						/* seems to feel that isn't good */
			                        /* enough and has created a bit */
			                        /* to mean "really use them" */
    unsigned int weight_width_slope_only: 1;	/* This bit seems stupid to me */
    unsigned int save_to_dir: 1;		/* Loaded from an sfdir collection rather than a simple sfd file */
    unsigned int head_optimized_for_cleartype: 1;/* Bit in the 'head' flags field, if unset "East Asian fonts in the Windows Presentation Framework (Avalon) will not be hinted" */
    unsigned int ticked: 1;
    unsigned int internal_temp: 1;		/* Internal temporary font to be passed to freetype for rasterizing. Don't complain about oddities. Don't generate GPOS/GSUB tables, etc. */
    unsigned int complained_about_spiros: 1;
    unsigned int use_xuid: 1;			/* Adobe has deprecated these two */
    unsigned int use_uniqueid: 1;		/* fields. Mostly we don't want to use them */
	/* 2 bits left */
    struct metricsview *metrics;
    enum uni_interp uni_interp;
    NameList *for_new_glyphs;
    EncMap *map;		/* only used when opening a font to provide original default encoding */
    Layer grid;
    char *origname;		/* filename of font file (ie. if not an sfd) */
    char *autosavename;
    int display_size;		/* a val <0 => Generate our own images from splines, a value >0 => find a bdf font of that size */
    struct psdict *private;	/* read in from type1 file or provided by user */
    char *xuid;
    struct pfminfo pfminfo;
    struct ttflangname *names;
    char *cidregistry, *ordering;
    int supplement;
    int subfontcnt;
    struct splinefont **subfonts;
    struct splinefont *cidmaster;		/* Top level cid font */
    float cidversion;
#if HANYANG
    struct compositionrules *rules;
#endif
    char *comments;	/* Used to be restricted to ASCII, now utf8 */
    char *fontlog;
    int tempuniqueid;
    int top_enc;
    uint16 desired_row_cnt, desired_col_cnt;
    struct glyphnamehash *glyphnames;
    struct ttf_table *ttf_tables, *ttf_tab_saved;
	/* We copy: fpgm, prep, cvt, maxp (into ttf_tables) user can ask for others, into saved*/
    char **cvt_names;
    /* The end of this array is marked by a special entry: */
#define END_CVT_NAMES ((char *) (~(intpt) 0))
    struct instrdata *instr_dlgs;	/* Pointer to all table and character instruction dlgs in this font */
    struct shortview *cvt_dlg;
    struct kernclasslistdlg *kcld, *vkcld;
    struct kernclassdlg *kcd;
    struct texdata texdata;
    OTLookup *gsub_lookups, *gpos_lookups;
    /* Apple morx subtables become gsub, and kern subtables become gpos */
    AnchorClass *anchor;
    KernClass *kerns, *vkerns;
    FPST *possub;
    ASM *sm;				/* asm is a keyword */
    MacFeat *features;
    char *chosenname;			/* Set for files with multiple fonts in them */
    struct mmset *mm;			/* If part of a multiple master set */
    int16 macstyle;
    char *fondname;			/* For use in generating mac families */
    /* from the GPOS 'size' feature. design_size, etc. are measured in tenths of a point */
    /*  bottom is exclusive, top is inclusive */
    /*  if any field is 0, it is undefined. All may be undefined, All may be */
    /*  defined, or design_size may be defined without any of the others */
    /*  but we can't define the range without defining the other junk */
    /*  Name must contain an English language name, may contain others */
    uint16 design_size;
    uint16 fontstyle_id;
    struct otfname *fontstyle_name;
    uint16 design_range_bottom, design_range_top;
    struct otffeatname *feat_names;
    real strokewidth;
/* For GDEF Mark Attachment Class -- used in lookup flags */
/* As usual, class 0 is unused */
    int mark_class_cnt;
    char **mark_classes;		/* glyph name list */
    char **mark_class_names;		/* used within ff, utf8 (the name we've given to this class of marks) */
/* For GDEF Mark Attachment Sets -- used in lookup flags */
/* but here, set 0 is meaningful, since pst_usemarkfilteringset tells us */
    int mark_set_cnt;
    char **mark_sets;			/* glyph name list */
    char **mark_set_names;		/* used within ff, utf8 (the name we've given to this class of marks) */
    struct ff_glyphclasses *groups; // This stores arbitrary named character lists for use in kerning or in the feature file.
    struct ff_rawoffsets *groupkerns;
    struct ff_rawoffsets *groupvkerns;
    long long creationtime;		/* seconds since 1970 */
    long long modificationtime;
    short os2_version;			/* 0 means default rather than the real version 0 */
    short compression;			/* If we opened a compressed sfd file, then save it out compressed too */
    short gasp_version;			/* 0/1 currently */
    short gasp_cnt;
    struct gasp *gasp;
    struct MATH *MATH;
    float sfd_version;			/* Used only when reading in an sfd file */
    struct gfi_data *fontinfo;
    struct val_data *valwin;
#if !defined(_NO_PYTHON)
    void *python_temporary;
#endif
    void *python_persistent;		/* If python this will hold a python object, if not python this will hold a string containing a pickled object. We do nothing with it (if not python) except save it back out unchanged */
    int python_persistent_has_lists; // This affects whether arrays exist as tuples or as lists (thus allowing us to use tuples for foreign data).
    enum loadvalidation_state loadvalidation_state;
    LayerInfo *layers;
    int layer_cnt;
    int display_layer;
    struct Base *horiz_base, *vert_base;
    Justify *justify;
    int extrema_bound;			/* Splines do not count for extrema complaints when the distance between the endpoints is less than or equal to this */
    int width_separation;
    int sfntRevision;
#define sfntRevisionUnset	0x44445555
    int woffMajor;
#define woffUnset		0x4455
    int woffMinor;
    char *woffMetadata;
    real ufo_ascent, ufo_descent;	/* I don't know what these mean, they don't seem to correspond to any other ascent/descent pair, but retain them so round-trip ufo input/output leaves them unchanged */
	    /* ufo_descent is negative */
    char *styleMapFamilyName;
    struct sfundoes *undoes;
    char collab_uuid[ FF_UUID_STRING_SIZE ];
    int preferred_kerning; // 1 for U. F. O. native, 2 for feature file, 0 undefined. Input functions shall flag 2, I think. This is now in S. F. D. in order to round-trip U. F. O. consistently.
} SplineFont;

struct axismap {
    int points;	/* size of the next two arrays */
    real *blends;	/* between [0,1] ordered so that blend[0]<blend[1]<... */
    real *designs;	/* between the design ranges for this axis, typically [1,999] or [6,72] */
    real min, def, max;		/* For mac */
    struct macname *axisnames;	/* For mac */
};

struct named_instance {	/* For mac */
    real *coords;	/* array[axis], these are in user units */
    struct macname *names;
};

/* I am going to simplify my life and not encourage intermediate designs */
/*  this means I can easily calculate ConvertDesignVector, and don't have */
/*  to bother the user with specifying it. */
/* (NormalizeDesignVector is fairly basic and shouldn't need user help ever) */
/*  (As long as they want piecewise linear) */
/* I'm not going to support intermediate designs at all for apple var tables */
typedef struct mmset {
    int axis_count;
    char *axes[4];
    int instance_count;
    SplineFont **instances;
    SplineFont *normal;
    real *positions;	/* array[instance][axis] saying where each instance lies on each axis */
    real *defweights;	/* array[instance] saying how much of each instance makes the normal font */
			/* for adobe */
    struct axismap *axismaps;	/* array[axis] */
    char *cdv, *ndv;	/* for adobe */
    int named_instance_count;
    struct named_instance *named_instances;
    unsigned int changed: 1;
    unsigned int apple: 1;
} MMSet;

/* mac styles. Useful idea we'll just steal it */
enum style_flags { sf_bold = 1, sf_italic = 2, sf_underline = 4, sf_outline = 8,
	sf_shadow = 0x10, sf_condense = 0x20, sf_extend = 0x40 };

struct sflist {
    SplineFont *sf;
    int32 *sizes;
    FILE *tempttf;		/* For ttf */
    int id;			/* For ttf */
    int* ids;			/* One for each size */
    BDFFont **bdfs;		/* Ditto */
    EncMap *map;
    struct sflist *next;
    char **former_names;
    int len;
};

    /* Used for drawing text with mark to base anchors */
typedef struct anchorpos {
    SplineChar *sc;		/* This is the mark being positioned */
    int x,y;			/* Its origin should be shifted this much relative to that of the original base char */
    AnchorPoint *apm;		/* The anchor point in sc used to position it */
    AnchorPoint *apb;		/* The anchor point in the base character against which we are positioned */
    int base_index;		/* Index in this array to the base character (-1=> original base char) */
    unsigned int ticked: 1;	/* Used as a mark to mark */
} AnchorPos;

enum ttf_flags { ttf_flag_shortps = 1, ttf_flag_nohints = 2,
		    ttf_flag_applemode=4,
		    ttf_flag_pfed_comments=8, ttf_flag_pfed_colors=0x10,
		    ttf_flag_otmode=0x20,
		    ttf_flag_glyphmap=0x40,
		    ttf_flag_TeXtable=0x80,
		    ttf_flag_ofm=0x100,
		    ttf_flag_oldkern=0x200,	/* never set in conjunction with applemode */
		    ttf_flag_pfed_lookupnames=0x800,
		    ttf_flag_pfed_guides=0x1000,
		    ttf_flag_pfed_layers=0x2000,
		    ttf_flag_symbol=0x4000,
		    ttf_flag_dummyDSIG=0x8000,
		    ttf_native_kern=0x10000, // This applies mostly to U. F. O. right now.
		    ttf_flag_oldkernmappedonly=0x20000000 // Allow only mapped glyphs in the old-style "kern" table, required for Windows compatibility
		};
enum ttc_flags { ttc_flag_trymerge=0x1, ttc_flag_cff=0x2 };
enum openflags { of_fstypepermitted=1, of_askcmap=2, of_all_glyphs_in_ttc=4,
	of_fontlint=8, of_hidewindow=0x10, of_all_tables=0x20 };
enum ps_flags { ps_flag_nohintsubs = 0x10000, ps_flag_noflex=0x20000,
		    ps_flag_nohints = 0x40000, ps_flag_restrict256=0x80000,
		    ps_flag_afm = 0x100000, ps_flag_pfm = 0x200000,
		    ps_flag_tfm = 0x400000,
		    ps_flag_round = 0x800000,
/* CFF fonts are wrapped up in some postscript sugar -- unless they are to */
/*  go into a pdf file or an otf font */
		    ps_flag_nocffsugar = 0x1000000,
/* in type42 cid fonts we sometimes want an identity map from gid to cid */
		    ps_flag_identitycidmap = 0x2000000,
		    ps_flag_afmwithmarks = 0x4000000,
		    ps_flag_noseac = 0x8000000,
		    ps_flag_outputfontlog = 0x10000000,
		    ps_flag_mask = (ps_flag_nohintsubs|ps_flag_noflex|
			ps_flag_afm|ps_flag_pfm|ps_flag_tfm|ps_flag_round)
		};

struct compressors { char *ext, *decomp, *recomp; };
#define COMPRESSORS_EMPTY { NULL, NULL, NULL }
extern struct compressors compressors[];

enum archive_list_style { ars_tar, ars_zip };

#define ARCHIVERS_EMPTY { NULL, NULL, NULL, NULL, NULL, NULL, 0 }

struct fontdict;
struct pschars;
struct findsel;
struct charprocs;
struct enc;

#define chunkalloc(size)	calloc(1,size)
#define chunkfree(item,size)	free(item)

extern int SFOneWidth(SplineFont *sf);
extern int CIDOneWidth(SplineFont *sf);

enum fontformat { ff_pfa, ff_pfb, ff_pfbmacbin, ff_multiple, ff_mma, ff_mmb,
	ff_ptype3, ff_ptype0, ff_cid, ff_cff, ff_cffcid,
	ff_type42, ff_type42cid,
	ff_ttf, ff_ttfsym, ff_ttfmacbin, ff_ttc, ff_ttfdfont, ff_otf, ff_otfdfont,
	ff_otfcid, ff_otfciddfont, ff_svg, ff_ufo, ff_woff, ff_none };
struct cidbytes;
struct fd2data;
struct ttfinfo;
struct alltabs;

typedef struct growbuf {
    unsigned char *pt;
    unsigned char *base;
    unsigned char *end;
} GrowBuf;
extern void GrowBuffer(GrowBuf *gb);

struct glyphdata;
extern int UnitsParallel(BasePoint *u1,BasePoint *u2,int strict);
extern int CvtPsStem3(struct growbuf *gb, SplineChar *scs[MmMax], int instance_count,
	int ishstem, int round);
extern struct pschars *SplineFont2ChrsSubrs2(SplineFont *sf, int nomwid,
	int defwid, const int *bygid, int cnt, int flags,
	struct pschars **_subrs,int layer);
extern struct pschars *CID2ChrsSubrs2(SplineFont *cidmaster,struct fd2data *fds,
	int flags, struct pschars **_glbls,int layer);
enum bitmapformat { bf_bdf, bf_ttf, bf_sfnt_dfont, bf_sfnt_ms, bf_otb,
	bf_nfntmacbin, /*bf_nfntdfont, */bf_fon, bf_fnt, bf_palm,
	bf_ptype3,
	bf_none };
extern int32 filechecksum(FILE *file);
extern int _WriteWOFFFont(FILE *ttf,SplineFont *sf, enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags,EncMap *enc,int layer);
extern int WriteWOFFFont(char *fontname,SplineFont *sf, enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags,EncMap *enc,int layer);
extern int _WriteTTFFont(FILE *ttf,SplineFont *sf, enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags,EncMap *enc,int layer);
extern int WriteTTFFont(char *fontname,SplineFont *sf, enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags,EncMap *enc,int layer);
extern int _WriteType42SFNTS(FILE *type42,SplineFont *sf,enum fontformat format,
	int flags,EncMap *enc,int layer);
extern int WriteMacTTFFont(char *fontname,SplineFont *sf, enum fontformat format,
	int32 *bsizes, enum bitmapformat bf,int flags,EncMap *enc,int layer);
extern int WriteMacFamily(char *filename,struct sflist *sfs,enum fontformat format,
	enum bitmapformat bf,int flags,int layer);
extern int WriteTTC(const char *filename,struct sflist *sfs,enum fontformat format,
	enum bitmapformat bf,int flags,int layer,enum ttc_flags ttcflags);
extern long mactime(void);
extern void DefaultTTFEnglishNames(struct ttflangname *dummy, SplineFont *sf);
extern int AlreadyMSSymbolArea(SplineFont *sf,EncMap *map);
extern void OS2FigureCodePages(SplineFont *sf, uint32 CodePage[2]);
extern void OS2FigureUnicodeRanges(SplineFont *sf, uint32 Ranges[4]);
extern void SFDefaultOS2Info(struct pfminfo *pfminfo,SplineFont *sf,char *fontname);
extern void SFDefaultOS2Simple(struct pfminfo *pfminfo,SplineFont *sf);
extern void SFDefaultOS2SubSuper(struct pfminfo *pfminfo,int emsize,double italicangle);
extern int ScriptIsRightToLeft(uint32 script);
extern uint32 ScriptFromUnicode(uint32 u,SplineFont *sf);
extern uint32 SCScriptFromUnicode(SplineChar *sc);
extern int SCRightToLeft(SplineChar *sc);
extern void SFMatchGlyphs(SplineFont *sf,SplineFont *target,int addempties);
extern void MMMatchGlyphs(MMSet *mm);
extern const char *_GetModifiers(const char *fontname, const char *familyname, const char *weight);
extern const char *SFGetModifiers(const SplineFont *sf);

extern int Within4RoundingErrors(bigreal v1, bigreal v2);
extern int Within16RoundingErrors(bigreal v1, bigreal v2);
extern int RealNear(real a,real b);
extern int RealNearish(real a,real b);
extern int RealApprox(real a,real b);
extern int RealWithin(real a,real b,real fudge);

extern int PointsDiagonalable(SplineFont *sf,BasePoint **bp,BasePoint *unit);
extern int MergeDStemInfo(SplineFont *sf,DStemInfo **ds, DStemInfo *test);

extern void LineListFree(LineList *ll);
extern void LinearApproxFree(LinearApprox *la);
extern void SplineFree(Spline *spline);
extern SplinePoint *SplinePointCreate(real x, real y);
extern void SplinePointFree(SplinePoint *sp);
extern void SplinePointsFree(SplinePointList *spl);
extern void SplinePointListFree(SplinePointList *spl);
extern void SplinePointListsFree(SplinePointList *head);
extern void SplineSetSpirosClear(SplineSet *spl);
extern void RefCharFree(RefChar *ref);
extern void RefCharsFree(RefChar *ref);
extern void StemInfosFree(StemInfo *h);
extern void StemInfoFree(StemInfo *h);
extern void DStemInfosFree(DStemInfo *h);
extern void DStemInfoFree(DStemInfo *h);
extern void KernPairsFree(KernPair *kp);
extern void AnchorPointsFree(AnchorPoint *ap);
extern void AnchorClassesFree(AnchorClass *kp);
extern void TtfTablesFree(struct ttf_table *tab);
extern void ValDevFree(ValDevTab *adjust);
extern void DeviceTableFree(DeviceTable *adjust);
extern void PSTFree(PST *lig);

struct lookup_cvt { OTLookup *from, *to; int old;};
struct sub_cvt { struct lookup_subtable *from, *to; int old;};
struct ac_cvt { AnchorClass *from, *to; int old;};

extern void TTFLangNamesFree(struct ttflangname *l);
extern void AltUniFree(struct altuni *altuni);
extern void AltUniFigure(SplineFont *sf,EncMap *map,int check_dups);
extern void AltUniAdd(SplineChar *sc,int uni);
extern void AltUniAdd_DontCheckDups(SplineChar *sc,int uni);
extern void MinimumDistancesFree(MinimumDistance *md);
extern void LayerDefault(Layer *);
extern SplineChar *SplineCharCreate(int layer_cnt);
extern SplineChar *SFSplineCharCreate(SplineFont *sf);
extern RefChar *RefCharCreate(void);
extern void KernClassFreeContents(KernClass *kc);
extern void KernClassClearSpecialContents(KernClass *kc);
extern void KernClassListFree(KernClass *kc);
extern void KernClassListClearSpecialContents(KernClass *kc);
extern void OTLookupFree(OTLookup *lookup);
extern void OTLookupListFree(OTLookup *lookup );
extern void FPSTRuleContentsFree(struct fpst_rule *r, enum fpossub_format format);
extern void FPSTClassesFree(FPST *fpst);
extern void FPSTFree(FPST *fpst);
extern void ASMFree(ASM *sm);
extern void MacNameListFree(struct macname *mn);
extern void MacSettingListFree(struct macsetting *ms);
extern void MacFeatListFree(MacFeat *mf);
extern void GlyphVariantsFree(struct glyphvariants *gv);
extern void MathKernVContentsFree(struct mathkernvertex *mk);
extern void MathKernFree(struct mathkern *mk);
extern void SplineCharListsFree(struct splinecharlist *dlist);
extern void LayerFreeContents(SplineChar *sc, int layer);
extern void SplineCharFreeContents(SplineChar *sc);
extern void SplineCharFree(SplineChar *sc);
extern void EncMapFree(EncMap *map);
extern EncMap *EncMapFromEncoding(SplineFont *sf,Encoding *enc);
extern EncMap *EncMapNew(int encmax, int backmax, Encoding *enc);
extern EncMap *EncMap1to1(int enccount);
extern void ScriptLangListFree(struct scriptlanglist *sl);
extern void FeatureScriptLangListFree(FeatureScriptLangList *fl);
extern void SFBaseSort(SplineFont *sf);
extern struct baselangextent *BaseLangCopy(struct baselangextent *extent);
extern void BaseLangFree(struct baselangextent *extent);
extern void BaseScriptFree(struct basescript *bs);
extern void BaseFree(struct Base *base);
extern void SplineFontFree(SplineFont *sf);
extern void SplineFontClearSpecial(SplineFont *sf);

#if 1
// These relate to experimental support for U. F. O. groups.
#define GROUP_NAME_KERNING_UFO 1
#define GROUP_NAME_KERNING_FEATURE 2
#define GROUP_NAME_VERTICAL 4 // Otherwise horizontal.
#define GROUP_NAME_RIGHT 8 // Otherwise left (or above).

void GlyphGroupFree(struct ff_glyphclasses* group);
void GlyphGroupsFree(struct ff_glyphclasses* root);
void GlyphGroupKernFree(struct ff_rawoffsets* groupkern);
void GlyphGroupKernsFree(struct ff_rawoffsets* root);
#ifdef FF_UTHASH_GLIF_NAMES
struct glif_name_index;
int HashKerningClassNamesFlex(SplineFont *sf, struct glif_name_index * class_name_hash, int capitalize);
int HashKerningClassNames(SplineFont *sf, struct glif_name_index * class_name_hash);
int HashKerningClassNamesCaps(SplineFont *sf, struct glif_name_index * class_name_hash);
#endif
#endif // 1
extern void JstfLangFree(struct jstf_lang *jl);
extern void JustifyFree(Justify *just);
extern void OtfNameListFree(struct otfname *on);
extern void OtfFeatNameListFree(struct otffeatname *fn);
extern struct otffeatname *findotffeatname(uint32 tag,SplineFont *sf);
extern void MarkSetFree(int cnt,char **classes,char **names);
extern void MarkClassFree(int cnt,char **classes,char **names);
extern void MMSetFreeContents(MMSet *mm);
extern void MMSetFree(MMSet *mm);
extern void MMSetClearSpecial(MMSet *mm);
extern void SplineRefigure3(Spline *spline);
extern void SplineRefigure(Spline *spline);
extern Spline *SplineMake3(SplinePoint *from, SplinePoint *to);
extern int SplinePointListIsClockwise(const SplineSet *spl);
extern void SplineCharLayerFindBounds(SplineChar *sc,int layer,DBounds *bounds);
extern void SplineCharFindBounds(SplineChar *sc,DBounds *bounds);
extern void SplineFontLayerFindBounds(SplineFont *sf,int layer,DBounds *bounds);
extern void SplineFontFindBounds(SplineFont *sf,DBounds *bounds);
extern void CIDLayerFindBounds(SplineFont *sf,int layer,DBounds *bounds);
extern void SplineSetQuickBounds(SplineSet *ss,DBounds *b);
extern void SplineCharLayerQuickBounds(SplineChar *sc,int layer,DBounds *bounds);
extern void SplineCharQuickBounds(SplineChar *sc, DBounds *b);
extern void SplinePointCategorize(SplinePoint *sp);
extern void SPLCategorizePoints(SplinePointList *spl);
extern SplinePointList *SplinePointListCopy1(const SplinePointList *spl);
extern SplinePointList *SplinePointListCopy(const SplinePointList *base);
extern void BpTransform(BasePoint *to, BasePoint *from, real transform[6]);
/* The order of the enum elements below doesn't make much sense, but it's done*/
/*  this way to preserve binary compatibility */
enum transformPointType { tpt_OnlySelected, tpt_AllPoints, tpt_OnlySelectedInterpCPs };
/*
 * As SplinePointListTransform() does a few things, this is a mask to selectively be
 * able to disable some of them.
 */
enum transformPointMask {
    tpmask_dontFixControlPoints = 1 << 1,
    tpmask_operateOnSelectedBCP = 1 << 2
};
extern SplinePointList *SplinePointListTransform(SplinePointList *base, real transform[6], enum transformPointType allpoints );
extern SplinePointList *SplinePointListTransformExtended(SplinePointList *base, real transform[6],
							 enum transformPointType tpt, enum transformPointMask tpmask );
extern HintMask *HintMaskFromTransformedRef(RefChar *ref,BasePoint *trans,
	SplineChar *basesc,HintMask *hm);
extern SplinePointList *SPLCopyTranslatedHintMasks(SplinePointList *base,
	SplineChar *basesc, SplineChar *subsc, BasePoint *trans);
extern SplinePointList *SPLCopyTransformedHintMasks(RefChar *r,
	SplineChar *basesc, BasePoint *trans,int layer);
extern void RefCharFindBounds(RefChar *rf);
extern void SCReinstanciateRefChar(SplineChar *sc,RefChar *rf,int layer);
enum piecemeal_flags { pf_antialias=1, pf_bbsized=2, pf_ft_nohints=4, pf_ft_recontext=8 };
#define STD_BDF_PROPS_EMPTY { NULL, 0, 0 }

/* Two lines intersect in at most 1 point */
/* Two quadratics intersect in at most 4 points */
/* Two cubics intersect in at most 9 points */ /* Plus an extra space for a trailing -1 */
extern int SplinesIntersect(const Spline *s1, const Spline *s2, BasePoint pts[9],
	extended t1s[10], extended t2s[10]);
extern int _CubicSolve(const Spline1D *sp,bigreal sought,extended ts[3]);
extern int CubicSolve(const Spline1D *sp,bigreal sought,extended ts[3]);
/* Uses an iterative approximation */
extern extended IterateSplineSolve(const Spline1D *sp, extended tmin, extended tmax, extended sought_y);
/* Uses an iterative approximation and then tries to fix things up */
extern extended IterateSplineSolveFixup(const Spline1D *sp, extended tmin, extended tmax, extended sought_y);
extern void SplineFindExtrema(const Spline1D *sp, extended *_t1, extended *_t2 );

#define CURVATURE_ERROR	-1e9
extern bigreal SplineCurvature(Spline *s, bigreal t);

extern double CheckExtremaForSingleBitErrors(const Spline1D *sp, double t, double othert);
extern int Spline2DFindExtrema(const Spline *sp, extended extrema[4] );
extern int Spline2DFindPointsOfInflection(const Spline *sp, extended poi[2] );
extern void SplineRemoveExtremaTooClose(Spline1D *sp, extended *_t1, extended *_t2 );
extern void SCMakeDependent(SplineChar *dependent,SplineChar *base);
extern SplinePoint *SplineBisect(Spline *spline, extended t);
extern bigreal SplineLength(Spline *spline);
extern int SplineIsLinear(Spline *spline);
extern int SPInterpolate(const SplinePoint *sp);
enum ae_type { ae_all, ae_between_selected, ae_only_good, ae_only_good_rm_later };
extern int SpIsExtremum(SplinePoint *sp);
extern int Spline1DCantExtremeX(const Spline *s);
extern int Spline1DCantExtremeY(const Spline *s);
extern Spline *SplineAddExtrema(Spline *s,int always,real lenbound,
	real offsetbound,DBounds *b);
extern SplineFont *SplineFontEmpty(void);
extern void SFIncrementXUID(SplineFont *sf);
extern SplineSet *SplineSetReverse(SplineSet *spl);
extern void BP_HVForce(BasePoint *vector);
extern void SplineCharDefaultPrevCP(SplinePoint *base);
extern void SplineCharDefaultNextCP(SplinePoint *base);
extern void SplineCharTangentNextCP(SplinePoint *sp);
extern void SplineCharTangentPrevCP(SplinePoint *sp);
/**
 * This is like SPAdjustControl but you have not wanting to move the
 * BCP at all, but you would like the current location of the passed
 * BCP to reshape the spline through the splinepoint. For example, if
 * you drag the spline between two points then you might like to touch
 * the inside BCP between the two splinepoints to reshape the whole
 * curve through a curve point.
 */
extern void SPTouchControl(SplinePoint *sp,BasePoint *which, int order2);
extern void SPAdjustControl(SplinePoint *sp,BasePoint *cp, BasePoint *to,int order2);

extern SplineSet *SSttfApprox(SplineSet *ss);
extern SplineSet *SSPSApprox(SplineSet *ss);
extern SplineSet *SplineSetsPSApprox(SplineSet *ss);
extern void SplineRefigure2(Spline *spline);
extern void SplineRefigureFixup(Spline *spline);
extern Spline *SplineMake2(SplinePoint *from, SplinePoint *to);
extern Spline *SplineMake(SplinePoint *from, SplinePoint *to, int order2);
extern void SCConvertToOrder2(SplineChar *sc);
extern void SFConvertToOrder2(SplineFont *sf);
extern int IntersectLines(BasePoint *inter,
	BasePoint *line1_1, BasePoint *line1_2,
	BasePoint *line2_1, BasePoint *line2_2);
extern int IntersectLinesClip(BasePoint *inter,
	BasePoint *line1_1, BasePoint *line1_2,
	BasePoint *line2_1, BasePoint *line2_2);

extern double BlueScaleFigureForced(struct psdict *private_,real bluevalues[], real otherblues[]);
extern double BlueScaleFigure(struct psdict *private_,real bluevalues[], real otherblues[]);
extern void FindBlues( SplineFont *sf, int layer, real blues[14], real otherblues[10]);
extern void QuickBlues(SplineFont *sf, int layer, BlueData *bd);
extern void FindHStems( SplineFont *sf, real snaps[12], real cnt[12]);
extern void FindVStems( SplineFont *sf, real snaps[12], real cnt[12]);
extern void SCGuessHintInstancesList(SplineChar *sc,int layer,StemInfo *hstem,StemInfo *vstem,DStemInfo *dstem,int hvforce,int dforce);
extern real HIlen( StemInfo *stems);
extern real HIoverlap( HintInstance *mhi, HintInstance *thi);
extern int StemListAnyConflicts(StemInfo *stems);
extern HintInstance *HICopyTrans(HintInstance *hi, real mul, real offset);

typedef struct bluezone {
    real base;
    int cvtindex;
    real family_base;      /* NaN if none */
    int family_cvtindex;
    real overshoot;        /* relative to baseline, NOT to base */
    int highest;           /* used in autoinstructing for HStem positioning */
    int lowest;            /* as above */
} BlueZone;

typedef struct stdstem {
    real width;            /* -1 if none */
    int cvtindex;
    struct stdstem *snapto;/* NULL means stem isn't snapped to any other */
    int stopat;            /* at which ppem stop snapping to snapto */
} StdStem;

typedef struct globalinstrct {
    SplineFont *sf;
    int layer;
    BlueData *bd;
    double fudge;

    /* Did we initialize the tables needed? 'maxp' is skipped because */
    /* its initialization always succeeds. */
    int cvt_done;
    int fpgm_done;
    int prep_done;

    /* PS private data with truetype-specific information added */
    BlueZone blues[12];    /* like in BlueData */
    int      bluecnt;
    StdStem  stdhw;
    StdStem  *stemsnaph;   /* StdHW excluded */
    int      stemsnaphcnt;
    StdStem  stdvw;
    StdStem  *stemsnapv;   /* StdVW excluded */
    int      stemsnapvcnt;
} GlobalInstrCt;

extern void InitGlobalInstrCt( GlobalInstrCt *gic,SplineFont *sf,int layer,
	BlueData *bd );
extern void FreeGlobalInstrCt( GlobalInstrCt *gic );
extern void NowakowskiSCAutoInstr( GlobalInstrCt *gic,SplineChar *sc );

extern void SCClearHintMasks(SplineChar *sc,int layer,int counterstoo);
extern void SCFigureHintMasks(SplineChar *sc,int layer);
extern void _SplineCharAutoHint( SplineChar *sc, int layer, BlueData *bd, struct glyphdata *gd2, int gen_undoes );
extern void SplineCharAutoHint( SplineChar *sc,int layer, BlueData *bd);
extern void SFSCAutoHint( SplineChar *sc,int layer,BlueData *bd);
extern void SplineFontAutoHint( SplineFont *sf, int layer);
extern void SplineFontAutoHintRefs( SplineFont *sf, int layer);
extern int SplineFontIsFlexible(SplineFont *sf,int layer, int flags);
extern int SCDrawsSomething(SplineChar *sc);
extern int SCWorthOutputting(SplineChar *sc);
extern void SFLigaturePrepare(SplineFont *sf);
extern void SFLigatureCleanup(SplineFont *sf);
extern void SFKernClassTempDecompose(SplineFont *sf,int isv);
extern void SFKernCleanup(SplineFont *sf,int isv);

typedef struct sfd_getfontmetadatadata
{

    // these indicate if we saw some metadata or not.
    // perhaps the caller wants to do something special
    // if the metadata was present/missing.
    int hadtimes;
    int had_layer_cnt;

    // state that is mostly interesting to SFD_GetFontMetaData() only
    struct Base*        last_base;
    struct basescript*  last_base_script;
    OTLookup*           lastpotl;
    OTLookup*           lastsotl;
    KernClass*          lastkc;
    KernClass*          lastvkc;
    struct ff_glyphclasses* lastgroup;
    struct ff_rawoffsets* lastgroupkern;
    struct ff_rawoffsets* lastgroupvkern;
    FPST*               lastfp;
    ASM*                lastsm;
    struct ttf_table*   lastttf[2];

} SFD_GetFontMetaDataData;
extern void SFD_GetFontMetaDataData_Init( SFD_GetFontMetaDataData* d );
extern bool SFD_GetFontMetaData( FILE *sfd,
                                 char *tok,
                                 SplineFont *sf,
                                 SFD_GetFontMetaDataData* d );
extern const char*FindUnicharName(void);
extern Encoding *_FindOrMakeEncoding(const char *name,int make_it);
extern Encoding *FindOrMakeEncoding(const char *name);
extern MacFeat *SFDParseMacFeatures(FILE *sfd, char *tok);
extern SplineFont *SFDRead(char *filename);
extern SplineFont *_SFDRead(char *filename,FILE *sfd);
enum ttfflags { ttf_onlystrikes=1, ttf_onlyonestrike=2, ttf_onlykerns=4, ttf_onlynames=8 };
extern uint16 _MacStyleCode( const char *styles, SplineFont *sf, uint16 *psstyle );
extern uint16 MacStyleCode( SplineFont *sf, uint16 *psstyle );

extern int getAdobeEnc(const char *name);

extern void MatMultiply(real m1[6], real m2[6], real to[6]);

extern void GlyphHashFree(SplineFont *sf);
extern SplineChar *SFHashName(SplineFont *sf,const char *name);
extern int SFFindGID(SplineFont *sf, int unienc, const char *name );
extern int SFCIDFindCID(SplineFont *sf, int unienc, const char *name );
extern SplineChar *SFGetChar(SplineFont *sf, int unienc, const char *name );
extern int SFFindExistingSlot(SplineFont *sf, int unienc, const char *name );
extern int SFHasCID(SplineFont *sf, int cid);

extern void PSCharsFree(struct pschars *chrs);
extern void PSDictFree(struct psdict *chrs);
extern char *PSDictHasEntry(struct psdict *dict, const char *key);
extern int PSDictSame(struct psdict *dict1, struct psdict *dict2);

struct cidmap;			/* private structure to encoding.c */

int getushort(FILE *ttf);
int32 getlong(FILE *ttf);
void putshort(FILE *file,int sval);
void putlong(FILE *file,int val);
void putfixed(FILE *file,real dval);
int ttfcopyfile(FILE *ttf, FILE *other, int pos, const char *table_name);

extern int UniFromName(const char *name,enum uni_interp interp, Encoding *encname);
extern NameList *DefaultNameListForNewFonts(void);
extern NameList *NameListByName(const char *name);
extern int  SSTtfNumberPoints(SplineSet *ss);
extern int  SCNumberPoints(SplineChar *sc,int layer);
extern int  SCPointsNumberedProperly(SplineChar *sc,int layer);

int SFFigureDefWidth(SplineFont *sf, int *_nomwid);

extern int ClassesMatch(int cnt1,char **classes1,int cnt2,char **classes2);
extern FPST *FPSTGlyphToClass(FPST *fpst);

extern ASM *ASMFromOpenTypeForms(SplineFont *sf,uint32 script);
extern ASM *ASMFromFPST(SplineFont *sf,FPST *fpst,int ordered);

extern char *utf8_verify_copy(const char *str);

extern char *Utf8ToMacStr(const char *ustr,int macenc,int maclang);
extern uint8 MacEncFromMacLang(int maclang);
extern uint16 WinLangToMac(int winlang);
extern MacFeat *FindMacFeature(SplineFont *sf, int feat,MacFeat **secondary);
extern struct macsetting *FindMacSetting(SplineFont *sf, int feat, int set,struct macsetting **secondary);

extern int32 UniFromEnc(int enc, Encoding *encname);
extern int32 EncFromUni(int32 uni, Encoding *encname);

/* Colinear & between */

enum psstrokeflags { /* sf_removeoverlap=2,*/ sf_handle_eraser=4,
	sf_correctdir=8, sf_clearbeforeinput=16 };

extern int MMValid(MMSet *mm,int complain);

enum Compare_Ret {	SS_DiffContourCount	= 1,
			SS_MismatchOpenClosed	= 2,
			SS_DisorderedContours	= 4,
			SS_DisorderedStart	= 8,
			SS_DisorderedDirection	= 16,
			SS_PointsMatch		= 32,
			SS_ContourMatch		= 64,
			SS_NoMatch		= 128,
			SS_RefMismatch		= 256,
			SS_WidthMismatch	= 512,
			SS_VWidthMismatch	= 1024,
			SS_HintMismatch		= 2048,
			SS_HintMaskMismatch	= 4096,
			SS_LayerCntMismatch	= 8192,
			SS_ContourMismatch	= 16384,
			SS_UnlinkRefMatch	= 32768,

			BC_DepthMismatch	= 1<<16,
			BC_BoundingBoxMismatch	= 2<<16,
			BC_BitmapMismatch	= 4<<16,
			BC_NoMatch		= 8<<16,
			BC_Match		= 16<<16,

			SS_RefPtMismatch	= 32<<16
		};

enum font_compare_flags { fcf_outlines=1, fcf_exact=2, fcf_warn_not_exact=4,
	fcf_hinting=8, fcf_hintmasks=0x10, fcf_hmonlywithconflicts=0x20,
	fcf_warn_not_ref_exact=0x40,
	fcf_bitmaps=0x80, fcf_names = 0x100, fcf_gpos=0x200, fcf_gsub=0x400,
	fcf_adddiff2sf1=0x800, fcf_addmissing=0x1000 };

# if HANYANG
extern void SFDDumpCompositionRules(FILE *sfd,struct compositionrules *rules);
extern struct compositionrules *SFDReadCompositionRules(FILE *sfd);
extern void SFModifyComposition(SplineFont *sf);
extern void SFBuildSyllables(SplineFont *sf);
# endif

extern void SFTimesFromFile(SplineFont *sf,FILE *);

extern int SFHasInstructions(SplineFont *sf);
extern int RefDepth(RefChar *ref,int layer);

extern uint32 *SFScriptsInLookups(SplineFont *sf,int gpos);
extern uint32 *SFLangsInScript(SplineFont *sf,int gpos,uint32 script);
extern uint32 *SFFeaturesInScriptLang(SplineFont *sf,int gpos,uint32 script,uint32 lang);
extern OTLookup **SFLookupsInScriptLangFeature(SplineFont *sf,int gpos,uint32 script,uint32 lang, uint32 feature);
extern SplineChar **SFGlyphsWithPSTinSubtable(SplineFont *sf,struct lookup_subtable *subtable);
extern void SFFindUnusedLookups(SplineFont *sf);
extern struct lookup_subtable *SFFindLookupSubtable(SplineFont *sf,char *name);
extern struct lookup_subtable *SFFindLookupSubtableAndFreeName(SplineFont *sf,char *name);
extern OTLookup *SFFindLookup(SplineFont *sf,char *name);
struct scriptlanglist *SLCopy(struct scriptlanglist *sl);
struct scriptlanglist *SListCopy(struct scriptlanglist *sl);
extern FeatureScriptLangList *FeatureListCopy(FeatureScriptLangList *fl);
extern int DefaultLangTagInOneScriptList(struct scriptlanglist *sl);

extern RefChar *HasUseMyMetrics(SplineChar *sc,int layer);

extern EncMap *EncMapFromEncoding(SplineFont *sf,Encoding *enc);

extern void InitSimpleStuff(void);

extern struct math_constants_descriptor {
    char *script_name;
    int offset;
    int devtab_offset;
} math_constants_descriptor[];

#define MATH_CONSTANTS_DESCRIPTOR_EMPTY { NULL, 0, 0}

extern const char *knownweights[], *realweights[], **noticeweights[];

struct lang_frequencies;
extern struct gradient *GradientCopy(struct gradient *old,real transform[6]);
extern void GradientFree(struct gradient *grad);
extern struct pattern *PatternCopy(struct pattern *old,real transform[6]);
extern void PatternFree(struct pattern *pat);
extern void BrushCopy(struct brush *into, struct brush *from,real transform[6]);
extern void PenCopy(struct pen *into, struct pen *from,real transform[6]);

extern bigreal SFCapHeight(SplineFont *sf, int layer, int return_error);
extern bigreal SFXHeight(SplineFont *sf, int layer, int return_error);

/**
 * Visitor for SPLFirstVisitSplines()
 */
typedef void (*SPLFirstVisitSplinesVisitor)( SplinePoint* splfirst, Spline* s, void* udata );

/**
 * Visitor Function: print debug information about each spline
 */

/**
 * Given a SplinePointList* that you want to visit each spline in the
 * iteration is not as simple as it could be, so you can call this
 * function passing spl->first as 'splfirst' and a visitor function
 * which will see each spline in the splfirst colleciton.
 *
 * For debug, you can pass SPLFirstVisitorDebug which will print
 * information for each item in the splfirst collection.
 *
 * You can pass any arbitrary data in as udata and SPLFirstVisit()
 * will pass that udata to your visitor function without change. If
 * you want a return value from your visitor, pass a pointer to a
 * struct as udata. eg:
 *
 * typedef struct SPLFirstVisitorFoundSoughtDataS
 * {
 *    SplinePoint* sought;
 *    int found;
 * } SPLFirstVisitorFoundSoughtData;
 *
 * // ...
 *
 *	SPLFirstVisitorFoundSoughtData d;
 *	d.sought = sought;
 *	d.found  = 0;
 *	SPLFirstVisit( spl->first, SPLFirstVisitorFoundSought, &d );
 *	if( d.found )
 *           return 1;
 *
 */
extern void SPLFirstVisitSplines( SplinePoint* splfirst, SPLFirstVisitSplinesVisitor f, void* udata );

/**
 * Visitor for SPLFirstVisitPoints()
 */
typedef void (*SPLFirstVisitPointsVisitor)( SplinePoint* splfirst, Spline* s, SplinePoint* sp, void* udata );


/**
 * Applies a visitor to the container and returns false if no point in the SPL
 * has an x coordinate of 'x'.
 */
extern SplinePoint* SplinePointListContainsPointAtX( SplinePointList* container, real x );


/**
 * It is like a == b, but also true if a is within
 * tolerence of b.
 */
extern bool equalWithTolerence( real a, real b, real tolerence );

// The following functions are in splineutil.c at present.
char * upper_case(const char * input);

#include "ustring.h"

#ifdef _WIN32
#define BAD_LOCALE_HACK
typedef char* locale_t;
#define LC_GLOBAL_LOCALE ((locale_t)-1)
#define LC_ALL_MASK LC_ALL
#define LC_COLLATE_MASK LC_COLLATE
#define LC_CTYPE_MASK LC_CTYPE
#define LC_MONETARY_MASK LC_MONETARY
#define LC_NUMERIC_MASK LC_NUMERIC
#define LC_TIME_MASK LC_TIME
#endif

static inline void switch_to_c_locale(locale_t * tmplocale_p, locale_t * oldlocale_p) {
#ifndef BAD_LOCALE_HACK
  *tmplocale_p = newlocale(LC_NUMERIC_MASK, "C", NULL);
  if (*tmplocale_p == NULL) fprintf(stderr, "Failed to create temporary locale.\n");
  else if ((*oldlocale_p = uselocale(*tmplocale_p)) == NULL) {
    fprintf(stderr, "Failed to change locale.\n");
    freelocale(*tmplocale_p); *tmplocale_p = NULL;
  }
#else
  // Yes, it is dirty. But so is an operating system that doesn't support threaded locales.
  *oldlocale_p = (locale_t)copy(setlocale(LC_NUMERIC_MASK, "C"));
  if (*oldlocale_p == NULL) fprintf(stderr, "Failed to change locale.\n");
#endif
}

static inline void switch_to_old_locale(locale_t * tmplocale_p, locale_t * oldlocale_p) {
#ifndef BAD_LOCALE_HACK
  if (*oldlocale_p != NULL) { uselocale(*oldlocale_p); } else { uselocale(LC_GLOBAL_LOCALE); }
  *oldlocale_p = NULL; // This ends the lifecycle of the temporary old locale storage.
  if (*tmplocale_p != NULL) { freelocale(*tmplocale_p); *tmplocale_p = NULL; }
#else
  if (*oldlocale_p != NULL) {
    setlocale(LC_NUMERIC_MASK, (char*)(*oldlocale_p));
    free((char*)(*oldlocale_p));
    *oldlocale_p = NULL;
  }
#endif
}

#if 0
#define DECLARE_TEMP_LOCALE() char oldloc[25];
#define SWITCH_TO_C_LOCALE() strncpy( oldloc,setlocale(LC_NUMERIC,NULL),24 ); oldloc[24]='\0'; setlocale(LC_NUMERIC,"C");
#define SWITCH_TO_OLD_LOCALE() setlocale(LC_NUMERIC,oldloc);
#else
#define DECLARE_TEMP_LOCALE() locale_t tmplocale; locale_t oldlocale; // Declare temporary locale storage.
#define SWITCH_TO_C_LOCALE() switch_to_c_locale(&tmplocale, &oldlocale); // Switch to the C locale temporarily and cache the old locale.
#define SWITCH_TO_OLD_LOCALE() switch_to_old_locale(&tmplocale, &oldlocale); // Switch to the cached locale.
#endif



#endif
