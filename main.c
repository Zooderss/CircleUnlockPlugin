/*
 * circle_unlock - hold O (Circle) on the Vita's wake/lock screen to
 * simulate the swipe-up gesture that normally dismisses it.
 *
 * Load this under the *main section of ux0:tai/config.txt so it only
 * runs inside the shell process (LiveArea), not inside games.
 *
 * IMPORTANT: the two NIDs below (SCE_CTRL_PEEK_NID and SCE_TOUCH_PEEK_NID)
 * must be verified against vitasdk-headers/db.yml for your target
 * firmware before you trust this build. They're filled in with the
 * commonly-cited values for sceCtrlPeekBufferPositive2 and sceTouchPeek,
 * but NIDs have occasionally been corrected upstream, so double check:
 *   https://github.com/vitasdk/vita-headers/blob/master/db.yml
 * (search for "sceCtrlPeekBufferPositive2" and "sceTouchPeek")
 *
 * This also has NOT been tested against the real shell swipe-recognizer,
 * so the swipe coordinates/timing below (see SWIPE tuning constants)
 * are a starting point, not a guarantee. Expect to iterate.
 */

#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <taihen.h>
#include <string.h>

/* --- NIDs: verify these against vitasdk-headers db.yml for your FW --- */
#define SCE_CTRL_PEEK_NID   0xA9C3CED6  /* sceCtrlPeekBufferPositive2 */
#define SCE_TOUCH_PEEK_NID  0xFF082DF0  /* sceTouchPeek               */

/* --- Tuning --- */
#define HOLD_FRAMES_REQUIRED 60  /* ~1s of holding O at 60Hz polling  */
#define SWIPE_INJECT_FRAMES  20  /* frames over which the fake swipe plays out */
#define SWIPE_START_Y        1000
#define SWIPE_END_Y          100
#define SWIPE_X              960 /* horizontal center; front panel is ~1920 wide */

static tai_hook_ref_t g_ctrl_hook_ref;
static tai_hook_ref_t g_touch_hook_ref;
static SceUID g_ctrl_hook_uid = -1;
static SceUID g_touch_hook_uid = -1;

static int g_circle_hold_frames = 0;
static int g_swipe_active = 0;
static int g_swipe_frame = 0;

static int ctrl_peek_patched(int port, SceCtrlData *pad_data, int count) {
    int ret = TAI_CONTINUE(int, g_ctrl_hook_ref, port, pad_data, count);

    if (ret > 0 && pad_data != NULL) {
        if (pad_data->buttons & SCE_CTRL_CIRCLE) {
            g_circle_hold_frames++;
        } else {
            g_circle_hold_frames = 0;
        }

        if (g_circle_hold_frames >= HOLD_FRAMES_REQUIRED && !g_swipe_active) {
            g_swipe_active = 1;
            g_swipe_frame = 0;
        }
    }

    return ret;
}

static int touch_peek_patched(int port, SceTouchData *pData, int nBufs) {
    int ret = TAI_CONTINUE(int, g_touch_hook_ref, port, pData, nBufs);

    /* Only fake input on the front panel (port 0) */
    if (g_swipe_active && port == 0 && pData != NULL && nBufs > 0) {
        int y = SWIPE_START_Y - ((SWIPE_START_Y - SWIPE_END_Y) * g_swipe_frame) / SWIPE_INJECT_FRAMES;

        memset(&pData[0], 0, sizeof(pData[0]));
        pData[0].reportNum = 1;
        pData[0].report[0].x = SWIPE_X;
        pData[0].report[0].y = y;
        pData[0].report[0].id = 0;
        pData[0].report[0].force = 128;

        ret = 1;

        g_swipe_frame++;
        if (g_swipe_frame > SWIPE_INJECT_FRAMES) {
            g_swipe_active = 0;
            g_circle_hold_frames = 0;
        }
    }

    return ret;
}

int module_start(SceSize argc, const void *args) {
    (void)argc;
    (void)args;

    g_ctrl_hook_uid = taiHookFunctionImport(&g_ctrl_hook_ref,
        TAI_MAIN_MODULE,
        TAI_ANY_LIBRARY,
        SCE_CTRL_PEEK_NID,
        ctrl_peek_patched);

    g_touch_hook_uid = taiHookFunctionImport(&g_touch_hook_ref,
        TAI_MAIN_MODULE,
        TAI_ANY_LIBRARY,
        SCE_TOUCH_PEEK_NID,
        touch_peek_patched);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
    (void)argc;
    (void)args;

    if (g_ctrl_hook_uid >= 0) taiHookRelease(g_ctrl_hook_uid, g_ctrl_hook_ref);
    if (g_touch_hook_uid >= 0) taiHookRelease(g_touch_hook_uid, g_touch_hook_ref);

    return SCE_KERNEL_STOP_SUCCESS;
}
