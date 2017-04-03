#ifndef JEMALLOC_INTERNAL_RTREE_STRUCTS_H
#define JEMALLOC_INTERNAL_RTREE_STRUCTS_H

struct rtree_node_elm_s {
	atomic_p_t	child; /* (rtree_{node,leaf}_elm_t *) */
};

struct rtree_leaf_elm_s {
#ifdef RTREE_LEAF_COMPACT
	/*
	 * Single pointer-width field containing all three leaf element fields.
	 * For example, on a 64-bit x64 system with 48 significant virtual
	 * memory address bits, the index, extent, and slab fields are packed as
	 * such:
	 *
	 * x: index
	 * e: extent
	 * b: slab
	 * k: lock
	 *
	 *   00000000 xxxxxxxx eeeeeeee [...] eeeeeeee eeee00bk
	 */
	atomic_p_t	le_bits;
#else
	atomic_p_t	le_extent; /* (extent_t *), lock in low bit */
	atomic_u_t	le_szind; /* (szind_t) */
	atomic_b_t	le_slab; /* (bool) */
#endif
};

struct rtree_leaf_elm_witness_s {
	const rtree_leaf_elm_t	*elm;
	witness_t		witness;
};

struct rtree_leaf_elm_witness_tsd_s {
	rtree_leaf_elm_witness_t	witnesses[RTREE_ELM_ACQUIRE_MAX];
};

struct rtree_level_s {
	/* Number of key bits distinguished by this level. */
	unsigned		bits;
	/*
	 * Cumulative number of key bits distinguished by traversing to
	 * corresponding tree level.
	 */
	unsigned		cumbits;
};

struct rtree_ctx_cache_elm_s {
	uintptr_t		leafkey;
	rtree_leaf_elm_t	*leaf;
};

struct rtree_ctx_s {
#ifndef _MSC_VER
	JEMALLOC_ALIGNED(CACHELINE)
#endif
	rtree_ctx_cache_elm_t	cache[RTREE_CTX_NCACHE];
};

struct rtree_s {
	malloc_mutex_t		init_lock;
	/* Number of elements based on rtree_levels[0].bits. */
#if RTREE_HEIGHT > 1
	rtree_node_elm_t	root[1U << (RTREE_NSB/RTREE_HEIGHT)];
#else
	rtree_leaf_elm_t	root[1U << (RTREE_NSB/RTREE_HEIGHT)];
#endif
};

#endif /* JEMALLOC_INTERNAL_RTREE_STRUCTS_H */
