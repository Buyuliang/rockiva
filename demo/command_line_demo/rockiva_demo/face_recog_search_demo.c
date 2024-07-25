/****************************************************************************
 *
 *    Copyright (c) 2022 by Rockchip Corp.  All rights reserved.
 *
 *    The material in this file is confidential and contains trade secrets
 *    of Rockchip Corporation. This is proprietary information owned by
 *    Rockchip Corporation. No part of this work may be disclosed,
 *    reproduced, copied, transmitted, or used in any way for any purpose,
 *    without the express written permission of Rockchip Corporation.
 *
 *****************************************************************************/
#include <dirent.h>
#include <linux/limits.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "rockiva_face_api.h"

#include "iva_app_ctx.h"
#include "utils/face_db.h"
#include "utils/common_utils.h"

#define FACE_LIBRARY_NAME "face"

int load_face_lib(const char *face_db_path, const char *face_lib_name) {
    int ret;
    sqlite3* db;
    open_db(face_db_path, &db);

    int face_num_;

    ret = get_face_count(db, &face_num_);
    face_data face_lib_[face_num_];

    get_all_face(db, face_lib_, &face_num_);

    RockIvaFaceIdInfo face_info[face_num_];
    int feature_size = face_lib_->size;
    printf("feature_size=%d\n", feature_size);
    void *feature_data = (void *)malloc(face_num_*feature_size);
    for (int i = 0; i < face_num_; i++) {
        strncpy(face_info[i].faceIdInfo, face_lib_[i].info, ROCKIVA_FACE_INFO_SIZE_MAX);
        memcpy(feature_data+i*feature_size, face_lib_[i].feature, feature_size);
    }

    release_face_data(face_lib_, face_num_);

    ROCKIVA_FACE_FeatureLibraryControl(face_lib_name, ROCKIVA_FACE_FEATURE_INSERT, face_info, face_num_, feature_data, feature_size);

    close_db(db);
    free(feature_data);

    printf("load face num %d\n", face_num_);

    return 0;
}

int main(int argc, char** argv)
{
    char* recog_feature_path = argv[1];

    printf("recog_feature_path: %s\n", recog_feature_path);

    RockIvaRetCode ret;

    load_face_lib(FACE_DATABASE_PATH, FACE_LIBRARY_NAME);

    char* feature;
    int feature_size;
    feature_size = ReadDataFile(recog_feature_path, &feature);
    if (feature_size <= 0) {
        printf("read feature size error\n");
        return -1;
    }

    printf("read feature size: %d\n", feature_size);

    RockIvaFaceSearchResults results;
    ROCKIVA_FACE_SearchFeature(FACE_LIBRARY_NAME, feature, feature_size, 1, 5, &results);
    for (int i = 0; i < results.num; i++) {
        printf("serch result %d face_info=%s score=%f\n", i, results.faceIdScore[i].faceIdInfo, results.faceIdScore[i].score);
    }

    printf("iva app exit\n");
    return 0;
}
