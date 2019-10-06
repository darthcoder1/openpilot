CC = clang
CXX = clang++

BUILD_TARGET=$(shell uname)
BUILD_MODE=Debug

PHONELIBS = ../../../phonelibs

WARN_FLAGS = -Werror=implicit-function-declaration \
             -Werror=incompatible-pointer-types \
             -Werror=int-conversion \
             -Werror=return-type \
             -Werror=format-extra-args

ifeq ($(BUILD_MODE), Debug)
    CFLAGS = -g -std=gnu11 -fPIC $(WARN_FLAGS)
    CXXFLAGS = -g -std=c++11 -fPIC $(WARN_FLAGS)
else
    CFLAGS = -std=gnu11 -fPIC -O2 $(WARN_FLAGS)
    CXXFLAGS = -std=c++11 -fPIC -O2 $(WARN_FLAGS)
endif


NANOVG_FLAGS = -I$(PHONELIBS)/nanovg


ifeq ($(BUILD_TARGET), Linux)
    FRAMEBUFFER_LIBS = -lEGL -lX11
    OPENGL_LIBS = -lGLESv2
    OTHER_LIBS =
else
    FRAMEBUFFER_LIBS = -lutils -lgui -lEGL
    OPENGL_LIBS = -lGLESv3
    OTHER_LIBS =  -llog
endif

OBJS = spinner.o \
    ../../common/framebuffer.o \
    $(PHONELIBS)/nanovg/nanovg.o \
    ../../common/spinner.o \
    opensans_semibold.o \
    img_spinner_track.o \
    img_spinner_comma.o

DEPS := $(OBJS:.o=.d)

.PHONY: all
all: spinner

spinner: $(OBJS)
	@echo "[ LINK ] $@"
	$(CXX) -fPIC -o '$@' $^ \
	      -s \
        $(FRAMEBUFFER_LIBS) \
        -L/system/vendor/lib64 \
        $(OPENGL_LIBS) \
        $(OTHER_LIBS) \
        -lm

../../common/framebuffer.o: ../../common/framebuffer.cc
	@echo "[ CXX ] $@"
	$(CXX) $(CXXFLAGS) -MMD \
           -I$(PHONELIBS)/android_frameworks_native/include \
           -I$(PHONELIBS)/android_system_core/include \
           -I$(PHONELIBS)/android_hardware_libhardware/include \
           -c -o '$@' '$<'

opensans_semibold.o: ../../assets/fonts/opensans_semibold.ttf
	@echo "[ bin2o ] $@"
	cd '$(dir $<)' && ld -r -b binary '$(notdir $<)' -o '$(abspath $@)'

img_spinner_track.o: ../../assets/img_spinner_track.png
	@echo "[ bin2o ] $@"
	cd '$(dir $<)' && ld -r -b binary '$(notdir $<)' -o '$(abspath $@)'

img_spinner_comma.o: ../../assets/img_spinner_comma.png
	@echo "[ bin2o ] $@"
	cd '$(dir $<)' && ld -r -b binary '$(notdir $<)' -o '$(abspath $@)'

%.o: %.c
	@echo "[ CC ] $@"
	$(CC) $(CFLAGS) -MMD \
          -I../.. \
          $(NANOVG_FLAGS) \
          -c -o '$@' '$<'

.PHONY: clean
clean:
	rm -f spinner $(OBJS) $(DEPS)

-include $(DEPS)
