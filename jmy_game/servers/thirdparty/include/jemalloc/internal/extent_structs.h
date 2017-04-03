#ifndef JEMALLOC_INTERNAL_EXTENT_STRUCTS_H
#define JEMALLOC_INTERNAL_EXTENT_STRUCTS_H

typedef enum {
	extent_state_active   = 0,
	extent_state_dirty    = 1,
	extent_state_muzzy    = 2,
	extent_state_retained = 3
} extent_state_t;

/* Extent (span of pages).  Use accessor functions for e_* fields. */
struct extent_s {
	/*
	 * Bitfield containing several fields:
	 *
	 * a: arena_ind
	 * b: slab
	 * c: committed
	 * z: zeroed
	 * t: state
	 * i: szind
	 * n: sn
	 * f: nfree
	 *
	 * nnnnnnnn ... nnnnnfff fffffffi iiiiiiit tzcbaaaa aaaaaaaa
	 *
	 * arena_ind: Arena from which this extent came, or all 1 bits if
	 *            unassociated.
	 *
	 * slab: The slab flag indicates whether the extent is used for a slab
	 *       of small regions.  This helps differentiate small size classes,
	 *       and it indicates whether interior pointers can be looked up via
	 *       iealloc().
	 *
	 * committed: The committed flag indicates whether physical memory is
	 *            committed to the extent, whether explicitly or implicitly
	 *            as on a system that overcommits and satisfies physical
	 *            memory needs on demand via soft page faults.
	 *
	 * zeroed: The zeroed flag is used by extent recycling code to track
	 *         whether memory is zero-filled.
	 *
	 * state: The state flag is an extent_state_t.
	 *
	 * szind: The szind flag indicates usable size class index for
	 *        allocations residing in this extent, regardless of whether the
	 *        extent is a slab.  Extent size and usable size often differ
	 *        even for non-slabs, either due to large_pad or promotion of
	 *        sampled small regions.
	 *
	 * nfree: Number of free regions in slab.
	 *
	 * sn: Serial number (potentially non-unique).
	 *
	 *     Serial numbers may wrap around if JEMALLOC_MUNMAP is defined, but
	 *     as long as comparison functions fall back on address comparison
	 *     for equal serial numbers, stable (if imperfect) ordering is
	 *     maintained.
	 *
	 *     Serial numbers may not be unique even in the absence of
	 *     wrap-around, e.g. when splitting an extent and assigning the same
	 *     serial number to both resulting adjacent extents.
	 */
	uint64_t		e_bits;
#define EXTENT_BITS_ARENA_SHIFT		0
#define EXTENT_BITS_ARENA_MASK \
    (((uint64_t)(1U << MALLOCX_ARENA_BITS) - 1) << EXTENT_BITS_ARENA_SHIFT)

#define EXTENT_BITS_SLAB_SHIFT		MALLOCX_ARENA_BITS
#define EXTENT_BITS_SLAB_MASK \
    ((uint64_t)0x1U << EXTENT_BITS_SLAB_SHIFT)

#define EXTENT_BITS_COMMITTED_SHIFT	(MALLOCX_ARENA_BITS + 1)
#define EXTENT_BITS_COMMITTED_MASK \
    ((uint64_t)0x1U << EXTENT_BITS_COMMITTED_SHIFT)

#define EXTENT_BITS_ZEROED_SHIFT	(MALLOCX_ARENA_BITS + 2)
#define EXTENT_BITS_ZEROED_MASK \
    ((uint64_t)0x1U << EXTENT_BITS_ZEROED_SHIFT)

#define EXTENT_BITS_STATE_SHIFT		(MALLOCX_ARENA_BITS + 3)
#define EXTENT_BITS_STATE_MASK \
    ((uint64_t)0x3U << EXTENT_BITS_STATE_SHIFT)

#define EXTENT_BITS_SZIND_SHIFT		(MALLOCX_ARENA_BITS + 5)
#define EXTENT_BITS_SZIND_MASK \
    (((uint64_t)(1U << LG_CEIL_NSIZES) - 1) << EXTENT_BITS_SZIND_SHIFT)

#define EXTENT_BITS_NFREE_SHIFT \
    (MALLOCX_ARENA_BITS + 5 + LG_CEIL_NSIZES)
#define EXTENT_BITS_NFREE_MASK \
    ((uint64_t)((1U << (LG_SLAB_MAXREGS + 1)) - 1) << EXTENT_BITS_NFREE_SHIFT)

#define EXTENT_BITS_SN_SHIFT \
    (MALLOCX_ARENA_BITS + 5 + LG_CEIL_NSIZES + (LG_SLAB_MAXREGS + 1))
#define EXTENT_BITS_SN_MASK		(UINT64_MAX << EXTENT_BITS_SN_SHIFT)

	/* Pointer to the extent that this structure is responsible for. */
	void			*e_addr;

	/* Extent size. */
	size_t			e_size;

	/*
	 * List linkage, used by a variety of lists:
	 * - arena_bin_t's slabs_full
	 * - extents_t's LRU
	 * - stashed dirty extents
	 * - arena's large allocations
	 * - arena's extent structure freelist
	 */
	ql_elm(extent_t)	ql_link;

	/* Linkage for per size class sn/address-ordered heaps. */
	phn(extent_t)		ph_link;

	union {
		/* Small region slab metadata. */
		arena_slab_data_t	e_slab_data;

		/* Profile counters, used for large objects. */
		union {
			void		*e_prof_tctx_pun;
			prof_tctx_t	*e_prof_tctx;
		};
	};
};
typedef ql_head(extent_t) extent_list_t;
typedef ph(extent_t) extent_heap_t;

/* Quantized collection of extents, with built-in LRU queue. */
struct extents_s {
	malloc_mutex_t		mtx;

	/*
	 * Quantized per size class heaps of extents.
	 *
	 * Synchronization: mtx.
	 */
	extent_heap_t		heaps[NPSIZES+1];

	/*
	 * Bitmap for which set bits correspond to non-empty heaps.
	 *
	 * Synchronization: mtx.
	 */
	bitmap_t		bitmap[BITMAP_GROUPS(NPSIZES+1)];

	/*
	 * LRU of all extents in heaps.
	 *
	 * Synchronization: mtx.
	 */
	extent_list_t		lru;

	/*
	 * Page sum for all extents in heaps.
	 *
	 * The synchronization here is a little tricky.  Modifications to npages
	 * must hold mtx, but reads need not (though, a reader who sees npages
	 * without holding the mutex can't assume anything about the rest of the
	 * state of the extents_t).
	 */
	atomic_zu_t		npages;

	/* All stored extents must be in the same state. */
	extent_state_t		state;

	/*
	 * If true, delay coalescing until eviction; otherwise coalesce during
	 * deallocation.
	 */
	bool			delay_coalesce;
};

#endif /* JEMALLOC_INTERNAL_EXTENT_STRUCTS_H */
