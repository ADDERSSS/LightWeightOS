#include "fs/fatfs/fatfs.h" 
#include "dev/dev.h"
#include "fs/file.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "dev/dev.h"
#include "core/memory.h"
#include "tools/klib.h"
#include "fs/fs.h"
#include <sys/fcntl.h>

static int bread_sector (fat_t * fat, int sector) {
    if (sector == fat->curr_sector) {
        return 0;
    }

    int cnt = dev_read(fat->fs->dev_id, sector, fat->fat_buffer, 1);
    if (cnt == 1) {
        fat->curr_sector = sector;
        return 0;
    }
    return -1;
}

static int bwrite_secotr (fat_t * fat, int sector) {
    int cnt = dev_write(fat->fs->dev_id, sector, fat->fat_buffer, 1);
    return (cnt == 1) ? 0 : -1;
}

int cluster_is_valid (cluster_t cluster) {
    return (cluster < 0xFFF8) && (cluster >= 0x2);     // 值是否正确
}

/**
 * 获取指定簇的下一个簇
 */
int cluster_get_next (fat_t * fat, cluster_t curr) {
    if (!cluster_is_valid(curr)) {
        return FAT_CLUSTER_INVALID;
    }

    // 取fat表中的扇区号和在扇区中的偏移
    int offset = curr * sizeof(cluster_t);
    int sector = offset / fat->bytes_per_sec;
    int off_sector = offset % fat->bytes_per_sec;
    if (sector >= fat->tbl_sectors) {
        log_printf("cluster too big. %d", curr);
        return FAT_CLUSTER_INVALID;
    }

    // 读扇区，然后取其中簇数据
    int err = bread_sector(fat, fat->tbl_start + sector);
    if (err < 0) {
        return FAT_CLUSTER_INVALID;
    }

    return *(cluster_t*)(fat->fat_buffer + off_sector);
}

int cluster_set_next (fat_t * fat, cluster_t curr, cluster_t next) {
    if (!cluster_is_valid(curr)) {
        return -1;
    }

    int offset = curr * sizeof(cluster_t);
    int sector = offset / fat->bytes_per_sec;
    int off_sector = offset % fat->bytes_per_sec;
    if (sector >= fat->tbl_sectors) {
        log_printf("cluster too big. %d", curr);
        return -1;
    }

    // 读缓存
    int err = bread_sector(fat, fat->tbl_start + sector);
    if (err < 0) {
        return -1;
    }

    // 改next
    *(cluster_t*)(fat->fat_buffer + off_sector) = next;

    // 回写到多个表中
    for (int i = 0; i < fat->tbl_cnt; i++) {
        err = bwrite_secotr(fat, fat->tbl_start + sector);
        if (err < 0) {
            log_printf("write cluster failed.");
            return -1;
        }
        sector += fat->tbl_sectors;
    }
    return 0;
}

void diritem_get_name (diritem_t * item, char * dest) {
    char * c = dest;
    char * ext = (char *)0;

    kernel_memset(dest, 0, SFN_LEN + 1);     // 最多11个字符
    for (int i = 0; i < 11; i++) {
        if (item->DIR_Name[i] != ' ') {
            *c++ = item->DIR_Name[i];
        }

        if (i == 7) {
            ext = c;
            *c++ = '.';
        }
    }

    // 没有扩展名的情况
    if (ext && (ext[1] == '\0')) {
        ext[0] = '\0';
    }
}

void cluster_free_chain(fat_t * fat, cluster_t start) {
    while (cluster_is_valid(start)) {
        cluster_t next = cluster_get_next(fat, start);
        cluster_set_next(fat, start, FAT_CLUSTER_FREE);
        start = next;
    }
}

static void to_sfn (char * dest, const char * src) {
    kernel_memset(dest, ' ', 11);

    char * curr = dest;
    char * end = dest + 11;
    while (*src && (curr < end)) {
        char c = *src++;
        switch (c) {
            case '.':
                curr = dest + 8;
                break;
            default:
                if ((c >= 'a') && (c <= 'z')) {
                    c = c -'a' + 'A';
                }
                *curr++ = c;
                break;
        }
    }
}

int diritem_name_match (diritem_t * item, const char * path) {
    char buf[11];
    to_sfn(buf, path);
    return (kernel_memcmp(buf, item->DIR_Name, 11) == 0);
}

static diritem_t * read_dir_entry (fat_t * fat, int index) {
    if ((index < 0) || (index >= fat->root_end_cnt)) {
        return (diritem_t *)0;
    }

    int offset = index * sizeof(diritem_t);
    int err = bread_sector(fat, fat->root_start + offset / fat->bytes_per_sec);
    if (err < 0) {
        return (diritem_t *)0;
    }
    return (diritem_t *)(fat->fat_buffer + offset % fat->bytes_per_sec);
}

static int write_dir_entry (fat_t * fat, diritem_t * item, int index) {
    if ((index < 0) || (index >= fat->root_end_cnt)) {
        return -1;
    }

    int offset = index * sizeof(diritem_t);
    int sector = fat->root_start + offset / fat->bytes_per_sec;
    int err = bread_sector(fat, sector);
    if (err < 0) {
        return -1;
    }
    kernel_memcpy(fat->fat_buffer + offset % fat->bytes_per_sec, item, sizeof(diritem_t));
    return bwrite_secotr(fat, sector);
}

int diritem_init(diritem_t * item, uint8_t attr,const char * name) {
    to_sfn((char *)item->DIR_Name, name);
    item->DIR_FstClusHI = (uint16_t )(FAT_CLUSTER_INVALID >> 16);
    item->DIR_FstClusL0 = (uint16_t )(FAT_CLUSTER_INVALID & 0xFFFF);
    item->DIR_FileSize = 0;
    item->DIR_Attr = attr;
    item->DIR_NTRes = 0;

    // 时间写固定值，简单方便
    item->DIR_CrtTime = 0;
    item->DIR_CrtDate = 0;
    item->DIR_WrtTime = item->DIR_CrtTime;
    item->DIR_WrtDate = item->DIR_CrtDate;
    item->DIR_LastAccDate = item->DIR_CrtDate;
    return 0;
}

file_type_t diritem_get_type (diritem_t * item) {
    file_type_t type = FILE_UNKNOWN;

    // 长文件名和volum id
    if (item->DIR_Attr & (DIRITEM_ATTR_VOLUME_ID | DIRITEM_ATTR_HIDDEN | DIRITEM_ATTR_SYSTEM)) {
        return FILE_UNKNOWN;
    }

    if ((item->DIR_Attr & DIRITEM_ATTR_LONG_NAME)  == DIRITEM_ATTR_LONG_NAME) {
        return FILE_UNKNOWN;
    }

    return item->DIR_Attr & DIRITEM_ATTR_DIRECTORY ? FILE_DIR : FILE_NORMAL;
}

static void read_from_diritem (fat_t * fat, file_t * file, diritem_t * item, int index) {
    file->type = diritem_get_type(item);
    file->size = item->DIR_FileSize;
    file->pos = 0;
    file->p_index = index;
    file->sblk = (item->DIR_FstClusHI << 16) | (item->DIR_FstClusL0);
    file->cblk = file->sblk;
}

static int move_file_pos (file_t * file, fat_t * fat, uint32_t move_bytes, int expand) {
	uint32_t c_offset = file->pos % fat->cluster_byte_size;

    // 跨簇，则调整curr_cluster。注意，如果已经是最后一个簇了，则curr_cluster不会调整
	if (c_offset + move_bytes >= fat->cluster_byte_size) {
        cluster_t next = cluster_get_next(fat, file->cblk);
		if (next == FAT_CLUSTER_INVALID) {
			return -1;
		}

        file->cblk = next;
	}

	file->pos += move_bytes;
	return 0;
}


int fatfs_mount (struct _fs_t * fs, int major, int minor) {
    int dev_id = dev_open(major, minor, (void *)0);
    if (dev_id < 0) {
        log_printf("open disk failed. major: %x, minor: %x", major, minor);
        return -1;
    }

    dbr_t * dbr = (dbr_t *)memory_alloc_page();
    if (!dbr) {
        log_printf("mount failed. can't alloc buf");
        goto mount_failed;
    }

    int cnt = dev_read(dev_id, 0, (char *)dbr, 1);
    if (cnt < 1) {
        log_printf("read dbr failed.");
        goto mount_failed;
    }

    fat_t * fat = &fs->fat_data;
    fat->fat_buffer = (uint8_t *)dbr;
    fat->bytes_per_sec = dbr->BPB_BytsPerSec;
    fat->tbl_start = dbr->BPB_RsvdSecCnt;
    fat->tbl_sectors = dbr->BPB_FATSz16;
    fat->tbl_cnt = dbr->BPB_NumFATs;
    fat->root_end_cnt = dbr->BPB_RootEntCnt;
    fat->sec_per_cluster = dbr->BPB_SecPerClus;
    fat->root_start = fat->tbl_start + fat->tbl_sectors * fat->tbl_cnt;
    fat->data_start = fat->root_start + fat->root_end_cnt * 32 / SECTOR_SIZE;
    fat->cluster_byte_size = fat->sec_per_cluster * dbr->BPB_BytsPerSec;
    fat->fs = fs;
    fat->curr_sector = -1;

    if (fat->tbl_cnt != 2) {
        log_printf("fat table error: major: %x, minor: %x", major, minor);
        goto mount_failed;
    }

    if (kernel_strncmp(dbr->BS_FileSysType, "FAT16", 5) != 0) {
        log_printf("not a fat16 fs: %x, minor: %x", major, minor);
        goto mount_failed;
    }

    fs->type = FS_FAT16;
    fs->data = &fs->fat_data;
    fs->dev_id = dev_id;
    return 0;

mount_failed:
    if (dbr) {
        memory_free_page((uint32_t)dbr);
    }
    dev_close(dev_id);
    return -1;
}

void fatfs_unmount (struct _fs_t * fs) {
    fat_t * fat = (fat_t *)fs->data;

    dev_close(fs->dev_id);
    memory_free_page((uint32_t)fat->fat_buffer);
}

int fatfs_open (struct _fs_t * fs, const char * path, file_t * file) {
    fat_t * fat = (fat_t *)fs->data;
    diritem_t * file_item = (diritem_t *)0;
    int p_index = -1;

    for (int i = 0; i < fat->root_end_cnt; i ++) {
        diritem_t * item = read_dir_entry(fat, i);

        if (item == (diritem_t *)0) {
            return -1;
        }

        if (item->DIR_Name[0] == DIRITEM_NAME_END) {
            p_index = i;
            break;
        }

        if (item->DIR_Name[0] == DIRITEM_NAME_FREE) {
            p_index = i;
            continue;
        }

        if (diritem_name_match(item, path)) {
            file_item = item;
            p_index = i;
            break;
        }
    }

    if (file_item) {
        read_from_diritem(fat, file, file_item, p_index);
        return 0;
    } else if ((file->mode & O_CREAT) && (p_index >= 0)) {
        diritem_t new_item;
        diritem_init(&new_item, 0, path);

        int err = write_dir_entry(fat, &new_item, p_index);
        if (err < 0) {
            log_printf("create file failed.");
            return -1;
        }

        read_from_diritem(fat, file, &new_item, p_index);
    }


    return 0;
}

int fatfs_read (char * buf, int size, file_t * file) {
    fat_t * fat = (fat_t *)file->fs->data;

    uint32_t nbytes = size;
    if (file->pos + nbytes > file->size) {
        nbytes = file->size - file->pos;
    }

    uint32_t total_read = 0;
    while (nbytes > 0) {
        uint32_t curr_read = nbytes;
		uint32_t cluster_offset = file->pos % fat->cluster_byte_size;
        uint32_t start_sector = fat->data_start + (file->cblk - 2) * fat->sec_per_cluster;  // 从2开始

        // 如果是整簇, 只读一簇
        if ((cluster_offset == 0) && (nbytes == fat->cluster_byte_size)) {
            int err = dev_read(fat->fs->dev_id, start_sector, buf, fat->sec_per_cluster);
            if (err < 0) {
                return total_read;
            }

            curr_read = fat->cluster_byte_size;
        } else {
            // 如果跨簇，只读第一个簇内的一部分
            if (cluster_offset + curr_read > fat->cluster_byte_size) {
                curr_read = fat->cluster_byte_size - cluster_offset;
            }

            // 读取整个簇，然后从中拷贝
            fat->curr_sector = -1;
            int err = dev_read(fat->fs->dev_id, start_sector, fat->fat_buffer, fat->sec_per_cluster);
            if (err < 0) {
                return total_read;
            }
            kernel_memcpy(buf, fat->fat_buffer + cluster_offset, curr_read);
        }

        buf += curr_read;
        nbytes -= curr_read;
        total_read += curr_read;

        // 前移文件指针
		int err = move_file_pos(file, fat, curr_read, 0);
		if (err < 0) {
            return total_read;
        }
	}

    return total_read;
}

int fatfs_write (char * buf, int size, file_t * file) {
    return -1;
}

void fatfs_close (file_t * file) {

}

int fatfs_seek (file_t * file, uint32_t offset, int dir) {
    if (dir != 0) {
        return -1;
    }

    fat_t * fat = (fat_t *)file->fs->data;
    cluster_t curr_cluster = file->sblk;
    uint32_t curr_pos = 0;
    uint32_t offset_to_move = offset;

    while (offset_to_move > 0) {
        uint32_t c_off = curr_pos % fat->cluster_byte_size;
        uint32_t curr_move = offset_to_move;

        // 不超过一簇，直接调整位置，无需跑到下一簇
        if (c_off + curr_move < fat->cluster_byte_size) {
            curr_pos += curr_move;
            break;
        }

        // 超过一簇，只在当前簇内移动
        curr_move = fat->cluster_byte_size - c_off;
        curr_pos += curr_move;
        offset_to_move -= curr_move;

        // 取下一簇
        curr_cluster = cluster_get_next(fat, curr_cluster);
        if (!cluster_is_valid(curr_cluster)) {
            return -1;
        }
    }

    // 最后记录一下位置
    file->pos = curr_pos;
    file->cblk = curr_cluster;
    return 0;
}

int fatfs_stat (file_t * file, struct stat * st) {
    return -1;
}

int fatfs_opendir (struct _fs_t * fs, const char * name, DIR * dir) {
    dir->index = 0;
    return 0;
}

int fatfs_readdir  (struct _fs_t * fs, DIR * dir, struct dirent * dirent) {
    fat_t * fat = (fat_t *)fs->data;

    while (dir->index < fat->root_end_cnt) {
        diritem_t * item = read_dir_entry(fat, dir->index);
        if (item == (diritem_t *)0) {
            return -1;
        }

        if (item->DIR_Name[0] == DIRITEM_NAME_END) {
            break;
        }

        if (item->DIR_Name[0] != DIRITEM_NAME_FREE) {
            file_type_t type = diritem_get_type(item);
            if ((type == FILE_NORMAL) || (type == FILE_DIR)) {
                dirent->size = item->DIR_FileSize;
                dirent->type = type;
                diritem_get_name(item, dirent->name);
                dirent->index = dir->index++;
                return 0;
            }
        }

        dir->index++;
    }

    return -1;
}

int fatfs_closedir (struct _fs_t * fs, DIR * dir) {
    return 0;
}

int fatfs_unlink (struct _fs_t * fs, const char * path) {
    fat_t * fat = (fat_t *)fs->data;

    // 遍历根目录的数据区，找到已经存在的匹配项
    for (int i = 0; i < fat->root_end_cnt; i++) {
        diritem_t * item = read_dir_entry(fat, i);
        if (item == (diritem_t *)0) {
            return -1;
        }

         // 结束项，不需要再扫描了，同时index也不能往前走
        if (item->DIR_Name[0] == DIRITEM_NAME_END) {
            break;
        }

        // 只显示普通文件和目录，其它的不显示
        if (item->DIR_Name[0] == DIRITEM_NAME_FREE) {
            continue;
        }

        // 找到要打开的目录
        if (diritem_name_match(item, path)) {
            // 释放簇
            int cluster = (item->DIR_FstClusHI << 16) | item->DIR_FstClusL0;
            cluster_free_chain(fat, cluster);

            // 写diritem项
            diritem_t item;
            kernel_memset(&item, 0, sizeof(diritem_t));
            return write_dir_entry(fat, &item, i);
        }
    }

    return -1;
}

fs_op_t fatfs_op = {
    .mount = fatfs_mount,
    .unmount = fatfs_unmount,
    .open = fatfs_open,
    .read = fatfs_read,
    .write = fatfs_write,
    .close = fatfs_close,
    .seek = fatfs_seek,
    .stat = fatfs_stat,

    .opendir = fatfs_opendir,
    .readdir = fatfs_readdir,
    .closedir = fatfs_closedir,
    .unlink = fatfs_unlink,
};