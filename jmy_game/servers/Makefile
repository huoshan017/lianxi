dirs := ../proto libjmy login_server gate_server game_server

define make_subdir
	@for subdir in $(dirs); do \
		(cd $$subdir && make $1) \
	done;
endef
all:
	$(call make_subdir, all)
clean:
	$(call make_subdir, clean)
