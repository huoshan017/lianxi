#undef a0dalloc
#undef a0malloc
#undef arena_aalloc
#undef arena_alloc_junk_small
#undef arena_basic_stats_merge
#undef arena_bin_index
#undef arena_bin_info
#undef arena_boot
#undef arena_choose
#undef arena_choose_hard
#undef arena_choose_impl
#undef arena_cleanup
#undef arena_dalloc
#undef arena_dalloc_bin_junked_locked
#undef arena_dalloc_junk_small
#undef arena_dalloc_no_tcache
#undef arena_dalloc_promoted
#undef arena_dalloc_small
#undef arena_decay
#undef arena_decay_tick
#undef arena_decay_ticks
#undef arena_dirty_decay_time_default_get
#undef arena_dirty_decay_time_default_set
#undef arena_dirty_decay_time_get
#undef arena_dirty_decay_time_set
#undef arena_muzzy_decay_time_default_get
#undef arena_muzzy_decay_time_default_set
#undef arena_muzzy_decay_time_get
#undef arena_muzzy_decay_time_set
#undef arena_destroy
#undef arena_dss_prec_get
#undef arena_dss_prec_set
#undef arena_extent_alloc_large
#undef arena_extent_dalloc_large_prep
#undef arena_extent_ralloc_large_expand
#undef arena_extent_ralloc_large_shrink
#undef arena_extent_sn_next
#undef arena_extents_dirty_dalloc
#undef arena_get
#undef arena_ichoose
#undef arena_ind_get
#undef arena_init
#undef arena_internal_add
#undef arena_internal_get
#undef arena_internal_sub
#undef arena_malloc
#undef arena_malloc_hard
#undef arena_migrate
#undef arena_new
#undef arena_nthreads_dec
#undef arena_nthreads_get
#undef arena_nthreads_inc
#undef arena_palloc
#undef arena_postfork_child
#undef arena_postfork_parent
#undef arena_prefork0
#undef arena_prefork1
#undef arena_prefork2
#undef arena_prefork3
#undef arena_prefork4
#undef arena_prefork5
#undef arena_prefork6
#undef arena_prof_accum
#undef arena_prof_promote
#undef arena_prof_tctx_get
#undef arena_prof_tctx_reset
#undef arena_prof_tctx_set
#undef arena_ralloc
#undef arena_ralloc_no_move
#undef arena_reset
#undef arena_salloc
#undef arena_sdalloc
#undef arena_sdalloc_no_tcache
#undef arena_set
#undef arena_slab_regind
#undef arena_stats_init
#undef arena_stats_mapped_add
#undef arena_stats_merge
#undef arena_tcache_fill_small
#undef arena_tdata_get
#undef arena_tdata_get_hard
#undef arena_vsalloc
#undef arenas
#undef arenas_tdata_cleanup
#undef b0get
#undef base_alloc
#undef base_boot
#undef base_delete
#undef base_extent_hooks_get
#undef base_extent_hooks_set
#undef base_ind_get
#undef base_new
#undef base_postfork_child
#undef base_postfork_parent
#undef base_prefork
#undef base_stats_get
#undef bitmap_ffu
#undef bitmap_full
#undef bitmap_get
#undef bitmap_info_init
#undef bitmap_init
#undef bitmap_set
#undef bitmap_sfu
#undef bitmap_size
#undef bitmap_unset
#undef bootstrap_calloc
#undef bootstrap_free
#undef bootstrap_malloc
#undef bt_init
#undef bt2gctx_mtx
#undef buferror
#undef ckh_count
#undef ckh_delete
#undef ckh_insert
#undef ckh_iter
#undef ckh_new
#undef ckh_pointer_hash
#undef ckh_pointer_keycomp
#undef ckh_remove
#undef ckh_search
#undef ckh_string_hash
#undef ckh_string_keycomp
#undef ctl_boot
#undef ctl_bymib
#undef ctl_byname
#undef ctl_nametomib
#undef ctl_postfork_child
#undef ctl_postfork_parent
#undef ctl_prefork
#undef decay_ticker_get
#undef dss_prec_names
#undef extent_ad_comp
#undef extent_addr_get
#undef extent_addr_randomize
#undef extent_addr_set
#undef extent_alloc
#undef extent_alloc_cache
#undef extent_alloc_dss
#undef extent_alloc_mmap
#undef extent_alloc_wrapper
#undef extent_arena_get
#undef extent_arena_set
#undef extent_base_get
#undef extent_before_get
#undef extent_boot
#undef extent_commit_wrapper
#undef extent_committed_get
#undef extent_committed_set
#undef extent_dalloc
#undef extent_dalloc_gap
#undef extent_dalloc_mmap
#undef extent_dalloc_wrapper
#undef extent_dalloc_wrapper_try
#undef extent_decommit_wrapper
#undef extent_dss_boot
#undef extent_dss_mergeable
#undef extent_dss_prec_get
#undef extent_dss_prec_set
#undef extent_heap_empty
#undef extent_heap_first
#undef extent_heap_insert
#undef extent_heap_new
#undef extent_heap_remove
#undef extent_heap_remove_first
#undef extent_hooks_default
#undef extent_hooks_get
#undef extent_hooks_set
#undef extent_in_dss
#undef extent_init
#undef extent_last_get
#undef extent_list_append
#undef extent_list_first
#undef extent_list_init
#undef extent_list_last
#undef extent_list_remove
#undef extent_list_replace
#undef extent_merge_wrapper
#undef extent_nfree_dec
#undef extent_nfree_get
#undef extent_nfree_inc
#undef extent_nfree_set
#undef extent_past_get
#undef extent_prof_tctx_get
#undef extent_prof_tctx_set
#undef extent_purge_forced_wrapper
#undef extent_purge_lazy_wrapper
#undef extent_size_get
#undef extent_size_quantize_ceil
#undef extent_size_quantize_floor
#undef extent_size_set
#undef extent_slab_data_get
#undef extent_slab_data_get_const
#undef extent_slab_get
#undef extent_slab_set
#undef extent_sn_comp
#undef extent_sn_get
#undef extent_sn_set
#undef extent_snad_comp
#undef extent_split_wrapper
#undef extent_state_get
#undef extent_state_set
#undef extent_szind_get
#undef extent_szind_get_maybe_invalid
#undef extent_szind_set
#undef extent_usize_get
#undef extent_zeroed_get
#undef extent_zeroed_set
#undef extents_alloc
#undef extents_dalloc
#undef extents_evict
#undef extents_init
#undef extents_npages_get
#undef extents_prefork
#undef extents_postfork_child
#undef extents_postfork_parent
#undef extents_rtree
#undef extents_state_get
#undef ffs_llu
#undef ffs_lu
#undef ffs_u
#undef ffs_u32
#undef ffs_u64
#undef ffs_zu
#undef get_errno
#undef hash
#undef hash_fmix_32
#undef hash_fmix_64
#undef hash_get_block_32
#undef hash_get_block_64
#undef hash_rotl_32
#undef hash_rotl_64
#undef hash_x64_128
#undef hash_x86_128
#undef hash_x86_32
#undef iaalloc
#undef ialloc
#undef iallocztm
#undef iarena_cleanup
#undef idalloc
#undef idalloctm
#undef iealloc
#undef index2size
#undef index2size_compute
#undef index2size_lookup
#undef index2size_tab
#undef ipalloc
#undef ipalloct
#undef ipallocztm
#undef iralloc
#undef iralloct
#undef iralloct_realign
#undef isalloc
#undef isdalloct
#undef isthreaded
#undef ivsalloc
#undef ixalloc
#undef jemalloc_postfork_child
#undef jemalloc_postfork_parent
#undef jemalloc_prefork
#undef large_dalloc
#undef large_dalloc_finish
#undef large_dalloc_junk
#undef large_dalloc_maybe_junk
#undef large_dalloc_prep_junked_locked
#undef large_malloc
#undef large_palloc
#undef large_prof_tctx_get
#undef large_prof_tctx_reset
#undef large_prof_tctx_set
#undef large_ralloc
#undef large_ralloc_no_move
#undef large_salloc
#undef lg_floor
#undef lg_prof_sample
#undef malloc_cprintf
#undef malloc_getcpu
#undef malloc_mutex_prof_data_reset
#undef malloc_mutex_assert_not_owner
#undef malloc_mutex_assert_owner
#undef malloc_mutex_boot
#undef malloc_mutex_init
#undef malloc_mutex_lock
#undef malloc_mutex_lock_slow
#undef malloc_mutex_postfork_child
#undef malloc_mutex_postfork_parent
#undef malloc_mutex_prefork
#undef malloc_mutex_unlock
#undef malloc_printf
#undef malloc_snprintf
#undef malloc_strtoumax
#undef malloc_tsd_boot0
#undef malloc_tsd_boot1
#undef malloc_tsd_cleanup_register
#undef malloc_tsd_dalloc
#undef malloc_tsd_malloc
#undef malloc_tsd_no_cleanup
#undef malloc_vcprintf
#undef malloc_vsnprintf
#undef malloc_write
#undef narenas_auto
#undef narenas_total_get
#undef ncpus
#undef nhbins
#undef nstime_add
#undef nstime_compare
#undef nstime_copy
#undef nstime_divide
#undef nstime_idivide
#undef nstime_imultiply
#undef nstime_init
#undef nstime_init2
#undef nstime_monotonic
#undef nstime_msec
#undef nstime_ns
#undef nstime_nsec
#undef nstime_sec
#undef nstime_subtract
#undef nstime_update
#undef opt_abort
#undef opt_dirty_decay_time
#undef opt_muzzy_decay_time
#undef opt_dss
#undef opt_junk
#undef opt_junk_alloc
#undef opt_junk_free
#undef opt_lg_prof_interval
#undef opt_lg_prof_sample
#undef opt_lg_tcache_max
#undef opt_narenas
#undef opt_prof
#undef opt_prof_accum
#undef opt_prof_active
#undef opt_prof_final
#undef opt_prof_gdump
#undef opt_prof_leak
#undef opt_prof_prefix
#undef opt_prof_thread_active_init
#undef opt_stats_print
#undef opt_tcache
#undef opt_utrace
#undef opt_xmalloc
#undef opt_zero
#undef pages_boot
#undef pages_commit
#undef pages_decommit
#undef pages_huge
#undef pages_map
#undef pages_nohuge
#undef pages_purge_forced
#undef pages_purge_lazy
#undef pages_trim
#undef pages_unmap
#undef percpu_arena_choose
#undef percpu_arena_ind_limit
#undef percpu_arena_update
#undef pind2sz
#undef pind2sz_compute
#undef pind2sz_lookup
#undef pind2sz_tab
#undef pow2_ceil_u32
#undef pow2_ceil_u64
#undef pow2_ceil_zu
#undef prng_lg_range_u32
#undef prng_lg_range_u64
#undef prng_lg_range_zu
#undef prng_range_u32
#undef prng_range_u64
#undef prng_range_zu
#undef prng_state_next_u32
#undef prng_state_next_u64
#undef prng_state_next_zu
#undef prof_accum_add
#undef prof_accum_cancel
#undef prof_accum_init
#undef prof_active
#undef prof_active_get
#undef prof_active_get_unlocked
#undef prof_active_set
#undef prof_alloc_prep
#undef prof_alloc_rollback
#undef prof_backtrace
#undef prof_boot0
#undef prof_boot1
#undef prof_boot2
#undef prof_bt_count
#undef prof_cnt_all
#undef prof_dump_header
#undef prof_dump_open
#undef prof_free
#undef prof_free_sampled_object
#undef prof_gdump
#undef prof_gdump_get
#undef prof_gdump_get_unlocked
#undef prof_gdump_set
#undef prof_gdump_val
#undef prof_idump
#undef prof_interval
#undef prof_lookup
#undef prof_malloc
#undef prof_malloc_sample_object
#undef prof_mdump
#undef prof_postfork_child
#undef prof_postfork_parent
#undef prof_prefork0
#undef prof_prefork1
#undef prof_realloc
#undef prof_reset
#undef prof_sample_accum_update
#undef prof_sample_threshold_update
#undef prof_tctx_get
#undef prof_tctx_reset
#undef prof_tctx_set
#undef prof_tdata_cleanup
#undef prof_tdata_count
#undef prof_tdata_get
#undef prof_tdata_init
#undef prof_tdata_reinit
#undef prof_thread_active_get
#undef prof_thread_active_init_get
#undef prof_thread_active_init_set
#undef prof_thread_active_set
#undef prof_thread_name_get
#undef prof_thread_name_set
#undef psz2ind
#undef psz2u
#undef rtree_clear
#undef rtree_delete
#undef rtree_extent_read
#undef rtree_extent_szind_read
#undef rtree_leaf_alloc
#undef rtree_leaf_dalloc
#undef rtree_leaf_elm_acquire
#undef rtree_leaf_elm_bits_extent_get
#undef rtree_leaf_elm_bits_locked_get
#undef rtree_leaf_elm_bits_read
#undef rtree_leaf_elm_bits_slab_get
#undef rtree_leaf_elm_bits_szind_get
#undef rtree_leaf_elm_extent_read
#undef rtree_leaf_elm_extent_write
#undef rtree_leaf_elm_lookup
#undef rtree_leaf_elm_lookup_hard
#undef rtree_leaf_elm_release
#undef rtree_leaf_elm_slab_read
#undef rtree_leaf_elm_slab_write
#undef rtree_leaf_elm_szind_read
#undef rtree_leaf_elm_szind_slab_update
#undef rtree_leaf_elm_szind_write
#undef rtree_leaf_elm_witness_access
#undef rtree_leaf_elm_witness_acquire
#undef rtree_leaf_elm_witness_release
#undef rtree_leaf_elm_write
#undef rtree_leafkey
#undef rtree_new
#undef rtree_node_alloc
#undef rtree_node_dalloc
#undef rtree_read
#undef rtree_subkey
#undef rtree_szind_read
#undef rtree_szind_slab_read
#undef rtree_szind_slab_update
#undef rtree_write
#undef s2u
#undef s2u_compute
#undef s2u_lookup
#undef sa2u
#undef set_errno
#undef size2index
#undef size2index_compute
#undef size2index_lookup
#undef size2index_tab
#undef spin_adaptive
#undef stats_print
#undef tcache_alloc_easy
#undef tcache_alloc_large
#undef tcache_alloc_small
#undef tcache_alloc_small_hard
#undef tcache_arena_reassociate
#undef tcache_bin_flush_large
#undef tcache_bin_flush_small
#undef tcache_bin_info
#undef tcache_boot
#undef tcache_cleanup
#undef tcache_create
#undef tcache_dalloc_large
#undef tcache_dalloc_small
#undef tcache_enabled_get
#undef tcache_enabled_set
#undef tcache_event
#undef tcache_event_hard
#undef tcache_flush
#undef tcache_get
#undef tcache_get_hard
#undef tcache_maxclass
#undef tcache_prefork
#undef tcache_postfork_child
#undef tcache_postfork_parent
#undef tcache_salloc
#undef tcache_stats_merge
#undef tcaches
#undef tcaches_create
#undef tcaches_destroy
#undef tcaches_flush
#undef tcaches_get
#undef ticker_copy
#undef ticker_init
#undef ticker_read
#undef ticker_tick
#undef ticker_ticks
#undef tsd_arena_get
#undef tsd_arena_set
#undef tsd_arenap_get
#undef tsd_arenas_tdata_bypassp_get
#undef tsd_arenas_tdata_get
#undef tsd_arenas_tdata_set
#undef tsd_arenas_tdatap_get
#undef tsd_boot
#undef tsd_boot0
#undef tsd_boot1
#undef tsd_booted
#undef tsd_booted_get
#undef tsd_cleanup
#undef tsd_cleanup_wrapper
#undef tsd_fetch
#undef tsd_fetch_impl
#undef tsd_get
#undef tsd_get_allocates
#undef tsd_iarena_get
#undef tsd_iarena_set
#undef tsd_iarenap_get
#undef tsd_initialized
#undef tsd_init_check_recursion
#undef tsd_init_finish
#undef tsd_init_head
#undef tsd_narenas_tdata_get
#undef tsd_narenas_tdata_set
#undef tsd_narenas_tdatap_get
#undef tsd_wrapper_get
#undef tsd_wrapper_set
#undef tsd_nominal
#undef tsd_prof_tdata_get
#undef tsd_prof_tdata_set
#undef tsd_prof_tdatap_get
#undef tsd_rtree_ctx
#undef tsd_rtree_ctxp_get
#undef tsd_rtree_leaf_elm_witnessesp_get
#undef tsd_set
#undef tsd_tcache_enabled_get
#undef tsd_tcache_enabled_set
#undef tsd_tcache_enabledp_get
#undef tsd_tcache_get
#undef tsd_tcache_set
#undef tsd_tcachep_get
#undef tsd_thread_allocated_get
#undef tsd_thread_allocated_set
#undef tsd_thread_allocatedp_get
#undef tsd_thread_deallocated_get
#undef tsd_thread_deallocated_set
#undef tsd_thread_deallocatedp_get
#undef tsd_tls
#undef tsd_tsd
#undef tsd_tsdn
#undef tsd_witness_fork_get
#undef tsd_witness_fork_set
#undef tsd_witness_forkp_get
#undef tsd_witnessesp_get
#undef tsdn_fetch
#undef tsdn_null
#undef tsdn_rtree_ctx
#undef tsdn_tsd
#undef witness_assert_depth
#undef witness_assert_depth_to_rank
#undef witness_assert_lockless
#undef witness_assert_not_owner
#undef witness_assert_owner
#undef witness_depth_error
#undef witness_init
#undef witness_lock
#undef witness_lock_error
#undef witness_not_owner_error
#undef witness_owner
#undef witness_owner_error
#undef witness_postfork_child
#undef witness_postfork_parent
#undef witness_prefork
#undef witness_unlock
#undef witnesses_cleanup
#undef zone_register