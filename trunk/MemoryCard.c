#include "PsxCommon.h"
#include "MemoryCard.h"
#include <sys/stat.h>

char Mcd1Data[MCD_SIZE], Mcd2Data[MCD_SIZE];

void SeekMcd(FILE *f, u32 adr, char *str)
{
	struct stat buf;

	if (stat(str, &buf) != -1) 
	{
		if (buf.st_size == MCD_SIZE + 64)
			fseek(f, adr + 64, SEEK_SET);
		else if (buf.st_size == MCD_SIZE + 3904)
			fseek(f, adr + 3904, SEEK_SET);
		else if(adr)
			fseek(f, adr, SEEK_SET);
	} 
	else if(adr) fseek(f, adr, SEEK_SET);
}

void LoadMcd(int mcd) {
	FILE *f;
	char *data = NULL;

	if (mcd == 0) data = Mcd1Data;
	if (mcd == 1) data = Mcd2Data;

	char *str = Config.Mcd[mcd].Filename;

	if (*str == 0) sprintf(str, "sd:/wiisx/memcards/Mcd00%d.mcr", mcd+1);
	f = fopen(str, "rb");
	if (f == NULL) {
		CreateMcd(str);
		f = fopen(str, "rb");
		if (f != NULL) {
			SeekMcd(f, 0, str);
			fread(data, 1, MCD_SIZE, f);
			fclose(f);
		}
		else SysMessage(_("Failed loading MemCard %s\n"), str);
	}
	else {
		SeekMcd(f, 0, str);
		fread(data, 1, MCD_SIZE, f);
		fclose(f);
	}
}

void LoadMcds() {
	LoadMcd(0);
	LoadMcd(1);
}

void SaveMcd( int mcd, char *data, unsigned long adr, int size) {
	FILE *f;

	char *file = Config.Mcd[mcd].Filename;
	
	f = fopen(file, "r+b");
	if (f != NULL) {
		SeekMcd(f, adr, file);
		fwrite(data + adr, 1, size, f);
		fclose(f);
		return;
	}

	ConvertMcd(file, data);
}

void CreateMcd(char *mcd) {
	FILE *f;	
	struct stat buf;
	int s = MCD_SIZE;
	int i=0, j;

	f = fopen(mcd, "wb");
	if (f == NULL) return;

	if(stat(mcd, &buf)!=-1) {		
		if ((buf.st_size == MCD_SIZE + 3904) || strstr(mcd, ".gme")) {			
			s = s + 3904;
			fputc('1', f); s--;
			fputc('2', f); s--;
			fputc('3', f); s--;
			fputc('-', f); s--;
			fputc('4', f); s--;
			fputc('5', f); s--;
			fputc('6', f); s--;
			fputc('-', f); s--;
			fputc('S', f); s--;
			fputc('T', f); s--;
			fputc('D', f); s--;
			for(i=0;i<7;i++) {
				fputc(0, f); s--;
			}
			fputc(1, f); s--;
			fputc(0, f); s--;
			fputc(1, f); s--;
			fputc('M', f); s--; 
			fputc('Q', f); s--; 
			for(i=0;i<14;i++) {
				fputc(0xa0, f); s--;
			}
			fputc(0, f); s--;
			fputc(0xff, f);
			while (s-- > (MCD_SIZE+1)) fputc(0, f);
		} else if ((buf.st_size == MCD_SIZE + 64) || strstr(mcd, ".mem") || strstr(mcd, ".vgs")) {
			s = s + 64;				
			fputc('V', f); s--;
			fputc('g', f); s--;
			fputc('s', f); s--;
			fputc('M', f); s--;
			for(i=0;i<3;i++) {
				fputc(1, f); s--;
				fputc(0, f); s--;
				fputc(0, f); s--;
				fputc(0, f); s--;
			}
			fputc(0, f); s--;
			fputc(2, f);
			while (s-- > (MCD_SIZE+1)) fputc(0, f);
		}
	}
	fputc('M', f); s--;
	fputc('C', f); s--;
	while (s-- > (MCD_SIZE-127)) fputc(0, f);
	fputc(0xe, f); s--;

	for(i=0;i<15;i++) { // 15 blocks
		fputc(0xa0, f); s--;
		for(j=0;j<126;j++) {
			fputc(0x00, f); s--;
		}
		fputc(0xa0, f); s--;
	}

	while ((s--)>=0) fputc(0, f);		
	fclose(f);
}

void ConvertMcd(char *mcd, char *data) {
	FILE *f;
	int i=0;
	int s = MCD_SIZE;
	
	if (strstr(mcd, ".gme")) {		
		f = fopen(mcd, "wb");
		if (f != NULL) {		
			fwrite(data-3904, 1, MCD_SIZE+3904, f);
			fclose(f);
		}		
		f = fopen(mcd, "r+");		
		s = s + 3904;
		fputc('1', f); s--;
		fputc('2', f); s--;
		fputc('3', f); s--;
		fputc('-', f); s--;
		fputc('4', f); s--;
		fputc('5', f); s--;
		fputc('6', f); s--;
		fputc('-', f); s--;
		fputc('S', f); s--;
		fputc('T', f); s--;
		fputc('D', f); s--;
		for(i=0;i<7;i++) {
			fputc(0, f); s--;
		}		
		fputc(1, f); s--;
		fputc(0, f); s--;
		fputc(1, f); s--;
		fputc('M', f); s--;
		fputc('Q', f); s--;
		for(i=0;i<14;i++) {
			fputc(0xa0, f); s--;
		}
		fputc(0, f); s--;
		fputc(0xff, f);
		while (s-- > (MCD_SIZE+1)) fputc(0, f);
		fclose(f);
	} else if(strstr(mcd, ".mem") || strstr(mcd,".vgs")) {		
		f = fopen(mcd, "wb");
		if (f != NULL) {		
			fwrite(data-64, 1, MCD_SIZE+64, f);
			fclose(f);
		}		
		f = fopen(mcd, "r+");		
		s = s + 64;				
		fputc('V', f); s--;
		fputc('g', f); s--;
		fputc('s', f); s--;
		fputc('M', f); s--;
		for(i=0;i<3;i++) {
			fputc(1, f); s--;
			fputc(0, f); s--;
			fputc(0, f); s--;
			fputc(0, f); s--;
		}
		fputc(0, f); s--;
		fputc(2, f);
		while (s-- > (MCD_SIZE+1)) fputc(0, f);
		fclose(f);
	} else {
		f = fopen(mcd, "wb");
		if (f != NULL) {		
			fwrite(data, 1, MCD_SIZE, f);
			fclose(f);
		}
	}
}

