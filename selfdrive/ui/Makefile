CC = clang
CXX = clang++


PHONELIBS = ../../phonelibs

WARN_FLAGS = -Werror=implicit-function-declaration \
             -Werror=incompatible-pointer-types \
             -Werror=int-conversion \
             -Werror=return-type \
             -Werror=format-extra-args

CFLAGS = -std=gnu11 -fPIC -O2 $(WARN_FLAGS) -DARCH_$(ARCH) -DQCOM
CXXFLAGS = -std=c++11 -fPIC -O2 $(WARN_FLAGS) -DARCH_$(ARCH) -DQCOM

ifeq ($(ARCH),x86_64)
	ZMQ_LIBS = -lczmq -lzmq -L$(PHONELIBS)/zmq/x64/lib/
	ZMQ_FLAGS = -I$(PHONELIBS)/zmq/x64/include
	OPENSL_LIBS =
	CEREAL_LIBS = -lcapnp -lcapnp_c
	FRAMEBUFFER_LIBS =
	OPENGL_LIBS = -lGLESv2 -lX11 -lEGL
	OPENCL_LIBS = -lOpenCL

	OTHER_LIBS = -lpthread -lhardware
endif
ifeq ($(ARCH),aarch64)
	CFLAGS += -DQCOM

	ZMQ_LIBS = -l:libczmq.a -l:libzmq.a -lgnustl_shared
	OPENSL_LIBS = -lOpenSLES
	CEREAL_LIBS = -l:libcapn.a
	FRAMEBUFFER_LIBS = -lutils -lgui -lEGL
	OPENGL_LIBS = -lGLESv3
	OPENCL_LIBS = -lgsl -lCB -lOpenCL
	OTHER_INCLUDE_DIRS =
	OTHER_LIBS = -lhardware -lui -lcutils -llog -ladreno_utils
endif

CEREAL_OBJS = ../../cereal/gen/c/log.capnp.o

NANOVG_FLAGS = -I$(PHONELIBS)/nanovg
JSON_FLAGS = -I$(PHONELIBS)/json/src

OPENCL_FLAGS = -I$(PHONELIBS)/opencl/include
UUID_LIBS = -luuid


CFLAGS +=-I$(PHONELIBS)/android_frameworks_native/include \
          -I$(PHONELIBS)/android_system_core/include \
          -I$(PHONELIBS)/android_hardware_libhardware/include
CXXFLAGS += -I$(PHONELIBS)/android_frameworks_native/include \
            -I$(PHONELIBS)/android_system_core/include \
            -I$(PHONELIBS)/android_hardware_libhardware/include

ifeq ($(ARCH),aarch64)
CFLAGS += -mcpu=cortex-a57
CXXFLAGS += -mcpu=cortex-a57
endif

OBJS = slplay.o \
       ui.o \
       ../common/glutil.o \
       ../common/visionipc.o \
       ../common/ipc.o \
       ../common/visionimg.o \
       ../common/visionbuf_ion.o \
       ../common/framebuffer.o \
       ../common/params.o \
       ../common/util.o \
       ../common/touch.o \
       ../common/swaglog.o \
       $(PHONELIBS)/nanovg/nanovg.o \
       $(PHONELIBS)/json/src/json.o \
       $(CEREAL_OBJS)

DEPS := $(OBJS:.o=.d)

all: ui


ui: $(OBJS)
	@echo "[ LINK ] $@"
	$(CXX) -fPIC -o '$@' $^ \
	$(FRAMEBUFFER_LIBS) \
	$(CEREAL_LIBS) \
	$(ZMQ_LIBS) \
	-L/system/vendor/lib64 \
	$(OTHER_LIBS) \
	$(OPENGL_LIBS) \
	$(OPENCL_LIBS) \
	${OPENSL_LIBS} \
	${UUID_LIBS} \
	-Wl,-rpath=/system/lib64,-rpath=/system/comma/usr/lib \
	-lm

slplay.o: slplay.c
	@echo "[ CC ] $@"
	$(CC) $(CFLAGS) -fPIC \
	-I../ \
	$(OPENSL_LIBS) \
	-c -o '$@' $^

%.o: %.cc
	@echo "[ CXX ] $@"
	$(CXX) $(CXXFLAGS) -MMD \
	-Iinclude -I.. -I../.. \
	$(OPENCL_FLAGS) \
	-I$(PHONELIBS)/android_frameworks_native/include \
	-I$(PHONELIBS)/android_system_core/include \
	-I$(PHONELIBS)/android_hardware_libhardware/include \
	-I$(PHONELIBS)/libgralloc/include \
	-I$(PHONELIBS)/linux/include \
	-I$(ZMQ_FLAGS) \
	-c -o '$@' '$<'

%.o: %.c
	@echo "[ CC ] $@"
	$(CC) $(CFLAGS) -MMD \
	-Iinclude -I.. -I../.. \
	$(NANOVG_FLAGS) \
	$(ZMQ_FLAGS) \
	$(CEREAL_CFLAGS) \
	$(JSON_FLAGS) \
	$(OPENCL_FLAGS) \
	-I$(PHONELIBS)/linux/include \
	-c -o '$@' '$<'

.PHONY: clean

clean:
	rm -f ui $(OBJS) $(DEPS)

print_vars:
	echo $(ARCH)



-include $(DEPS)
