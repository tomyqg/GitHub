
#ifndef UTILITIES_H
#define	UTILITIES_H

#ifdef __cplusplus
extern "C" {
#endif


extern char timeString[];

extern char *formatLapTime(unsigned int, unsigned int);
extern char *formatElapsedTime(void);
extern void trim(char *s);
extern void trimn(char *s, int n);
extern float GetPredictiveScale(void);
extern void iSort(int arr[], int n);
extern void fSort(float arr[], int n);
extern int CalculateScaleTop(int rpm);
extern void EnsurePositive(char *pStr);
extern int kmc_stricmp(const char s1[], const char s2[]);


#ifdef __cplusplus
}
#endif

#endif
