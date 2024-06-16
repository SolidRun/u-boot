ifeq ($(CONFIG_ARCH_CN20K),y)
PLATFORM_CPPFLAGS += $(call cc-option,-march=armv9-a,)
endif
