/* Common Header */

#ifndef UTILS_H
#define UTILS_H

int ilog(int n);
void zeroWords(void *buff, int nWords);
void zeroBytes(void *buff, int nBytes);
char *itox(UINT32 x);
char *dsp_strcat(char *str1, char *str2);
char *dsp_strcpy(char *str1, char *str2);
void echoRegister(char *title, void *regAddr);
void readBirthday(void);
void delay(int interval);
UINT32 delta_t(UINT32 t0);
INT32 copyMem(void *src, void *dst, UINT32 len);

#endif
