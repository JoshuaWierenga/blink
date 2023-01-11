#-*-mode:makefile-gmake;indent-tabs-mode:t;tab-width:8;coding:utf-8-*-┐
#───vi: set et ft=make ts=8 tw=8 fenc=utf-8 :vi───────────────────────┘

third_party/gnulib_build/config.log:
	@if [ "$(MODE)" = "mingw" ]; then \
	    cd third_party/gnulib_build || exit 1; \
	    ../gnulib/gnulib-tool --import; \
	    autoreconf -i; \
	    ./configure; \
	    $(MAKE) -C lib; \
	fi