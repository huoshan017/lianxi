#ifndef JEMALLOC_INTERNAL_ARENA_INLINES_B_H
#define JEMALLOC_INTERNAL_ARENA_INLINES_B_H

#ifndef JEMALLOC_ENABLE_INLINE
szind_t arena_bin_index(arena_t *arena, arena_bin_t *bin);
prof_tctx_t *arena_prof_tctx_get(tsdn_t *tsdn, const void *ptr);
void arena_prof_tctx_set(tsdn_t *tsdn, const void *ptr, size_t usize,
    prof_tctx_t *tctx);
void arena_prof_tctx_reset(tsdn_t *tsdn, const void *ptr, prof_tctx_t *tctx);
void arena_decay_ticks(tsdn_t *tsdn, arena_t *arena, unsigned nticks);
void arena_decay_tick(tsdn_t *tsdn, arena_t *arena);
void *arena_malloc(tsdn_t *tsdn, arena_t *arena, size_t size, szind_t ind,
    bool zero, tcache_t *tcache, bool slow_path);
arena_t *arena_aalloc(tsdn_t *tsdn, const void *ptr);
size_t arena_salloc(tsdn_t *tsdn, const void *ptr);
size_t arena_vsalloc(tsdn_t *tsdn, const void *ptr);
void arena_dalloc_no_tcache(tsdn_t *tsdn, void *ptr);
void arena_dalloc(tsdn_t *tsdn, void *ptr, tcache_t *tcache, bool slow_path);
void arena_sdalloc_no_tcache(tsdn_t *tsdn, void *ptr, size_t size);
void arena_sdalloc(tsdn_t *tsdn, void *ptr, size_t size, tcache_t *tcache,
    bool slow_path);
#endif

#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_ARENA_C_))
JEMALLOC_INLINE szind_t
arena_bin_index(arena_t *arena, arena_bin_t *bin) {
	szind_t binind = (szind_t)(bin - arena->bins);
	assert(binind < NBINS);
	return binind;
}

JEMALLOC_INLINE prof_tctx_t *
arena_prof_tctx_get(tsdn_t *tsdn, const void *ptr) {
	cassert(config_prof);
	assert(ptr != NULL);

	const extent_t *extent = iealloc(tsdn, ptr);
	if (unlikely(!extent_slab_get(extent))) {
		return large_prof_tctx_get(tsdn, extent);
	}
	return (prof_tctx_t *)(uintptr_t)1U;
}

JEMALLOC_INLINE void
arena_prof_tctx_set(tsdn_t *tsdn, const void *ptr, size_t usize,
    prof_tctx_t *tctx) {
	cassert(config_prof);
	assert(ptr != NULL);

	extent_t *extent = iealloc(tsdn, ptr);
	if (unlikely(!extent_slab_get(extent))) {
		large_prof_tctx_set(tsdn, extent, tctx);
	}
}

JEMALLOC_INLINE void
arena_prof_tctx_reset(tsdn_t *tsdn, const void *ptr, prof_tctx_t *tctx) {
	cassert(config_prof);
	assert(ptr != NULL);

	extent_t *extent = iealloc(tsdn, ptr);
	assert(!extent_slab_get(extent));

	large_prof_tctx_reset(tsdn, extent);
}

JEMALLOC_ALWAYS_INLINE void
arena_decay_ticks(tsdn_t *tsdn, arena_t *arena, unsigned nticks) {
	tsd_t *tsd;
	ticker_t *decay_ticker;

	if (unlikely(tsdn_null(tsdn))) {
		return;
	}
	tsd = tsdn_tsd(tsdn);
	decay_ticker = decay_ticker_get(tsd, arena_ind_get(arena));
	if (unlikely(decay_ticker == NULL)) {
		return;
	}
	if (unlikely(ticker_ticks(decay_ticker, nticks))) {
		arena_decay(tsdn, arena, false);
	}
}

JEMALLOC_ALWAYS_INLINE void
arena_decay_tick(tsdn_t *tsdn, arena_t *arena) {
	malloc_mutex_assert_not_owner(tsdn, &arena->decay_dirty.mtx);
	malloc_mutex_assert_not_owner(tsdn, &arena->decay_muzzy.mtx);

	arena_decay_ticks(tsdn, arena, 1);
}

JEMALLOC_ALWAYS_INLINE void *
arena_malloc(tsdn_t *tsdn, arena_t *arena, size_t size, szind_t ind, bool zero,
    tcache_t *tcache, bool slow_path) {
	assert(!tsdn_null(tsdn) || tcache == NULL);
	assert(size != 0);

	if (likely(tcache != NULL)) {
		if (likely(size <= SMALL_MAXCLASS)) {
			return tcache_alloc_small(tsdn_tsd(tsdn), arena,
			    tcache, size, ind, zero, slow_path);
		}
		if (likely(size <= tcache_maxclass)) {
			return tcache_alloc_large(tsdn_tsd(tsdn), arena,
			    tcache, size, ind, zero, slow_path);
		}
		/* (size > tcache_maxclass) case falls through. */
		assert(size > tcache_maxclass);
	}

	return arena_malloc_hard(tsdn, arena, size, ind, zero);
}

JEMALLOC_ALWAYS_INLINE arena_t *
arena_aalloc(tsdn_t *tsdn, const void *ptr) {
	return extent_arena_get(iealloc(tsdn, ptr));
}

JEMALLOC_ALWAYS_INLINE size_t
arena_salloc(tsdn_t *tsdn, const void *ptr) {
	assert(ptr != NULL);

	rtree_ctx_t rtree_ctx_fallback;
	rtree_ctx_t *rtree_ctx = tsdn_rtree_ctx(tsdn, &rtree_ctx_fallback);

	szind_t szind = rtree_szind_read(tsdn, &extents_rtree, rtree_ctx,
	    (uintptr_t)ptr, true);
	assert(szind != NSIZES);

	return index2size(szind);
}

JEMALLOC_ALWAYS_INLINE size_t
arena_vsalloc(tsdn_t *tsdn, const void *ptr) {
	/*
	 * Return 0 if ptr is not within an extent managed by jemalloc.  This
	 * function has two extra costs relative to isalloc():
	 * - The rtree calls cannot claim to be dependent lookups, which induces
	 *   rtree lookup load dependencies.
	 * - The lookup may fail, so there is an extra branch to check for
	 *   failure.
	 */

	rtree_ctx_t rtree_ctx_fallback;
	rtree_ctx_t *rtree_ctx = tsdn_rtree_ctx(tsdn, &rtree_ctx_fallback);

	extent_t *extent;
	szind_t szind;
	if (rtree_extent_szind_read(tsdn, &extents_rtree, rtree_ctx,
	    (uintptr_t)ptr, false, &extent, &szind)) {
		return 0;
	}

	if (extent == NULL) {
		return 0;
	}
	assert(extent_state_get(extent) == extent_state_active);
	/* Only slab members should be looked up via interior pointers. */
	assert(extent_addr_get(extent) == ptr || extent_slab_get(extent));

	assert(szind != NSIZES);

	return index2size(szind);
}

JEMALLOC_INLINE void
arena_dalloc_no_tcache(tsdn_t *tsdn, void *ptr) {
	assert(ptr != NULL);

	rtree_ctx_t rtree_ctx_fallback;
	rtree_ctx_t *rtree_ctx = tsdn_rtree_ctx(tsdn, &rtree_ctx_fallback);

	szind_t szind;
	bool slab;
	rtree_szind_slab_read(tsdn, &extents_rtree, rtree_ctx, (uintptr_t)ptr,
	    true, &szind, &slab);

	if (config_debug) {
		extent_t *extent = rtree_extent_read(tsdn, &extents_rtree,
		    rtree_ctx, (uintptr_t)ptr, true);
		assert(szind == extent_szind_get(extent));
		assert(szind < NSIZES);
		assert(slab == extent_slab_get(extent));
	}

	if (likely(slab)) {
		/* Small allocation. */
		arena_dalloc_small(tsdn, ptr);
	} else {
		extent_t *extent = iealloc(tsdn, ptr);
		large_dalloc(tsdn, extent);
	}
}

JEMALLOC_ALWAYS_INLINE void
arena_dalloc(tsdn_t *tsdn, void *ptr, tcache_t *tcache, bool slow_path) {
	assert(!tsdn_null(tsdn) || tcache == NULL);
	assert(ptr != NULL);

	if (unlikely(tcache == NULL)) {
		arena_dalloc_no_tcache(tsdn, ptr);
		return;
	}

	rtree_ctx_t *rtree_ctx = tsd_rtree_ctx(tsdn_tsd(tsdn));
	szind_t szind;
	bool slab;
	rtree_szind_slab_read(tsdn, &extents_rtree, rtree_ctx, (uintptr_t)ptr,
	    true, &szind, &slab);

	if (config_debug) {
		extent_t *extent = rtree_extent_read(tsdn, &extents_rtree,
		    rtree_ctx, (uintptr_t)ptr, true);
		assert(szind == extent_szind_get(extent));
		assert(szind < NSIZES);
		assert(slab == extent_slab_get(extent));
	}

	if (likely(slab)) {
		/* Small allocation. */
		tcache_dalloc_small(tsdn_tsd(tsdn), tcache, ptr, szind,
		    slow_path);
	} else {
		if (szind < nhbins) {
			if (config_prof && unlikely(szind < NBINS)) {
				arena_dalloc_promoted(tsdn, ptr, tcache,
				    slow_path);
			} else {
				tcache_dalloc_large(tsdn_tsd(tsdn), tcache, ptr,
				    szind, slow_path);
			}
		} else {
			extent_t *extent = iealloc(tsdn, ptr);
			large_dalloc(tsdn, extent);
		}
	}
}

JEMALLOC_INLINE void
arena_sdalloc_no_tcache(tsdn_t *tsdn, void *ptr, size_t size) {
	assert(ptr != NULL);
	assert(size <= LARGE_MAXCLASS);

	szind_t szind;
	bool slab;
	if (!config_prof || !opt_prof) {
		/*
		 * There is no risk of being confused by a promoted sampled
		 * object, so base szind and slab on the given size.
		 */
		szind = size2index(size);
		slab = (szind < NBINS);
	}

	if ((config_prof && opt_prof) || config_debug) {
		rtree_ctx_t rtree_ctx_fallback;
		rtree_ctx_t *rtree_ctx = tsdn_rtree_ctx(tsdn,
		    &rtree_ctx_fallback);

		rtree_szind_slab_read(tsdn, &extents_rtree, rtree_ctx,
		    (uintptr_t)ptr, true, &szind, &slab);

		assert(szind == size2index(size));
		assert((config_prof && opt_prof) || slab == (szind < NBINS));

		if (config_debug) {
			extent_t *extent = rtree_extent_read(tsdn,
			    &extents_rtree, rtree_ctx, (uintptr_t)ptr, true);
			assert(szind == extent_szind_get(extent));
			assert(slab == extent_slab_get(extent));
		}
	}

	if (likely(slab)) {
		/* Small allocation. */
		arena_dalloc_small(tsdn, ptr);
	} else {
		extent_t *extent = iealloc(tsdn, ptr);
		large_dalloc(tsdn, extent);
	}
}

JEMALLOC_ALWAYS_INLINE void
arena_sdalloc(tsdn_t *tsdn, void *ptr, size_t size, tcache_t *tcache,
    bool slow_path) {
	assert(!tsdn_null(tsdn) || tcache == NULL);
	assert(ptr != NULL);
	assert(size <= LARGE_MAXCLASS);

	if (unlikely(tcache == NULL)) {
		arena_sdalloc_no_tcache(tsdn, ptr, size);
		return;
	}

	szind_t szind;
	bool slab;
	if (!config_prof || !opt_prof) {
		/*
		 * There is no risk of being confused by a promoted sampled
		 * object, so base szind and slab on the given size.
		 */
		szind = size2index(size);
		slab = (szind < NBINS);
	}

	if ((config_prof && opt_prof) || config_debug) {
		rtree_ctx_t *rtree_ctx = tsd_rtree_ctx(tsdn_tsd(tsdn));

		rtree_szind_slab_read(tsdn, &extents_rtree, rtree_ctx,
		    (uintptr_t)ptr, true, &szind, &slab);

		assert(szind == size2index(size));
		assert((config_prof && opt_prof) || slab == (szind < NBINS));

		if (config_debug) {
			extent_t *extent = rtree_extent_read(tsdn,
			    &extents_rtree, rtree_ctx, (uintptr_t)ptr, true);
			assert(szind == extent_szind_get(extent));
			assert(slab == extent_slab_get(extent));
		}
	}

	if (likely(slab)) {
		/* Small allocation. */
		tcache_dalloc_small(tsdn_tsd(tsdn), tcache, ptr, szind,
		    slow_path);
	} else {
		if (szind < nhbins) {
			if (config_prof && unlikely(szind < NBINS)) {
				arena_dalloc_promoted(tsdn, ptr, tcache,
				    slow_path);
			} else {
				tcache_dalloc_large(tsdn_tsd(tsdn),
				    tcache, ptr, szind, slow_path);
			}
		} else {
			extent_t *extent = iealloc(tsdn, ptr);
			large_dalloc(tsdn, extent);
		}
	}
}

#endif /* (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_ARENA_C_)) */
#endif /* JEMALLOC_INTERNAL_ARENA_INLINES_B_H */
