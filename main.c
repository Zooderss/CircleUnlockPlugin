#include <vitasdk.h>
#include <taihen.h>
#include <stdbool.h>

// Hook handle for sceCtrlPeekBufferPositive
static tai_hook_ref_t g_ctrl_hook_ref;
static SceUID g_ctrl_hook_id = -1;

// Hold timer configuration (0.5 seconds @ 60 FPS = 30 frames)
#define HOLD_THRESHOLD_FRAMES 30
static uint32_t g_circle_hold_counter = 0;
static bool g_unlock_triggered = false;

// SceShell internal/utility API for dismissing the lock screen
extern int sceShellUtilLockScreenDismiss(void);

// Hooked controller read function
static int sceCtrlPeekBufferPositive_patched(int port, SceCtrlData *pad_data, int count) {
    // Call the original function to populate pad_data
    int ret = TAI_CONTINUE(int, g_ctrl_hook_ref, port, pad_data, count);

    if (ret >= 0 && pad_data != NULL && count > 0) {
        // Check if the Circle button is currently held
        if (pad_data->buttons & SCE_CTRL_CIRCLE) {
            g_circle_hold_counter++;

            // Trigger when threshold is reached, ensuring it only fires once per hold
            if (g_circle_hold_counter >= HOLD_THRESHOLD_FRAMES && !g_unlock_triggered) {
                // Call SceShell API to dismiss the lock screen
                sceShellUtilLockScreenDismiss();
                
                g_unlock_triggered = true;
            }
        } else {
            // Reset state when Circle is released
            g_circle_hold_counter = 0;
            g_unlock_triggered = false;
        }
    }

    return ret;
}

// Plugin Entry Point
void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize argc, const void *argv) {
    // Hook sceCtrlPeekBufferPositive in SceShell's controller module
    g_ctrl_hook_id = taiHookFunctionImport(
        &g_ctrl_hook_ref,
        TAI_MAIN_MODULE,
        0xD197E3C7, // SceCtrl import interface NID
        0x67E9ED85, // sceCtrlPeekBufferPositive NID
        sceCtrlPeekBufferPositive_patched
    );

    return SCE_KERNEL_START_SUCCESS;
}

// Plugin Stop Point
int module_stop(SceSize argc, const void *argv) {
    if (g_ctrl_hook_id >= 0) {
        taiHookRelease(g_ctrl_hook_id, g_ctrl_hook_ref);
    }
    return SCE_KERNEL_STOP_SUCCESS;
}
