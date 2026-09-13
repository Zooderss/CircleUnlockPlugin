#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/proc[span_35](start_span)[span_35](end_span)essmgr.h>
#include <psp2/ctrl.h>
#include <psp2/shellutil.h>
#include <taihen.h>
#include <string.h>

#define PLUGIN_NAME "CircleUnlock"
#define HOLD_THRESHOLD_TICKS 60
#define THREAD_LOOP_DELAY_US 16000

SCE_MODULE_INFO(CircleUnlock, 0, 1, 1);

static SceUID g_worker_thread_id = -1;
static int g_running = 0;

static int unlock_worker_thread(SceSize args, void *argp) {
    SceCtrlData pad;
    unsigned int hold_counter = 0;
    int unlock_triggered = 0;

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITAL);

    while (g_running) {
        int ret = sceCtrlPeekBufferPositive(0, &pad, 1);
        
        if (ret >= 0) {
            if (pad.buttons & SCE_CTRL_CIRCLE) {
                hold_counter++;
                
                if (hold_counter >= HOLD_THRESHOLD_TICKS && !unlock_triggered) {
                    sceShellUtilUnlock(1);
                    unlock_triggered = 1;
                }
            } else {
                hold_counter = 0;
                unlock_triggered = 0;
            }
        }

        sceKernelDelayThread(THREAD_LOOP_DELAY_US);
    }

    return 0;
}

int module_start(SceSize args, const void *argp) {
    g_running = 1;

    g_worker_thread_id = sceKernelCreateThread(
        "CircleUnlockWorker",
        unlock_worker_thread,
        0x10000100,
        0x4000,
        0,
        0,
        NULL
    );

    if (g_worker_thread_id >= 0) {
        sceKernelStartThread(g_worker_thread_id, 0, NULL);
    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, const void *argp) {
    g_running = 0;

    if (g_worker_thread_id >= 0) {
        sceKernelWaitThreadEnd(g_worker_thread_id, NULL, NULL);
        sceKernelDeleteThread(g_worker_thread_id);
    }

    return SCE_KERNEL_STOP_SUCCESS;
}
