LOCAL_PATH := $(call my-dir)

#x264
include $(CLEAR_VARS)
LOCAL_MODULE    := x264
#EXPORT����ȫ��
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/x264/include
LOCAL_SRC_FILES := x264/libx264.a
#�ѱ����Դ�ļ���ӡ����
$(info $(LOCAL_SRC_FILES))
include $(PREBUILT_STATIC_LIBRARY)

#faac
include $(CLEAR_VARS)
LOCAL_MODULE    := faac
#EXPORT����ȫ��
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/faac/include
LOCAL_SRC_FILES := faac/libfaac.a
$(info $(LOCAL_SRC_FILES))
include $(PREBUILT_STATIC_LIBRARY)

#rtmp
include $(CLEAR_VARS)
LOCAL_MODULE    := rtmpdump
#EXPORT����ȫ��
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/rtmpdump/include
LOCAL_SRC_FILES := rtmpdump/librtmp.a
$(info $(LOCAL_SRC_FILES))
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := FMLive
LOCAL_SRC_FILES := FMLive.c queue.c
LOCAL_STATIC_LIBRARIES := x264 faac rtmpdump
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
