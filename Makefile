
ICONTEXT = PalmKey
APPID = Pkey
VERSION = 0.9.1

INCLUDES = -I..

CC = m68k-palmos-gcc
OBJRES = m68k-palmos-obj-res
BUILDPRC = build-prc
PILRC = pilrc
SED = sed

CFLAGS = -O0 -g -Wall -mno-own-gp -D__DEFINE_TYPES_ $(DEFINES) $(INCLUDES)

PRC = $(ICONTEXT)-$(VERSION).prc

OBJS = palmkey.o dict.o util.o md4/md4c.o md5/md5c.o sha1/sha1.o

all: $(PRC)

$(PRC): code.stamp bin.stamp
	$(BUILDPRC) $@ $(ICONTEXT) $(APPID) *.grc *.bin

code.stamp: palmkey
	$(OBJRES) palmkey
	touch code.stamp

bin.stamp: palmkey_rcp.h palmkey.rcp.in
	$(SED) -e 's/__VERSION__/$(VERSION)/g' palmkey.rcp.in > palmkey.rcp
	$(PILRC) palmkey.rcp
	touch bin.stamp

palmkey: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

md4/md4c.o: types.h md4/md4.h md4/md4c.c
	(cd md4; $(CC) $(CFLAGS) -c md4c.c)

md5/md5c.o: types.h md5/md5.h md5/md5c.c
	(cd md5; $(CC) $(CFLAGS) -c md5c.c)

sha1/sha1.o: types.h sha1/sha1.h sha1/sha1.c
	(cd sha1; $(CC) $(CFLAGS) -c sha1.c)

clean:
	rm -f *.[oa] md[45]/*.o sha1/sha1.o palmkey palmkey.rcp *.bin *.stamp *.[pg]rc
