dirs := ../proto libjmy common mysql login_server gate_server config_server game_server db_server test_client

define make_subdir
	@for subdir in $(dirs); do \
		(cd $$subdir && make $1) \
	done;
endef
all:
	$(call make_subdir, all)
clean:
	$(call make_subdir, clean)

proto:
	cd ../proto && make
libjmy:
	cd libjmy && make
common:
	cd common && make
mysql:
	cd mysql && make
ls:
	cd login_server && make
gt:
	cd gate_server && make
cs:
	cd config_server && make
gs:
	cd game_server && make
ds:
	cd db_server && make
test_client:
	cd test_client && make
