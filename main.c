#include <vitasdk.h>
#include <taihen.h>
#include <stdbool.h>

static tai_hook_ref_t g_ctrl_hook_ref;
static SceUID g_ctrl_hook_id = -1;

#define HOLD_THRESHOLD_FRAMES 30
static uint32_t g_circle_hold_counter = 0;
static bool g_unlock_triggered = false;

extern int sceShellUtilLockScreenDismiss(void);

// Define function pointer type matching sceCtrlPeekBufferPositive
typedef int (*SceCtrlPeekBufferFunc)(int port, SceCtrlData *pad_data, int count);

// Internal taiHEN structure layout to safely invoke hooks with arguments on modern GCC
typedef struct _my_tai_hook_user {
    uintptr_t next;
    void *func;
    void *old;
} _my_tai_hook_user;

// Typed continuation macro bypassing strict empty-parameter casting issues in GCC 15
#define SAFE_TAI_CONTINUE(func_type, hook, ...) ({ \
    _my_tai_hook_user *cur, *next; \
    cur = (_my_tai_hook_user *)(hook); \
    next = (_my_tai_hook_user *)cur->next; \
    (next == NULL) ? \
        ((func_type)(cur->old))(__VA_ARGS__) \
        : \
        ((func_type)(next->func))(__VA_ARGS__); \
})

// Hooked controller read function
static int sceCtrlPeekBufferPositive_patched(int port, SceCtrlData *pad_data, int count) {
    // Call the original function safely using our typed continuation macro
    int ret = SAFE_TAI_CONTINUE(SceCtrlPeekBufferFunc, g_ctrl_hook_ref, port, pad_data, count);

    if (ret >= 0 && pad_data != NULL && count > 0) {
        if (pad_data->buttons & SCE_CTRL_CIRCLE) {
            g_circle_hold_counter++;

            if (g_circle_hold_counter >= HOLD_THRESHOLD_FRAMES && !g_unlock_triggered) {
                sceShellUtilLockScreenDismiss();
                g_unlock_triggered = true;
            }
        } else {
            g_circle_hold_counter = 0;
            g_unlock_triggered = false;
        }
    }

    return ret;
}

int module_start(SceSize argc, const void *argv) {
    g_ctrl_hook_id = taiHookFunctionImport(
        &g_ctrl_hook_ref,
        TAI_MAIN_MODULE,
        0xD197E3C7, // SceCtrl NID
        0x67E9ED85, // sceCtrlPeekBufferPositive NID
        sceCtrlPeekBufferPositive_patched
    );

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *argv) {
    if (g_ctrl_hook_id >= 0) {
        taiHookRelease(g_ctrl_hook_id, g_ctrl_hook_ref);
    }
    return SCE_KERNEL_START_SUCCESS;
}
