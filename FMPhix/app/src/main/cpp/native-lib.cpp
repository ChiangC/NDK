#include <jni.h>
#include <string>
#include "dalvik.h"
#include "art_7_0.h"
//#include "art_method.h"//For Art 6.0

typedef Object *(*DvmDecodeIndirectRef)(void *thread, jobject jobj);

typedef void *(*DvmThreadSelf)();

DvmDecodeIndirectRef dvmDecodeIndirectRef;
DvmThreadSelf dvmThreadSelf;

extern "C" {

JNIEXPORT void JNICALL
Java_com_fmtech_fmphix_DexManager_replace(JNIEnv *env, jobject instance, jint sdkVersionCode,
                                          jobject wrongMethodObj, jobject rightMethodObj) {
    //找到Java方法在Native对应的Method结构体
    Method *wrongMethod = (Method *) env->FromReflectedMethod(wrongMethodObj);
    Method *rightMethod = (Method *) env->FromReflectedMethod(rightMethodObj);

    /*接下来操作的主要目的：修改rightMethod第一个成员变量ClassObject的status为CLASS_INITIALIZED*/
    /**
     * 通过函数dlopen打开指定的动态链接库，得到句柄后，再通过dlsym函数获取动态库中的函数地址，
     * 获得函数地址后进行调用
     */
    void *dvm_handle = dlopen("libdvm.so", RTLD_NOW);

    dvmDecodeIndirectRef = (DvmDecodeIndirectRef) dlsym(dvm_handle, sdkVersionCode > 10
                                                                    ? "_Z20dvmDecodeIndirectRefP6ThreadP8_jobject"
                                                                    :
                                                                    "dvmDecodeIndirectRef");

    dvmThreadSelf = (DvmThreadSelf) dlsym(dvm_handle, sdkVersionCode > 10 ? "_Z13dvmThreadSelfv"
                                                                          : "dvmThreadSelf");

    jclass methodClazz = env->FindClass("java/lang/reflect/Method");
    jmethodID rightMethodId = env->GetMethodID(methodClazz, "getDeclaringClass",
                                               "()Ljava/lang/Class;");

    jobject obj = env->CallObjectMethod(rightMethodObj, rightMethodId);

    ClassObject *clazz = (ClassObject *) dvmDecodeIndirectRef(dvmThreadSelf(), obj);
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

    jthrowable exc = env->ExceptionOccurred();
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

JNIEXPORT void JNICALL
Java_com_fmtech_fmphix_DexManager_replaceArt(JNIEnv *env, jobject instance, jint sdkVersionCode,
                                             jobject wrongMethodObj, jobject rightMethodObj) {
    art::mirror::ArtMethod *wrongMethod = (art::mirror::ArtMethod *) env->FromReflectedMethod(
            wrongMethodObj);
    art::mirror::ArtMethod *rightMethod = (art::mirror::ArtMethod *) env->FromReflectedMethod(
            rightMethodObj);

    /*方式一、ArtMethod内存拷贝整体替换*/
    jclass jclazz = env->FindClass("com/fmtech/fmphix/NativeStructsModel");
    size_t methodF1Id = (size_t)env->GetStaticMethodID(jclazz, "f1", "()V");
    size_t methodF2Id = (size_t)env->GetStaticMethodID(jclazz, "f2", "()V");

    int size = methodF2Id - methodF1Id;
    memcpy(wrongMethod, rightMethod, size);

//    /*方式二、ArtMethod属性替换*/
//    //方法所在类(即可以将一个类的方法指向另一个类的方法)
//    wrongMethod->declaring_class_ = rightMethod->declaring_class_;
//
//    /****Art 6.0 Start*****/
//    //快捷访问方式
////    wrongMethod->dex_cache_resolved_methods_ = rightMethod->dex_cache_resolved_methods_;
//    //
////    wrongMethod->dex_cache_resolved_types_ = rightMethod->dex_cache_resolved_types_;
//    /****Art 6.0 End*****/
//
//    //索引偏移量
//    wrongMethod->dex_code_item_offset_ = rightMethod->dex_code_item_offset_;
//    //方法索引
//    wrongMethod->method_index_ = rightMethod->method_index_;
//    //
//    wrongMethod->dex_method_index_ = rightMethod->dex_method_index_;
//
//    /****Art 6.0 Start*****/
//    //ArtMethod方法入口
////    wrongMethod->ptr_sized_fields_.entry_point_from_interpreter_ = rightMethod->ptr_sized_fields_.entry_point_from_interpreter_;
//    /****Art 6.0 End*****/
//
//    wrongMethod->ptr_sized_fields_.entry_point_from_jni_ = rightMethod->ptr_sized_fields_.entry_point_from_jni_;
//
//    //Art机器模式的入口
//    wrongMethod->ptr_sized_fields_.entry_point_from_quick_compiled_code_ = rightMethod->ptr_sized_fields_.entry_point_from_quick_compiled_code_;
//
//    /****Art 7.0 Start*****/
//    wrongMethod->ptr_sized_fields_.entry_point_from_jni_ = rightMethod->ptr_sized_fields_.entry_point_from_jni_;
//    wrongMethod->ptr_sized_fields_.dex_cache_resolved_types_ = rightMethod->ptr_sized_fields_.dex_cache_resolved_types_;
//    wrongMethod->ptr_sized_fields_.dex_cache_resolved_methods_ = rightMethod->ptr_sized_fields_.dex_cache_resolved_methods_;
//    wrongMethod->hotness_count_ = rightMethod->hotness_count_;
//    /****Art 7.0 End*****/

}

}
