/********************************************************
*   Copyright (C) 2016 All rights reserved.
*   
*   Filename:cam.c
*   Author  :tanw
*   Date    :2016-10-19
*   Describe:
*
********************************************************/
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ipc.h"
#include "cam.h"
#include "camera.h"
#include "convert.h"

#define CAMERA_DEV_PATH	"/dev/video0"

extern struct jpg_buf_t *jpg;

void task_cam(void)
{
	int i;
	int fd;
	int ret;
	unsigned int width;
	unsigned int height;
	unsigned int size;
	unsigned int index;
	unsigned int ismjpeg;
	char *yuv;
	char *rgb;

	/* A8的屏幕比较小，所以设了较低的像素 */
	width = 1024;
	height = 1024;
	fd = camera_init(CAMERA_DEV_PATH, &width, &height, &size, &ismjpeg);
	if (fd == -1)
		exit(EXIT_FAILURE);
	printf("width: %d\n", width);
	printf("height: %d\n", height);

	ret = camera_start(fd);
	if (ret == -1)
		exit(EXIT_FAILURE);

	if (!ismjpeg) {
		rgb = malloc(width * height * 3);
		convert_rgb_to_jpg_init();
	}

	/* 采集几张图片丢弃 */
	for (i = 0; i < 8; i++) {
		ret = camera_dqbuf(fd, (void **)&yuv, &size, &index);
		if (ret == -1)
			exit(EXIT_FAILURE);

		ret = camera_eqbuf(fd, index);
		if (ret == -1)
			exit(EXIT_FAILURE);
	}

	fprintf(stdout, "init camera success\n");

	sem_down(semid);
	*sync_flag -= 1;
	sem_up(semid);

	/* 循环采集图片 */
	while (1) {
		//
		ret = camera_dqbuf(fd, (void **)&yuv, &size, &index);
		if (ret == -1)
			exit(EXIT_FAILURE);

		if (ismjpeg) {
			sem_down(semid);
			memcpy(jpg->jpg_buf, yuv, size);
			jpg->jpg_size = size;
			sem_up(semid);
		} else {
			convert_yuv_to_rgb(yuv, rgb, width, height, 24);
			sem_down(semid);
			jpg->jpg_size = convert_rgb_to_jpg_work(rgb, jpg->jpg_buf, width, height, 24, 80);
			sem_up(semid);
		}

		ret = camera_eqbuf(fd, index);
		if (ret == -1)
			exit(EXIT_FAILURE);
	}

	/* 代码不应该运行到这里 */
	if (!ismjpeg) {
		convert_rgb_to_jpg_exit();
		free(rgb);
	}
	free(jpg);

	ret = camera_stop(fd);
	if (ret == -1)
		exit(EXIT_FAILURE);

	ret = camera_exit(fd);
	if (ret == -1)
		exit(EXIT_FAILURE);

}
