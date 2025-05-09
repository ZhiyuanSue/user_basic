include $(SCRIPT_MAKE_DIR)/build.mk
-include ./Makefile.env

all: gen_link ./link_app.o

gen_link:
	@python3 link_app.py user/build/${ARCH}/

-include ${BUILD}/*.d
./link_app.o: ./link_app.S
	@echo "CC	"$@
	@$(CC) $(CFLAGS) -o $@ -c $<