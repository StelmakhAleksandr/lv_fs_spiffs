#include "lv_fs_spiffs.h"

#include "SPIFFS.h"
#include "monitor.h"
#include <Arduino.h>

static void * fs_open (lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close (lv_fs_drv_t * drv, void * file_p);
static lv_fs_res_t fs_read (lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);
static lv_fs_res_t fs_seek (lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t fs_tell (lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);
static void * fs_dir_open (lv_fs_drv_t * drv, const char *path);
static lv_fs_res_t fs_dir_read (lv_fs_drv_t * drv, void * dir_p, char *fn);
static lv_fs_res_t fs_dir_close (lv_fs_drv_t * drv, void * dir_p);

void lv_fs_spiffs_init(void) {
	SPIFFS.begin(true);
	static lv_fs_drv_t fs_drv;                
	lv_fs_drv_init(&fs_drv);

	fs_drv.letter = LV_FS_SPIFFS_LETTER;
	fs_drv.open_cb = fs_open;
	fs_drv.close_cb = fs_close;
	fs_drv.read_cb = fs_read;
	fs_drv.write_cb = fs_write;
	fs_drv.seek_cb = fs_seek;
	fs_drv.tell_cb = fs_tell;

	fs_drv.dir_close_cb = fs_dir_close;
	fs_drv.dir_open_cb = fs_dir_open;
	fs_drv.dir_read_cb = fs_dir_read;

	lv_fs_drv_register(&fs_drv);
}

static void * fs_open (lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode) {
    char *spiffsmode = FILE_READ;
    if(mode == LV_FS_MODE_WR)
        spiffsmode = FILE_WRITE;
    return new File(SPIFFS.open(path, spiffsmode));
}

static lv_fs_res_t fs_close (lv_fs_drv_t * drv, void * file_p) {
	File *file = (File*)file_p;
	file->close();
	delete file;
	return LV_FS_RES_OK;
}

static lv_fs_res_t fs_read (lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br) {
	File *file = (File*)file_p;
	if(!file->available())
		return LV_FS_RES_UNKNOWN;
	*br = file->read((uint8_t*)buf, btr);
	return LV_FS_RES_OK;
}

static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw) {
	return LV_FS_RES_OK;
}

static lv_fs_res_t fs_seek (lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence) {
	File *file = (File*)file_p;
	SeekMode mode = SeekCur;
    switch(whence) {
        case LV_FS_SEEK_SET: mode = SeekSet;
        break; 
        case LV_FS_SEEK_CUR: mode = SeekCur;
        break;
        case LV_FS_SEEK_END: mode = SeekEnd;
        break;
        default : break;
    }
	if(file->seek(pos, mode)) 
		return LV_FS_RES_OK;
	return LV_FS_RES_UNKNOWN;
}

static lv_fs_res_t fs_tell (lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p) {
	File *file = (File*)file_p;
	*pos_p = file->position();
	return LV_FS_RES_OK;
}

static void * fs_dir_open (lv_fs_drv_t * drv, const char *path) {
	return new File(SPIFFS.open(path));
}

static lv_fs_res_t fs_dir_read (lv_fs_drv_t * drv, void * dir_p, char *fn) {
   	File *dir = (File*)dir_p;
	File file = dir->openNextFile();
    if(file) {
        strcpy(fn, file.name());
        return LV_FS_RES_OK;
    } 
    return LV_FS_RES_UNKNOWN;
}

static lv_fs_res_t fs_dir_close (lv_fs_drv_t * drv, void * dir_p) {
    File *dir = (File*)dir_p;
    dir->close();
	delete dir;
	return LV_FS_RES_OK;
}
