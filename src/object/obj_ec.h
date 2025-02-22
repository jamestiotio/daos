/**
 * (C) Copyright 2019-2023 Intel Corporation.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */

#ifndef __OBJ_EC_H__
#define __OBJ_EC_H__

#include <daos_types.h>
#include <daos/object.h>
#include <daos_obj.h>

#include <isa-l.h>

/** MAX number of data cells */
#define OBJ_EC_MAX_K		(64)
/** MAX number of parity cells */
#define OBJ_EC_MAX_P		(8)
#define OBJ_EC_MAX_M		(OBJ_EC_MAX_K + OBJ_EC_MAX_P)
/** Length of target bitmap */
#define OBJ_TGT_BITMAP_LEN						\
	(roundup(((OBJ_EC_MAX_M) / NBBY), 8))

/* EC parity is stored in a private address range that is selected by setting
 * the most-significant bit of the offset (an unsigned long). This effectively
 * limits the addressing of user extents to the lower 63 bits of the offset
 * range. The client stack should enforce this limitation.
 */
#define PARITY_INDICATOR DAOS_EC_PARITY_BIT

/** EC codec for object EC encoding/decoding */
struct obj_ec_codec {
	/** encode matrix, can be used to generate decode matrix */
	unsigned char		*ec_en_matrix;
	/**
	 * GF (galois field) tables, pointer to array of input tables generated
	 * from coding coefficients. Needed for both encoding and decoding.
	 */
	unsigned char		*ec_gftbls;
};

/** Shard IO descriptor */
struct obj_shard_iod {
	/** tgt index [0, k+p) */
	uint32_t		 siod_tgt_idx;
	/** start index in extend array in daos_iod_t */
	uint32_t		 siod_idx;
	/** the byte offset of this shard's data to the sgl/bulk */
	uint64_t		 siod_off;
	/** number of extends in extend array in daos_iod_t */
	uint32_t		 siod_nr;
};

struct obj_iod_array {
	/* number of iods (oia_iods) */
	uint32_t		 oia_iod_nr;
	/* number obj iods (oia_oiods) */
	uint32_t		 oia_oiod_nr;
	daos_iod_t		*oia_iods;
	struct dcs_iod_csums	*oia_iod_csums;
	struct obj_io_desc	*oia_oiods;
	/* byte offset array for target, need this info after RPC dispatched
	 * to specific target server as there is no oiod info already.
	 * one for each iod, NULL for replica.
	 */
	uint64_t		*oia_offs;
};

/** Evenly distributed for EC full-stripe-only mode */
#define OBJ_SIOD_EVEN_DIST	((uint32_t)1 << 0)
/** Flag used only for proc func, to only proc to one specific target */
#define OBJ_SIOD_PROC_ONE	((uint32_t)1 << 1)
/** Flag of single value EC */
#define OBJ_SIOD_SINGV		((uint32_t)1 << 2)

/**
 * Object IO descriptor.
 * NULL for replica obj, as each shard/tgt with same extends in iod.
 * Non-NULL for EC obj to specify IO descriptor for different targets.
 */
struct obj_io_desc {
	/**
	 * number of shard IODs involved for this object IO.
	 * for EC obj, if there is only one target for example partial update or
	 * fetch targeted with only one shard, oiod_siods should be NULL as need
	 * not carry extra info.
	 */
	uint16_t		 oiod_nr;
	/**
	 * the target index [0, tgt_nr), only used for EC evenly distributed
	 * single value.
	 */
	uint16_t		 oiod_tgt_idx;
	/**
	 * Flags, OBJ_SIOD_EVEN_DIST is for a special case that the extends
	 * only cover full stripe(s), then each target has same number of
	 * extends in the extend array (evenly distributed).
	 */
	uint32_t		 oiod_flags;
	/** shard IOD array */
	struct obj_shard_iod	*oiod_siods;
};

/** To record the recxs in original iod which include full stripes */
struct obj_ec_recx {
	/** index of the recx in original iod::iod_recxs array */
	uint32_t		oer_idx;
	/** number of full stripes in oer_recx */
	uint32_t		oer_stripe_nr;
	/**
	 * the byte offset of the start of oer_recx, in the extends covered by
	 * iod::iod_recxs array. Can be used to find corresponding sgl offset.
	 */
	uint64_t		oer_byte_off;
	/** the extend that includes the full stripes */
	daos_recx_t		oer_recx;
};

/** To record all full stripe recxs in one iod */
struct obj_ec_recx_array {
	/** number of recxs for each tgt */
	uint32_t		*oer_tgt_recx_nrs;
	/** start recxs idx for each tgt */
	uint32_t		*oer_tgt_recx_idxs;
	/** number of data tgts and parity tgts */
	uint32_t		 oer_k;
	uint32_t		 oer_p;
	/** matched index last time, only used for ec_recx_with_full_stripe */
	uint32_t		 oer_last;
	/** parity buffer pointer array, one for each parity tgt */
	uint8_t			*oer_pbufs[OBJ_EC_MAX_P];
	/** total number of full stripes in oer_recxs array */
	uint32_t		 oer_stripe_total;
	/** number of valid items in oer_recxs array */
	uint32_t		 oer_nr;
	/** full stripe recx array */
	struct obj_ec_recx	*oer_recxs;
};

/**
 * Object target oiod/offset.
 * Only used as temporary buffer to facilitate the RPC proc.
 */
struct obj_tgt_oiod {
	/* target idx [0, k + p) */
	uint32_t		 oto_tgt_idx;
	/* number of iods */
	uint32_t		 oto_iod_nr;

	/**
	 * target original idx (0, k + p), since the fetch request might be
	 * re-directed to the parity shard.
	 **/
	uint32_t		oto_orig_tgt_idx;

	/* offset array, oto_iod_nr offsets for each target */
	uint64_t		*oto_offs;
	/* oiod array, oto_iod_nr oiods for each target,
	 * each oiod with just one siod.
	 */
	struct obj_io_desc	*oto_oiods;
};

/**
 * Split obj request (only used on leader shard for obj update).
 * For object update, client sends update request to leader, the leader need to
 * split it for different targets before dispatch.
 */
struct obj_ec_split_req {
	uint32_t		 osr_start_shard;
	/* forward targets' tgt_oiods */
	struct obj_tgt_oiod	*osr_tgt_oiods;
	/* leader shard's iods */
	daos_iod_t		*osr_iods;
	/* leader shard's offsets (one for each iod) */
	uint64_t		*osr_offs;
	/* leader shard's iod_csums */
	struct dcs_iod_csums	*osr_iod_csums;
	/* csum_info for singvs */
	struct dcs_csum_info	*osr_singv_cis;
};

/**
 * Segment sorter to sort segments per target.
 * In EC IO request reassemble, it needs to regenerate a new sgl with iovs
 * grouped by target and each target's segments need to be sorted to be same
 * order as recxs. Before the sorting it does not know each target's segment
 * number. This sorter is to facilitate the handing.
 */
struct obj_ec_seg_head {
	uint32_t		 esh_tgt_idx;
	uint32_t		 esh_seg_nr;
	uint32_t		 esh_first;
	uint32_t		 esh_last;
};

struct obj_ec_seg {
	d_iov_t			 oes_iov;
	int32_t			 oes_next;
};

struct obj_ec_seg_sorter {
	uint32_t		 ess_seg_nr;
	uint32_t		 ess_seg_nr_total;
	uint32_t		 ess_tgt_nr;
	uint32_t		 ess_tgt_nr_total;
	struct obj_ec_seg_head	*ess_tgts;
	struct obj_ec_seg	*ess_segs;
};
#define OBJ_EC_SEG_NIL		 (-1)

/** ISAL codec for EC data recovery */
struct obj_ec_recov_codec {
	unsigned char		*er_gftbls;	/* GF tables */
	unsigned char		*er_de_matrix;	/* decode matrix */
	unsigned char		*er_inv_matrix;	/* invert matrix */
	unsigned char		*er_b_matrix;	/* temporary b matrix */
	uint32_t		*er_dec_idx;	/* decode index */
	uint32_t		*er_err_list;	/* target idx list in error */
	bool			*er_in_err;	/* boolean array for targets */
	uint32_t		 er_nerrs;	/* #targets in error */
	uint32_t		 er_data_nerrs; /* #data-targets in error */
};

/* EC recovery task */
struct obj_ec_recov_task {
	daos_iod_t		ert_iod;
	/* the original user iod pointer, used for the case that in singv
	 * degraded fetch, set the iod_size.
	 */
	daos_iod_t		*ert_oiod;
	d_sg_list_t		ert_sgl;
	daos_epoch_t		ert_epoch;
	daos_handle_t		ert_th;		/* read-only tx handle */
	uint32_t		ert_snapshot:1;	/* For snapshot flag */
};

/** EC obj IO failure information */
struct obj_ec_fail_info {
	/* missed (to be recovered) recx list */
	struct daos_recx_ep_list	*efi_recx_lists;
	/* list of error targets */
	uint32_t			*efi_tgt_list;
	/* number of lists in efi_recx_lists/efi_stripe_lists, equal to #iods */
	uint32_t			 efi_nrecx_lists;
	/* number of error targets */
	uint32_t			 efi_ntgts;
	struct obj_ec_recov_codec	*efi_recov_codec;
	/* to be recovered full-stripe list */
	struct daos_recx_ep_list	*efi_stripe_lists;
	/* The buffer for all the full-stripes in efi_stripe_lists.
	 * One iov for each recx_ep (with 1 or more stripes), for each stripe
	 * it contains ((k + p) * cell_byte_size) memory.
	 */
	d_sg_list_t			*efi_stripe_sgls;
	uint32_t			efi_stripe_sgls_nr;
	/* For each daos_recx_ep in efi_stripe_lists will create one recovery
	 * task to fetch the data from servers.
	 */
	struct obj_ec_recov_task	*efi_recov_tasks;
	uint32_t			 efi_recov_ntasks;
};

int
obj_ec_grp_start(uint16_t layout_ver, uint64_t hash, uint32_t grp_size);

struct obj_reasb_req;

/** Query the number of records in EC full stripe */
#define obj_ec_stripe_rec_nr(oca)					\
	((oca)->u.ec.e_k * (oca)->u.ec.e_len)
/** Query the number of records in one EC cell/target */
#define obj_ec_cell_rec_nr(oca)						\
	((oca)->u.ec.e_len)
/** Query the number of targets of EC obj class */
#define obj_ec_tgt_nr(oca)						\
	((oca)->u.ec.e_k + (oca)->u.ec.e_p)
/** Query the number of data targets of EC obj class */
#define obj_ec_data_tgt_nr(oca)						\
	((oca)->u.ec.e_k)
/** Query the number of parity targets of EC obj class */
#define obj_ec_parity_tgt_nr(oca)					\
	((oca)->u.ec.e_p)
/** Query the number of bytes in EC cell */
#define obj_ec_cell_bytes(iod, oca)					\
	(((oca)->u.ec.e_len) * ((iod)->iod_size))
/** Query the tgt idx of data cell for daos recx idx */
#define obj_ec_tgt_of_recx_idx(idx, stripe_rec_nr, e_len)		\
	(((idx) % (stripe_rec_nr)) / (e_len))

/* Get shard idx according to dkey hash within one group. logical idx -> physical idx */
#define obj_ec_shard_idx_by_layout_ver(layout_ver, dkey_hash, oca, t_idx)			\
	((obj_ec_grp_start(layout_ver, dkey_hash, obj_ec_tgt_nr(oca)) + t_idx) % obj_ec_tgt_nr(oca))

#define obj_ec_shard_idx(obj, dkey_hash, t_idx)	\
	obj_ec_shard_idx_by_layout_ver(obj->cob_layout_version, dkey_hash, &obj->cob_oca, t_idx)

/* Get shard index within the object layout */
#define obj_ec_shard(obj, dkey_hash, grp_idx, t_idx)				\
	(grp_idx * obj_ec_tgt_nr(&obj->cob_oca) + obj_ec_shard_idx(obj, dkey_hash, t_idx))

/* Get the parity index within one group, logical -> physical */
#define obj_ec_parity_idx(obj, dkey_hash, p_idx)					\
	((obj_ec_shard_idx(obj, dkey_hash, 0) + obj_ec_data_tgt_nr(&obj->cob_oca) + p_idx) %	\
	  obj_ec_tgt_nr(&obj->cob_oca))

/* Get parity start index according to dkey hash within one group */
#define obj_ec_parity_start(obj, dkey_hash) obj_ec_parity_idx(obj, dkey_hash, 0)

/* Get parity shard index within the object layout */
#define obj_ec_parity_shard(obj, dkey_hash, grp_idx, p_idx)		\
	((grp_idx) * obj_ec_tgt_nr(&obj->cob_oca) + obj_ec_parity_idx(obj, dkey_hash, p_idx))

/* Get the logical offset of shard within one group, physical idx -> logical idx */
#define obj_ec_shard_off(obj, dkey_hash, shard)				\
	((shard % obj_ec_tgt_nr(&obj->cob_oca) + obj_ec_tgt_nr(&obj->cob_oca) -		\
	 obj_ec_shard_idx(obj, dkey_hash, 0)) % obj_ec_tgt_nr(&obj->cob_oca))

/* Get the logical offset of the tgt_idx by start target of EC, physical idx -> logical idx */
#define obj_ec_shard_off_by_start(tgt_idx, oca, start_tgt)		\
	((tgt_idx + obj_ec_tgt_nr(oca) - start_tgt) % obj_ec_tgt_nr(oca))

/* Get the logical offset of tgt_idx by gl version. */
#define obj_ec_shard_off_by_layout_ver(layout_ver, dkey_hash, oca, tgt_idx)	\
	((tgt_idx + obj_ec_tgt_nr(oca) -					\
	 obj_ec_grp_start(layout_ver, dkey_hash, obj_ec_tgt_nr(oca))) %		\
	 obj_ec_tgt_nr(oca))
/**
 * Query the mapped VOS recx idx on data cells of daos recx idx, it is also the
 * parity's VOS recx idx on parity cells (the difference is parity's VOS recx
 * idx with highest bit set, see PARITY_INDICATOR.
 * Note that for replicated data on parity cells the VOS idx is unmapped
 * original daos recx idx to facilitate aggregation.
 */
#define obj_ec_idx_daos2vos(idx, stripe_rec_nr, e_len)			\
	((((idx) / (stripe_rec_nr)) * (e_len)) + ((idx) % (e_len)))
/** Query the original daos idx of mapped VOS index */
#define obj_ec_idx_vos2daos(vos_idx, stripe_rec_nr, e_len, tgt_idx)	       \
	((((vos_idx) / (e_len)) * stripe_rec_nr) + (tgt_idx) * (e_len) +       \
	 (vos_idx) % (e_len))

#define obj_ec_idx_parity2daos(vos_off, e_len, stripe_rec_nr)		\
	(((vos_off) / e_len) * stripe_rec_nr)
/**
 * Threshold size of EC single-value layout (even distribution).
 * When record_size <= OBJ_EC_SINGV_EVENDIST_SZ then stored in one data
 * target, or will evenly distributed to all data targets.
 */
#define OBJ_EC_SINGV_EVENDIST_SZ(data_tgt_nr)	(((data_tgt_nr) / 8 + 1) * 4096)
/** Alignment size of sing value local size */
#define OBJ_EC_SINGV_CELL_ALIGN			(8)

/** Local rec size, padding bytes and offset in the global record */
struct obj_ec_singv_local {
	uint64_t	esl_off;
	uint64_t	esl_size;
	uint32_t	esl_bytes_pad;
};

/** Query the target index for small sing-value record */
#define obj_ec_singv_small_idx(obj, dkey_hash, iod)	obj_ec_shard_idx(obj, dkey_hash, 0)

/* check EC data shard by its logical offset */
static inline bool
is_ec_data_shard_by_tgt_off(uint32_t tgt_off, struct daos_oclass_attr *oca)
{
	D_ASSERT(daos_oclass_is_ec(oca));
	return tgt_off < obj_ec_data_tgt_nr(oca);
}

static inline bool
is_ec_data_shard_by_layout_ver(uint16_t layout_ver, uint64_t dkey_hash,
			       struct daos_oclass_attr *oca, uint32_t shard)
{
	D_ASSERT(daos_oclass_is_ec(oca));
	return obj_ec_shard_off_by_layout_ver(layout_ver, dkey_hash, oca, shard) <
	       obj_ec_data_tgt_nr(oca);
}

/* check EC parity shard by its logical offset */
static inline bool
is_ec_parity_shard_by_tgt_off(uint32_t tgt_off, struct daos_oclass_attr *oca)
{
	D_ASSERT(daos_oclass_is_ec(oca));
	return tgt_off >= obj_ec_data_tgt_nr(oca);
}

static inline bool
is_ec_parity_shard_by_layout_ver(uint16_t layout_ver, uint64_t dkey_hash,
				 struct daos_oclass_attr *oca, uint32_t shard)
{
	D_ASSERT(daos_oclass_is_ec(oca));
	return obj_ec_shard_off_by_layout_ver(layout_ver, dkey_hash, oca, shard) >=
	       obj_ec_data_tgt_nr(oca);
}

/** Query if the single value record is stored in one data target */
static inline bool
obj_ec_singv_one_tgt(daos_size_t iod_size, d_sg_list_t *sgl,
		     struct daos_oclass_attr *oca)
{
	uint64_t size = OBJ_EC_SINGV_EVENDIST_SZ(obj_ec_data_tgt_nr(oca));

	if ((iod_size != DAOS_REC_ANY && iod_size <= size) ||
	    (sgl != NULL && daos_sgl_buf_size(sgl) <= size))
		return true;

	return false;
}

/* Query the cell size (#bytes) of evenly distributed singv */
static inline uint64_t
obj_ec_singv_cell_bytes(uint64_t rec_gsize, struct daos_oclass_attr *oca)
{
	uint32_t	data_tgt_nr = obj_ec_data_tgt_nr(oca);
	uint64_t	cell_size;

	cell_size = rec_gsize / data_tgt_nr;
	if ((rec_gsize % data_tgt_nr) != 0)
		cell_size++;
	cell_size = roundup(cell_size, OBJ_EC_SINGV_CELL_ALIGN);

	return cell_size;
}

/** Query local record size and needed padding for evenly distributed singv */
static inline void
obj_ec_singv_local_sz(uint64_t rec_gsize, struct daos_oclass_attr *oca,
		      uint32_t tgt_idx, struct obj_ec_singv_local *loc,
		      bool update)
{
	uint32_t	data_tgt_nr = obj_ec_data_tgt_nr(oca);
	uint64_t	cell_size;

	D_ASSERT(tgt_idx < obj_ec_tgt_nr(oca));

	cell_size = obj_ec_singv_cell_bytes(rec_gsize, oca);
	/* For update, the parity buffer is immediately following data buffer,
	 * to avoid insert extra sgl segment (for last data shard's padding).
	 * For fetch, fetching from parity shard is only for EC recovery, in
	 * that case it allocates enough buffer (obj_ec_singv_stripe_buf_size)
	 * and to simplify data recovery (avoid data movement for the case that
	 * last data shard with padding bytes) the parity data's offset in fetch
	 * buffer is aligned to cell size boundary.
	 */
	if (tgt_idx >= data_tgt_nr && update)
		loc->esl_off = rec_gsize + (tgt_idx - data_tgt_nr) * cell_size;
	else
		loc->esl_off = tgt_idx * cell_size;
	if (tgt_idx == data_tgt_nr - 1) {
		/* the last data target possibly with less size and padding */
		loc->esl_size = rec_gsize - (data_tgt_nr - 1) * cell_size;
		loc->esl_bytes_pad = cell_size - loc->esl_size;
	} else {
		loc->esl_size = cell_size;
		loc->esl_bytes_pad = 0;
	}
}

/** Query the number of data cells the recx covers */
static inline uint32_t
obj_ec_recx_cell_nr(daos_recx_t *recx, struct daos_oclass_attr *oca)
{
	uint64_t	recx_end, start, end;

	recx_end = recx->rx_idx + recx->rx_nr;
	start = roundup(recx->rx_idx, obj_ec_cell_rec_nr(oca));
	end = rounddown(recx_end, obj_ec_cell_rec_nr(oca));
	if (start > end)
		return 1;
	return (end - start) / obj_ec_cell_rec_nr(oca) +
	       ((recx->rx_idx % obj_ec_cell_rec_nr(oca)) != 0) +
	       ((recx_end % obj_ec_cell_rec_nr(oca)) != 0);
}

static inline int
obj_io_desc_init(struct obj_io_desc *oiod, uint32_t tgt_nr, uint32_t flags)
{
#if 0
	/* XXX refine it later */
	if (tgt_nr < 2 || flags == OBJ_SIOD_EVEN_DIST) {
		oiod->oiod_siods = NULL;
		oiod->oiod_nr = tgt_nr;
		oiod->oiod_flags = flags;
		return 0;
	}
#endif
	if ((flags & OBJ_SIOD_SINGV) == 0) {
		D_ALLOC_ARRAY(oiod->oiod_siods, tgt_nr);
		if (oiod->oiod_siods == NULL)
			return -DER_NOMEM;
	}
	oiod->oiod_flags = flags;
	oiod->oiod_nr = tgt_nr;
	return 0;
}

static inline void
obj_io_desc_fini(struct obj_io_desc *oiod)
{
	if (oiod == NULL)
		return;

	oiod->oiod_nr = 0;
	oiod->oiod_tgt_idx = 0;
	oiod->oiod_flags = 0;
	D_FREE(oiod->oiod_siods);
}

static inline struct obj_shard_iod *
obj_shard_iod_get(struct obj_io_desc *oiod, uint32_t tgt)
{
	int i;

	for (i = 0; i < oiod->oiod_nr; i++) {
		if (oiod->oiod_siods[i].siod_tgt_idx == tgt)
			return &oiod->oiod_siods[i];
	}

	return NULL;
}

/* translate the queried VOS shadow list to daos extents */
static inline void
obj_shadow_list_vos2daos(uint32_t nr, struct daos_recx_ep_list *lists,
			 struct daos_oclass_attr *oca)
{
	struct daos_recx_ep_list	*list;
	daos_recx_t			*recx;
	uint64_t			 stripe_rec_nr =
						obj_ec_stripe_rec_nr(oca);
	uint64_t			 cell_rec_nr =
						obj_ec_cell_rec_nr(oca);
	uint32_t			 i, j;

	if (lists == NULL)
		return;
	for (i = 0; i < nr; i++) {
		list = &lists[i];
		for (j = 0; j < list->re_nr; j++) {
			uint64_t end;

			recx = &list->re_items[j].re_recx;
			D_ASSERT((recx->rx_idx & PARITY_INDICATOR) != 0);
			recx->rx_idx &= ~PARITY_INDICATOR;
			end = roundup(DAOS_RECX_END(*recx), cell_rec_nr) *
			      obj_ec_data_tgt_nr(oca);
			recx->rx_idx = rounddown(recx->rx_idx, cell_rec_nr);
			recx->rx_idx = obj_ec_idx_vos2daos(recx->rx_idx,
						stripe_rec_nr, cell_rec_nr, 0);
			recx->rx_nr = end - recx->rx_idx;
		}
	}
}

/* Break iod's recxs on cell_size boundary, for the use case that translate
 * mapped VOS extend to original daos extent - one mapped VOS extend possibly
 * corresponds to multiple original dis-continuous daos extents.
 */
static inline int
obj_iod_break(daos_iod_t *iod, struct daos_oclass_attr *oca)
{
	daos_recx_t	*recx, *new_recx;
	uint64_t	 cell_size = obj_ec_cell_rec_nr(oca);
	uint64_t	 rec_nr;
	uint32_t	 i, j, stripe_nr;

	for (i = 0; i < iod->iod_nr; i++) {
		recx = &iod->iod_recxs[i];
		stripe_nr = obj_ec_recx_cell_nr(recx, oca);
		D_ASSERT(stripe_nr >= 1);
		if (stripe_nr == 1)
			continue;
		D_ALLOC_ARRAY(new_recx, stripe_nr + iod->iod_nr - 1);
		if (new_recx == NULL)
			return -DER_NOMEM;
		for (j = 0; j < i; j++)
			new_recx[j] = iod->iod_recxs[j];
		rec_nr = recx->rx_nr;
		for (j = 0; j < stripe_nr; j++) {
			if (j == 0) {
				new_recx[i].rx_idx = recx->rx_idx;
				new_recx[i].rx_nr = cell_size -
						    (recx->rx_idx % cell_size);
				rec_nr -= new_recx[i].rx_nr;
			} else {
				new_recx[i + j].rx_idx =
					new_recx[i + j - 1].rx_idx +
					new_recx[i + j - 1].rx_nr;
				D_ASSERT(new_recx[i + j].rx_idx % cell_size ==
					 0);
				if (j == stripe_nr - 1) {
					new_recx[i + j].rx_nr = rec_nr;
				} else {
					new_recx[i + j].rx_nr = cell_size;
					rec_nr -= cell_size;
				}
			}
		}
		for (j = i + 1; j < iod->iod_nr; j++)
			new_recx[j + stripe_nr - 1] = iod->iod_recxs[j];
		i += (stripe_nr - 1);
		iod->iod_nr += (stripe_nr - 1);
		D_FREE(iod->iod_recxs);
		iod->iod_recxs = new_recx;
	}

	return 0;
}

/* translate iod's recxs from unmapped daos extend to mapped vos extents */
static inline void
obj_iod_recx_daos2vos(uint32_t iod_nr, daos_iod_t *iods,
		      struct daos_oclass_attr *oca)
{
	daos_iod_t	*iod;
	daos_recx_t	*recx;
	uint64_t	 stripe_rec_nr = obj_ec_stripe_rec_nr(oca);
	uint64_t	 cell_rec_nr = obj_ec_cell_rec_nr(oca);
	uint32_t	 i, j;

	for (i = 0; i < iod_nr; i++) {
		iod = &iods[i];
		if (iod->iod_type == DAOS_IOD_SINGLE)
			continue;

		for (j = 0; j < iod->iod_nr; j++) {
			recx = &iod->iod_recxs[j];
			D_ASSERT((recx->rx_idx & PARITY_INDICATOR) == 0);
			recx->rx_idx = obj_ec_idx_daos2vos(recx->rx_idx,
							   stripe_rec_nr,
							   cell_rec_nr);
		}
	}
}

/* translate iod's recxs from mapped VOS extend to unmapped daos extents */
static inline int
obj_iod_recx_vos2daos(uint32_t iod_nr, daos_iod_t *iods, uint32_t tgt_idx,
		      struct daos_oclass_attr *oca)
{
	daos_iod_t	*iod;
	daos_recx_t	*recx;
	uint64_t	 stripe_rec_nr = obj_ec_stripe_rec_nr(oca);
	uint64_t	 cell_rec_nr = obj_ec_cell_rec_nr(oca);
	uint32_t	 i, j;
	int		 rc;

	for (i = 0; i < iod_nr; i++) {
		iod = &iods[i];
		if (iod->iod_type == DAOS_IOD_SINGLE)
			continue;

		rc = obj_iod_break(iod, oca);
		if (rc != 0) {
			D_ERROR("obj_iod_break failed, "DF_RC"\n", DP_RC(rc));
			return rc;
		}
		for (j = 0; j < iod->iod_nr; j++) {
			recx = &iod->iod_recxs[j];
			D_ASSERT((recx->rx_idx & PARITY_INDICATOR) == 0);
			recx->rx_idx = obj_ec_idx_vos2daos(recx->rx_idx,
						stripe_rec_nr, cell_rec_nr,
						tgt_idx);
		}
	}

	return 0;
}

static inline void
obj_iod_idx_vos2parity(uint32_t iod_nr, daos_iod_t *iods)
{
	daos_iod_t	*iod;
	daos_recx_t	*recx;
	uint32_t	 i, j;

	for (i = 0; i < iod_nr; i++) {
		iod = &iods[i];
		if (iod->iod_type == DAOS_IOD_SINGLE)
			continue;
		for (j = 0; j < iod->iod_nr; j++) {
			recx = &iod->iod_recxs[j];
			D_ASSERT((recx->rx_idx & PARITY_INDICATOR) == 0);
			recx->rx_idx |= PARITY_INDICATOR;
		}
	}
}

static inline void
obj_iod_idx_parity2vos(uint32_t iod_nr, daos_iod_t *iods)
{
	daos_iod_t	*iod;
	daos_recx_t	*recx;
	uint32_t	 i, j;

	for (i = 0; i < iod_nr; i++) {
		iod = &iods[i];
		if (iod->iod_type == DAOS_IOD_SINGLE)
			continue;
		for (j = 0; j < iod->iod_nr; j++) {
			recx = &iod->iod_recxs[j];
			D_ASSERT((recx->rx_idx & PARITY_INDICATOR) != 0);
			recx->rx_idx &= ~PARITY_INDICATOR;
		}
	}
}

static inline bool
obj_ec_tgt_in_err(uint32_t *err_list, uint32_t nerrs, uint16_t tgt_idx)
{
	uint32_t	i;

	for (i = 0; i < nerrs; i++) {
		if (err_list[i] == tgt_idx)
			return true;
	}
	return false;
}

/* obj_class.c */
int obj_ec_codec_init(void);
void obj_ec_codec_fini(void);
struct obj_ec_codec *obj_ec_codec_get(daos_oclass_id_t oc_id);

static inline struct obj_ec_codec *
obj_id2ec_codec(daos_obj_id_t id)
{
	return obj_ec_codec_get(daos_obj_id2class(id));
}

/* check if list_1 is fully covered by list_2 */
static inline bool
obj_ec_parity_list_covered(struct daos_recx_ep_list *list_1, struct daos_recx_ep_list *list_2)
{
	struct daos_recx_ep	*rep_1;
	struct daos_recx_ep	*rep_2;
	unsigned int		 i, j;

	if (list_1->re_nr == 0 || list_2->re_nr == 0)
		return false;

	for (i = 0; i < list_1->re_nr; i++) {
		rep_1 = &list_1->re_items[i];
		for (j = 0; j < list_2->re_nr; j++) {
			rep_2 = &list_2->re_items[j];
			if (rep_1->re_rec_size != rep_2->re_rec_size) {
				D_ERROR("mismatch rec_size %d:%d\n",
					rep_1->re_rec_size, rep_2->re_rec_size);
				return false;
			}
			if (DAOS_RECX_COVERED(rep_1->re_recx, rep_2->re_recx))
				break;
			if (j == list_2->re_nr - 1) {
				D_ERROR("not fully covered recx list\n");
				return false;
			}
		}
	}
	return true;
}

static inline int
obj_ec_parity_lists_match(struct daos_recx_ep_list *lists_1,
			  struct daos_recx_ep_list *lists_2,
			  unsigned int nr)
{
	struct daos_recx_ep_list	*list_1, *list_2;
	unsigned int			 i, j;

	for (i = 0; i < nr; i++) {
		list_1 = &lists_1[i];
		list_2 = &lists_2[i];
		if (list_1->re_ep_valid != list_2->re_ep_valid) {
			D_ERROR("got different ep_valid in EC data recovery\n");
			return -DER_IO;
		}
		if (list_1->re_nr != list_2->re_nr) {
			if (obj_ec_parity_list_covered(list_1, list_2) ||
			    obj_ec_parity_list_covered(list_2, list_1)) {
				D_DEBUG(DB_IO, "parity list mismatch but fully covered\n");
				return -DER_FETCH_AGAIN;
			}
			D_ERROR("got different parity recx in EC data recovery\n");
			return -DER_DATA_LOSS;
		}
		if (list_1->re_nr == 0)
			continue;
		for (j = 0; j < list_1->re_nr; j++) {
			if ((list_1->re_items[j].re_recx.rx_idx !=
			     list_2->re_items[j].re_recx.rx_idx) ||
			    (list_1->re_items[j].re_recx.rx_nr !=
			     list_2->re_items[j].re_recx.rx_nr)) {
				D_ERROR("got different parity recx in EC data recovery\n");
				return -DER_DATA_LOSS;
			}
			if (list_1->re_items[j].re_ep != list_2->re_items[j].re_ep)
				return -DER_FETCH_AGAIN;
		}
	}

	return 0;
}

struct dc_object;
/* cli_ec.c */
int obj_ec_req_reasb(struct dc_object *obj, daos_iod_t *iods, uint64_t dkey_hash, d_sg_list_t *sgls,
		     struct obj_reasb_req *reasb_req, uint32_t iod_nr, bool update);
void obj_ec_recxs_fini(struct obj_ec_recx_array *recxs);
void obj_ec_seg_sorter_fini(struct obj_ec_seg_sorter *sorter);
void obj_ec_tgt_oiod_fini(struct obj_tgt_oiod *tgt_oiods);
struct obj_tgt_oiod *obj_ec_tgt_oiod_init(struct obj_io_desc *r_oiods,
			uint32_t iod_nr, uint8_t *tgt_bitmap,
			uint32_t tgt_max_idx, uint32_t tgt_nr, uint32_t start_tgt,
			struct daos_oclass_attr *oca);
struct obj_tgt_oiod *obj_ec_tgt_oiod_get(struct obj_tgt_oiod *tgt_oiods,
			uint32_t tgt_nr, uint32_t tgt_idx);
void obj_ec_fetch_set_sgl(struct dc_object *obj, struct obj_reasb_req *reasb_req,
			  uint64_t dkey_hash, uint32_t iod_nr);
void obj_ec_update_iod_size(struct obj_reasb_req *reasb_req, uint32_t iod_nr);
int obj_ec_recov_add(struct obj_reasb_req *reasb_req,
		     struct daos_recx_ep_list *recx_lists, unsigned int nr);
int obj_ec_parity_check(struct obj_reasb_req *reasb_req,
			struct daos_recx_ep_list *recx_lists, unsigned int nr);
struct obj_ec_fail_info *obj_ec_fail_info_get(struct obj_reasb_req *reasb_req,
					      bool create, uint16_t p);

int obj_ec_fail_info_parity_get(struct dc_object *obj, struct obj_reasb_req *reasb_req,
				uint64_t dkey_hash, uint32_t *parity_tgt_idx, uint8_t *cur_bitmap);
int obj_ec_fail_info_insert(struct obj_reasb_req *reasb_req, uint16_t fail_tgt);
void obj_ec_fail_info_reset(struct obj_reasb_req *reasb_req);
void obj_ec_fail_info_free(struct obj_reasb_req *reasb_req);
int obj_ec_recov_prep(struct dc_object *obj, struct obj_reasb_req *reasb_req,
		      uint64_t dkey_hash, daos_iod_t *iods, uint32_t iod_nr);
void obj_ec_recov_data(struct obj_reasb_req *reasb_req, uint32_t iod_nr);

#endif /* __OBJ_EC_H__ */
