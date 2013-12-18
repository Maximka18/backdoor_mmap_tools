LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  ../cred.c \
  ../kallsyms.c \
  autoexec.c \
  ../ptmx.c \
  ../backdoor_mmap.c

LOCAL_MODULE := run_autoexec
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES += .
LOCAL_STATIC_LIBRARIES += libdevice_database
LOCAL_STATIC_LIBRARIES += libcutils libc
LOCAL_LDFLAGS += -static

include $(BUILD_EXECUTABLE)
