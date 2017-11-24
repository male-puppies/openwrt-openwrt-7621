/*
 Copyright (c) 2004 Qualcomm Atheros, Inc..
 All Rights Reserved.
 Qualcomm Atheros Confidential and Proprietary.
 */
 
#ifndef _CRC_H_
#define _CRC_H_

extern char computeCrc8(char *buf, char len);
extern unsigned char computeCrc8_OtpV8(unsigned char *buf, unsigned char len);
extern unsigned short computeChecksumOnly(unsigned short *pHalf, unsigned short length);

#endif  //_CRC_H_
