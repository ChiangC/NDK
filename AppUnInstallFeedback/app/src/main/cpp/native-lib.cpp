#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring

JNICALL
Java_com_fmtech_appuninstallfeedback_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "双进程守护：App卸载监听";
    return env->NewStringUTF(hello.c_str());
}
