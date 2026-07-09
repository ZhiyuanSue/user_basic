include $(SCRIPT_MAKE_DIR)/build.mk
-include ./Makefile.env

# RendezvOS cpio mode: top-level `make user` sets RENDEZVOS_FILESYSTEM_MODE=1
# → stub link_app.o (_num_app=0); ELFs live in initramfs (see RENDEZVOS.md).
all: gen_link ./link_app.o

gen_link:
	@python3 link_app.py user/build/${ARCH}/

-include ${BUILD}/*.d
./link_app.o: ./link_app.S
	@echo "CC	"$@
	@$(CC) $(CFLAGS) -o $@ -c $<
