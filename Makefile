all: libsock.a libplugin.a include

libsock.a:
	cd src/libsock; \
	make; \
	if [ ! -e ../../lib/libsock.a ]; then ln -s `readlink -f libsock.a` ../../lib/; fi

libplugin.a:
	cd src/libplugin; \
	make; \
    if [ ! -e ../../lib/libplugin.a ]; then ln -s `readlink -f libplugin.a` ../../lib/; fi

include: src/*/include
	for d in src/*; do dd=$(basename "$d"); if [ ! -e include/${dd/lib//} ]; then ln -s $d/include include/${dd/lib//}; fi; done
