#ifndef _MEMORYCARD_H_
#define _MEMORYCARD_H_

#define MCD_SIZE	(1024 * 8 * 16)

extern char Mcd1Data[MCD_SIZE], Mcd2Data[MCD_SIZE];

void LoadMcd(int mcd);
void LoadMcds();
void SaveMcd(int mcd, char *data, unsigned long adr, int size);
void CreateMcd(char *mcd);
void ConvertMcd(char *mcd, char *data);		// Back compatible? R-r-r

#endif
