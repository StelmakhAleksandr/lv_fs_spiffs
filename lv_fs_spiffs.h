#ifndef LV_FS_SPIFFS_H
#define LV_FS_SPIFFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if LV_LVGL_H_INCLUDE_SIMPLE
#include <lvgl.h>
#else
#include <lvgl/lvgl.h>
#endif

#if LV_USE_FS_SPIFFS

void lv_fs_spiffs_init(void);

#endif	/*LV_USE_FS_SPIFFS*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_FS_SPIFFS_H*/

