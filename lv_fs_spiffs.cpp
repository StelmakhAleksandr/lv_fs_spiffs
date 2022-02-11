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

void lv_fs_spiffs_init(void)
{
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

/**
 * Open a file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable
 * @param path path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static void * fs_open (lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
	Serial.printf("fileOpen %s \r\n", path);
	// showMonitor();
    char *spiffsmode = FILE_READ;
    if(mode == LV_FS_MODE_WR)
        spiffsmode = FILE_WRITE;
    return new File(SPIFFS.open(path, spiffsmode));
}


/**
 * Close an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable. (opened with lv_ufs_open)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_close (lv_fs_drv_t * drv, void * file_p)
{
	Serial.printf("fileClose \r\n");
	// showMonitor();
	File *file = (File*)file_p;
	file->close();
	delete file;
	return LV_FS_RES_OK;
}

/**
 * Read data from an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable.
 * @param buf pointer to a memory block where to store the read data
 * @param btr number of Bytes To Read
 * @param br the real number of read bytes (Byte Read)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_read (lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
	File *file = (File*)file_p;
	if(!file->available())
		return LV_FS_RES_UNKNOWN;
	*br = file->read((uint8_t*)buf, btr);
	return LV_FS_RES_OK;
}

/**
 * Write into a file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable
 * @param buf pointer to a buffer with the bytes to write
 * @param btr Bytes To Write
 * @param br the number of real written bytes (Bytes Written). NULL if unused.
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
	// (void) drv;		/*Unused*/
	// *bw = fwrite(buf, 1, btw, file_p);
	return LV_FS_RES_OK;
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable. (opened with lv_ufs_open )
 * @param pos the new position of read write pointer
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek (lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
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

/**
 * Give the position of the read write pointer
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable.
 * @param pos_p pointer to to store the result
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_tell (lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
	File *file = (File*)file_p;
	*pos_p = file->position();
	return LV_FS_RES_OK;
}

/**
 * Initialize a 'fs_read_dir_t' variable for directory reading
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to a 'fs_read_dir_t' variable
 * @param path path to a directory
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static void * fs_dir_open (lv_fs_drv_t * drv, const char *path)
{
	return new File(SPIFFS.open(path));
}

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'fs_read_dir_t' variable
 * @param fn pointer to a buffer to store the filename
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_read (lv_fs_drv_t * drv, void * dir_p, char *fn)
{
   	File *dir = (File*)dir_p;
	File file = dir->openNextFile();
    if(file) {
        strcpy(fn, file.name());
        return LV_FS_RES_OK;
    } 
    return LV_FS_RES_UNKNOWN;
}

/**
 * Close the directory reading
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'fs_read_dir_t' variable
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_close (lv_fs_drv_t * drv, void * dir_p)
{
    File *dir = (File*)dir_p;
    dir->close();
	delete dir;
	return LV_FS_RES_OK;
}
