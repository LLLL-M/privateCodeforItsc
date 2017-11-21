#ifndef _MD5_H
#define _MD5_H

#ifdef __cplusplus
extern "C" {
#endif

void sadp_MessageDigest(unsigned char *szInput, unsigned int inputLen, unsigned char szOutput[16], int iIteration);

#ifdef __cplusplus
}
#endif

#endif

