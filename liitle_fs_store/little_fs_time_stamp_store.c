/**
 * @file little_fs_time_stamp_store.c
 * @author James Wang (woodsking2@hotmail.com)
 * @brief
 * @version 0.1
 * @date 2020-07-27
 *
 * Copyright (c) 2020 Walnutin Technology Company Limited. All Rights Reserved.
 *
 */
#include "little_fs_time_stamp_store.h"
#include "lfs.h"
#include "lfs_util.h"
#include <tchar.h>

#define INVAILD_FILE 0
#define CURRENT_PATH "New_document.txt"
#define BUFF_SIZE 255

lfs_t lfs;
lfs_file_t file;
lfs_dir_t dir;

int user_provided_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int user_provided_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int user_provided_block_device_erase(const struct lfs_config *c, lfs_block_t block);
int user_provided_block_device_sync();
bool is_binary_exist(void);
int create_file(const struct lfs_config *c);

int compar(const void *p1, const void *p2);
// int readfile(char *const name, uint32_t time_stamp, uint8_t const *buff);
int writefile(char const *name, uint32_t time_stamp, uint8_t const *buff, uint32_t buff_len);

const struct lfs_config cfg = {
    // block device operations
    .read = user_provided_block_device_read,
    .prog = user_provided_block_device_prog,
    .erase = user_provided_block_device_erase,
    .sync = user_provided_block_device_sync,

    // block device configuration
    .read_size = 16,
    .prog_size = 16,
    .block_size = 4096,
    .block_count = 128,
    .cache_size = 16,
    .lookahead_size = 16,
    .block_cycles = 500,
};

/**
 * @brief 将指定buffer通过存储到little fs中
 *
 * @param name[in] 模块名
 * @param time_stamp [in] 时间戳，精确到秒
 * @param buff[in] 要存储的buffer指针
 * @param buff_len[in] 要存储的buff长度
 */
Little_fs_time_stamp_store_result little_fs_time_stamp_store_save(char const *name, uint32_t time_stamp, uint8_t const *buff, uint32_t buff_len)
{
    Little_fs_time_stamp_store_result result = Little_fs_time_stamp_store_result_success;
    int err = lfs_mount(&lfs, &cfg);

    if (err)
    {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    char write[64] = {0};
    sprintf(write, "%s+%u", name, time_stamp);
    printf("%s\n", write);
    err = lfs_file_open(&lfs, &file, write, LFS_O_RDWR | LFS_O_CREAT);
    if (err < 0)
    {
        printf("error:Failed to open the file%d\n", err);
        result = Little_fs_time_stamp_store_result_unknown_error;
        goto RETURNED;
    }

    lfs_file_seek(&lfs, &file, 0, LFS_SEEK_END);

    err = lfs_file_write(&lfs, &file, buff, buff_len);
    if (err < 0)
    {
        printf("error:Failed to write the file%d\n", err);

        result = Little_fs_time_stamp_store_result_unknown_error;
    }

RETURNED:

    lfs_file_close(&lfs, &file);

    lfs_unmount(&lfs);

    return result;
}

/**
 * @brief 从little fs中提取出数据，尽可能的填充到buffer中，格式为 {timestamp,buffer}
 *
 * @param name[in] 模块名
 * @param time_stamp[in] 时间戳，精确到秒
 * @param buff[out] buffer
 * @param buff_len[int] buff长度
 */
Little_fs_time_stamp_store_result little_fs_time_stamp_store_read(char const *name, uint32_t time_stamp, uint8_t const *buff, uint32_t buff_len)
{
    Little_fs_time_stamp_store_result result = Little_fs_time_stamp_store_result_success;

    int err = lfs_mount(&lfs, &cfg);
    if (err)
    {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    char read[64] = {0};
    sprintf(read, "%s+%u", name, time_stamp);
    printf("%s\n\n", read);
    err = lfs_file_open(&lfs, &file, read, LFS_O_RDWR);
    if (err < 0)
    {
        printf("error:Failed to open the file%d\n", err);
        result = Little_fs_time_stamp_store_result_unknown_error;
        goto RETURNED;
    }

    lfs_file_seek(&lfs, &file, 0, LFS_SEEK_SET);

    int count = lfs_file_read(&lfs, &file, buff, buff_len);
    if (count < 0)
    {
        printf("error:Failed to read the file%d\n", err);
        result = Little_fs_time_stamp_store_result_unknown_error;
    }

RETURNED:
    lfs_file_close(&lfs, &file);

    lfs_unmount(&lfs);

    return result;
}

Little_fs_time_stamp_store_result little_fs_time_stamp_store_load(char const *name, uint32_t start_time, uint8_t const *buff, uint32_t buff_len)
{
    memset(buff, 0, buff_len);
    struct lfs_info li = {0};

    Little_fs_time_stamp_store_result result = Little_fs_time_stamp_store_result_success;

    int err = lfs_dir_open(&lfs, &dir, "");
    if (err)
    {
        printf("error:failed to open the dir%d", err);
        return Little_fs_time_stamp_store_result_unknown_error;
    }

    uint32_t time_stamp_count[30] = {0};
    int count = 0;

    do
    {
        err = lfs_dir_read(&lfs, &dir, &li);
        if (err < 0)
        {
            printf("error:failed to read the dir%d", err);
            result = Little_fs_time_stamp_store_result_unknown_error;
            goto RETURNED;
        }

        if (strlen(li.name) != 0 && li.size == 0 || strlen(li.name) && li.size == 0)
        {
            continue;
        }
        else if (strlen(li.name) != 0 && li.size != 0)
        {
            int len = strlen(li.name);
            if (strncmp(name, li.name, strlen(name)) == 0)
            {

                for (int i = 0; i < len; i++)
                {

                    if (li.name[i] == '+')
                    {
                        int time_stamp = 0;
                        sscanf(li.name + (i + 1), "%d", &time_stamp);
                        if (count >= 29)
                        {
                            qsort(time_stamp_count, 30, sizeof(int), compar);
                            if (time_stamp_count[0] > time_stamp && start_time < time_stamp)
                            {
                                time_stamp_count[count] = time_stamp;
                            }
                            continue;
                        }
                        if (start_time < time_stamp)
                        {
                            time_stamp_count[count] = time_stamp;
                            count++;
                        }
                    }
                }
            }
        }
        printf(li.name);
        printf(" %d\n", li.size);
    } while (err != 0);

    for (int i = 0; i <= 29; i++)
    {
        printf("%d\n", time_stamp_count[i]);
    }

    int buffer_len = 0;
    int len = 1024;
    for (int i = 0; i < 30; i++)
    {
        if (time_stamp_count[i] == 0)
        {
            continue;
        }

        buffer_len += readfile(name, time_stamp_count[i], buff + buffer_len, len - 1);
        len = 1024 - buffer_len;
        if (buffer_len == 1023 || buffer_len < 0)
        {
            printf("error:The file does not exist or the file is empty");
            break;
        }
    }

    printf("%s", buff);

RETURNED:

    lfs_dir_close(&lfs, &dir);

    return result;
}

int compar(const void *p1, const void *p2)
{
    return (*(int *)p2 - *(int *)p1);
}

int readfile(char *const name, uint32_t time_stamp, uint8_t const *buff, uint32_t buff_len)
{
    int result = 0;
    int err = lfs_mount(&lfs, &cfg);
    if (err)
    {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    char read[64] = {0};
    sprintf(read, "%s+%u", name, time_stamp);

    int len = strlen(read);
    strncpy(buff, read, len);
    buff_len -= len;

    err = lfs_file_open(&lfs, &file, read, LFS_O_RDWR);
    if (err < 0)
    {
        printf("error:failed to open the file%d", err);
        return -1;
    }

    lfs_file_seek(&lfs, &file, 0, LFS_SEEK_END);
    int max_size = lfs_file_tell(&lfs, &file);
    lfs_file_seek(&lfs, &file, 0, LFS_SEEK_SET);

    err = lfs_file_read(&lfs, &file, buff + len, buff_len);
    if (err < 0)
    {
        printf("error:failed to open the file%d", err);
        result = -1;
    }

    result = err + len;

    lfs_file_close(&lfs, &file);
    lfs_unmount(&lfs);

    return result;
}

int writefile(char const *name, uint32_t time_stamp, uint8_t const *buff, uint32_t buff_len)
{
    int result = 0;
    int err = lfs_mount(&lfs, &cfg);

    if (err)
    {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    err = lfs_file_open(&lfs, &file, name, LFS_O_RDWR | LFS_O_CREAT);
    if (err < 0)
    {
        printf("error:Failed to open the file%d\n", err);
        result = -1;
        goto RETURNED;
    }

    lfs_file_seek(&lfs, &file, 0, LFS_SEEK_END);
    char write[64] = {0};
    sprintf(write, "%s+%u", name, time_stamp);
    err = lfs_file_write(&lfs, &file, write, buff_len);
    if (err < 0)
    {
        printf("error:Failed to write the file%d\n", err);

        result = -1;
    }

    lfs_file_seek(&lfs, &file, 0, LFS_SEEK_END);

    err = lfs_file_write(&lfs, &file, buff, buff_len);
    if (err < 0)
    {
        printf("error:Failed to write the file%d\n", err);

        result = -1;
    }

RETURNED:

    lfs_file_close(&lfs, &file);

    lfs_unmount(&lfs);

    return result;
}

//读文件
int user_provided_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    int result = 0;

    if (!is_binary_exist())
    {
        create_file(c);
        memset(buffer, 0xff, size);
        return 0;
    }

    FILE *const file = fopen(CURRENT_PATH, "rb+"); //"rb+"打开一个二进制文件，文件必须存在，允许读写

    fseek(file, c->block_size * block + off, SEEK_SET); //偏移量设c->block_size*c->block_count

    uint32_t count = fread(buffer, 1, size, file); //从c->block_size*c->block_count的偏移量读取c->block_size*c->block_count字节
    if (count < size)
    {
        printf("error:Failed to read file");
        result = -1;
    }

    fclose(file);

    return result;
}

int user_provided_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{

    int result = 0;

    if (!is_binary_exist())
    {
        create_file(c);
    }

    FILE *const file = fopen(CURRENT_PATH, "rb+"); // ab+ 打开或新建一个二进制文件，可以读，但只允许在文件末尾追写

    fseek(file, c->block_size * block + off, SEEK_SET); //偏移量设c->block_size*c->block_count
    uint32_t count = fwrite(buffer, 1, size, file);     //从c->block_size*c->block_count的偏移量读取c->block_size*c->block_count字节
    if (count < size)
    {
        printf("error:Failed to read file");
        result = -1;
    }

    fclose(file);
    if (result == 0)
    {
        uint8_t temp_buffer[4096] = {0};
        int read_result = user_provided_block_device_read(c, block, off, temp_buffer, size);
        if (read_result == 0)
        {
            int cmp_result = memcmp(temp_buffer, buffer, size);
            if (cmp_result == 0)
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }
    return result;
}

bool is_binary_exist(void)
{
    FILE *file = fopen(CURRENT_PATH, "rb+");
    if (INVAILD_FILE == file)
    {
        return false;
    }

    fclose(file);
    return true;
}

int create_file(const struct lfs_config *c)
{
    int result = 0;
    FILE *const file = fopen(CURRENT_PATH, "wb+");
    if (INVAILD_FILE == file)
    {
        printf("error::Create file to failed");
        return -1;
    }

    uint8_t const erase_char = 0xff;
    int write_result = 0;
    for (uint32_t i = 0; i < c->block_count * c->block_size; i++)
    {
        write_result = fwrite(&erase_char, 1, 1, file);
        if (write_result < 0)
        {
            goto RETURNED;
            result - 1;
        }
    }

RETURNED:

    fclose(file);
    return result;
}

int user_provided_block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
    int result = 0;

    if (!is_binary_exist())
    {
        create_file(c);
        return 0;
    }

    FILE *const file = fopen(CURRENT_PATH, "rb+");
    if (INVAILD_FILE == file)
    {
        printf("error:Failed to read file");
        return -1;
    }

    fseek(file, c->block_size * block, SEEK_SET);

    char *buffer = (char *)malloc(c->block_size);
    if (buffer == NULL)
    {
        printf("error:malloc memory to failed");
        fclose(file);
        return -1;
    }
    memset(buffer, 0xff, c->block_size);

    int count = fwrite(buffer, 1, c->block_size, file);
    if (count <= 0)
    {
        printf("error:Failed to frwite file");
        result = -1;
    }

    free(buffer);
    fclose(file);

    return result;
}

int user_provided_block_device_sync()
{
    return 0;
}
