#-*-mode:makefile-gmake;indent-tabs-mode:t;tab-width:8;coding:utf-8-*-┐
#───vi: set et ft=make ts=8 tw=8 fenc=utf-8 :vi───────────────────────┘

MINGW-W64_TOOLCHAIN_HOST   := x86_64-w64-mingw32
MINGW-W64_TOOLCHAIN_PREFIX := $(MINGW-W64_TOOLCHAIN_HOST)-gcc
MINGW-W64_TOOLCHAIN_SUFFIX := posix

MINGW-W64_TOOLCHAIN_AR     := $(MINGW-W64_TOOLCHAIN_PREFIX)-ar-$(MINGW-W64_TOOLCHAIN_SUFFIX)
MINGW-W64_TOOLCHAIN_CC     := $(MINGW-W64_TOOLCHAIN_PREFIX)-$(MINGW-W64_TOOLCHAIN_SUFFIX)
MINGW-W64_TOOLCHAIN_RANLIB := $(MINGW-W64_TOOLCHAIN_PREFIX)-ranlib-$(MINGW-W64_TOOLCHAIN_SUFFIX)

# TODO Ideally, all remaining modules would be replaced with comso versions
GNULIB_MODULES :=								\
	fcntl										\
	fcntl-h										\
	getrusage									\
	ioctl										\
	poll-h										\
	poll										\
	pread										\
	sigaction									\
	termios

GNULIB_FILES :=									\
	third_party/gnulib_build/config.h			\
	third_party/gnulib_build/lib/fcntl.h		\
	third_party/gnulib_build/lib/poll.h			\
	third_party/gnulib_build/lib/signal.h		\
	third_party/gnulib_build/lib/sys/ioctl.h	\
	third_party/gnulib_build/lib/sys/resource.h	\
	third_party/gnulib_build/lib/termios.h		\
	third_party/gnulib_build/lib/unistd.h

GNULIB_LIB := third_party/gnulib_build/lib/libgnu.a

o/$(MODE)/x86_64-mingw-w64/%.o: %.S
	@mkdir -p $(@D)
	$(MINGW-W64_TOOLCHAIN_CC) -static $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<


o/$(MODE)/x86_64-mingw-w64/%.o: %.c third_party/gnulib_build/config.h
	@mkdir -p $(@D)
	$(MINGW-W64_TOOLCHAIN_CC) -static $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -D_WIN32_WINNT=0x0600 -Iblink/windows/headerwrappers/ -c -o $@ $<

o/$(MODE)/x86_64-mingw-w64/third_party/gnulib_build/lib/%.o: private	\
	CFLAGS +=															\
		-Ithird_party/gnulib_build/										\
		-Ithird_party/gnulib_build/lib/

# TODO: Find a way to allow rerunning this without running the clean target first
# TODO: Rerun this whenever GNULIB_MODULES changes
third_party/gnulib_build/config.h:
	@case "$(MAKECMDGOALS)" in \
	*x86_64-mingw-w64*) cd third_party/gnulib_build || exit 1; \
		../gnulib/gnulib-tool --lgpl --import $(GNULIB_MODULES) && \
		autoreconf -i && \
		./configure --host=$(MINGW-W64_TOOLCHAIN_HOST) AR=$(MINGW-W64_TOOLCHAIN_AR) RANLIB=$(MINGW-W64_TOOLCHAIN_RANLIB) && \
		$(MAKE) -C lib;; \
	esac
