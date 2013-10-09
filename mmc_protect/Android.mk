LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  ../kallsyms.c \
  unlock_mmc_protect.c \
  ../ptmx.c \
  ../backdoor_mmap.c

LOCAL_MODULE := unlock_mmc_protect
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES += .
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_STATIC_LIBRARIES += libdevice_database
LOCAL_STATIC_LIBRARIES += libcutils libc

include $(BUILD_EXECUTABLE)
