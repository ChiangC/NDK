#include <jni.h>
#include <string>
#include "dalvik.h"

typedef Object *(*DvmDecodeIndirectRef)(void *thread, jobject jobj);
typedef void* (*DvmThreadSelf)();
DvmDecodeIndirectRef  dvmDecodeIndirectRef;
DvmThreadSelf dvmThreadSelf;

extern "C" {

JNIEXPORT void JNICALL
Java_com_fmtech_fmphix_DexManager_replace(JNIEnv *env, jobject instance, jint sdkVersionCode, jobject wrongMethodObj, jobject rightMethodObj) {
    //找到Java方法在Native对应的Method结构体
    Method *wrongMethod = (Method*)env->FromReflectedMethod(wrongMethodObj);
    Method *rightMethod = (Method*)env->FromReflectedMethod(rightMethodObj);

    /*接下来操作的主要目的：修改rightMethod第一个成员变量ClassObject的status为CLASS_INITIALIZED*/
    /**
     * 通过函数dlopen打开指定的动态链接库，得到句柄后，再通过dlsym函数获取动态库中的函数地址，
     * 获得函数地址后进行调用
     */
    void* dvm_handle = dlopen("libdvm.so", RTLD_NOW);

    dvmDecodeIndirectRef = (DvmDecodeIndirectRef)dlsym(dvm_handle, sdkVersionCode > 10 ?"_Z20dvmDecodeIndirectRefP6ThreadP8_jobject" :
                                                                   "dvmDecodeIndirectRef");

    dvmThreadSelf = (DvmThreadSelf)dlsym(dvm_handle, sdkVersionCode > 10 ? "_Z13dvmThreadSelfv" : "dvmThreadSelf");

    jclass methodClazz = env->FindClass("java/lang/reflect/Method");
    jmethodID rightMethodId = env->GetMethodID(methodClazz, "getDeclaringClass", "()Ljava/lang/Class;");

    jobject obj = env->CallObjectMethod(rightMethodObj, rightMethodId);

    ClassObject *clazz = (ClassObject*)dvmDecodeIndirectRef(dvmThreadSelf(), obj);
    clazz->status = CLASS_INITIALIZED;

    wrongMethod->accessFlags |= ACC_PUBLIC;
    wrongMethod->methodIndex = rightMethod->methodIndex;
    wrongMethod->jniArgInfo = rightMethod->jniArgInfo;
    wrongMethod->registersSize = rightMethod->registersSize;
    wrongMethod->outsSize = rightMethod->outsSize;

    //方法参数原型
    wrongMethod->prototype = rightMethod->prototype;
    wrongMethod->insns = rightMethod->insns;
    wrongMethod->nativeFunc = rightMethod->nativeFunc;

    jthrowable  exc = env->ExceptionOccurred();
    if (exc) {
        /* We don't do much with the exception, except thatwe print a debug message for it, clear it, andthrow a new exception. */
        jclass newExcCls;
        env->ExceptionDescribe();
        env->ExceptionClear();
        newExcCls = env->FindClass("java/lang/IllegalArgumentException");
        if (newExcCls == NULL) {
            /* Unable to find the exception class, give up. */
            return;
        }
        env->ThrowNew(newExcCls, "thrown from C code");
    }
}


}
