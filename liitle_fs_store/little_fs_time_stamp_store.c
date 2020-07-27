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
}

/**
 * @brief 从little fs中提取出数据，尽可能的填充到buffer中，格式为 {timestamp,buffer}
 *
 * @param name[in] 模块名
 * @param time_stamp[in] 时间戳，精确到秒
 * @param buff[out] buffer
 * @param buff_len[int] buff长度
 */
Little_fs_time_stamp_store_result little_fs_time_stamp_store_save(char const *name, uint32_t time_stamp, uint8_t const *buff, uint32_t buff_len)
{
}
