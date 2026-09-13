#include <vitasdk.h>
#include <taihen.h>
#include <stdbool.h>

static tai_hook_ref_t g_ctrl_hook_ref;
static SceUID g_ctrl_hook_id = -1;

#define HOLD_THRESHOLD_FRAMES 30
static uint32_t g_circle_hold_counter = 0;
static bool g_unlock_triggered = false;

extern int sceShellUtilLockScreenDismiss(void);

static int sceCtrlPeekBufferPositive_patched(int port, SceCtrlData *pad_data, int count) {
    int ret = TAI_CONTINUE(int, g_ctrl_hook_ref, port, pad_data, count);

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

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize argc, const void *argv) {
    g_ctrl_hook_id = taiHookFunctionImport(
        &g_ctrl_hook_ref,
        TAI_MAIN_MODULE,
        0xD197E3C7,
        0x67E9ED85,
        sceCtrlPeekBufferPositive_patched
    );

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *argv) {
    if (g_ctrl_hook_id >= 0) {
        taiHookRelease(g_ctrl_hook_id, g_ctrl_hook_ref);
    }
    return SCE_KERNEL_STOP_SUCCESS;
}
