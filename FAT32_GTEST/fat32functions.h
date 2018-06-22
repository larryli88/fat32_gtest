#pragma once
/*
* fucntions in fat32.c
*/
DWORD ReadSector(unsigned char out[512], DWORD sector_num, DWORD size);
DWORD ReadCluster(unsigned char out[16384], DWORD sector_num, DWORD size);
static bool WriteSector(const char *path, int sector_num, const char *data);
void printBootSectorInfo();
void printSectorHex(unsigned char a[], int sector_size);
sector getRootDir();
dirEntry readDirEntry(sector sect, DWORD size, int offset);
sector getSubDirSect(dirEntry dir);
void printDirEntryName(dirEntry dir);
void printDirEntryName2(dirEntry dir);
dirItem_t newDirItem();
void printDirEntryInfo(dirEntry dir);
void printDirectory(sector sect);
fileEntry_t* setDirHashTable(sector sect, fileEntry_t* hashTable[], filePath_t* parentFolder);
dirItem_t setDirItem(sector sect);
char setSearchName(dirEntry dir);
dirPrint_t newDirPrint();
int searchHashTable(fileEntry_t* hashTable[], unsigned char name[12]);
bool searchDirItem(dirItem_t dirItem, unsigned char name[12]);
bool deleteDirEntry(sector sect, int offset);
bool createDirEntry(sector sect, int offset, unsigned char name[8], unsigned char ext[3]);
void recoverDirEntry(const char* path);