/********************************************************
*   Copyright (C) 2016 All rights reserved.
*   
*   Filename:cam.h
*   Author  :tanw
*   Date    :2016-10-19
*   Describe:
*
********************************************************/
#ifndef _CAM_H
#define _CAM_H

#define JPG_MAX_SIZE	(128 * 1024)
struct jpg_buf_t {
	unsigned char jpg_buf[JPG_MAX_SIZE - sizeof(unsigned int)];
	unsigned int jpg_size;
};

void task_cam(void);

#endif
