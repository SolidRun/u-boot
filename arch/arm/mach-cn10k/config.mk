ifeq ($(CONFIG_ARCH_CN10K),y)
PLATFORM_CPPFLAGS += $(call cc-option,-march=armv9-a,)
PLATFORM_CPPFLAGS += $(call cc-option,-mtune=neoverse-n2)
endif
