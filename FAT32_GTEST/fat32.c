#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <WinBase.h>

#include "fat32.h"
#include "fat32functions.h"
#pragma warning(disable : 4996)

/* this function read a designated sector
* Input: char out				store the sector info
DWORD sector_num		the sector that the function reads
DWORD size			the size of a sector
* Return value: length
*
*/
DWORD ReadSector(unsigned char out[512], DWORD sector_num, DWORD size) {
	DWORD start = sector_num * size;
	OVERLAPPED over = { 0 };
	over.Offset = start;
	DWORD readsize;

	HANDLE handle = CreateFile(TEXT("\\\\.\\D:"), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		printf("invalid handle value");
		return 0;
	}

	if (ReadFile(handle, out, size, &readsize, &over) == 0)
	{
		CloseHandle(handle);
		printf("ReadFile error");
		return 0;
	}
	CloseHandle(handle);

	return size;
}
/*
 * same function but for a cluster
 */
DWORD ReadCluster(unsigned char out[16384], DWORD sector_num, DWORD size) {
	DWORD start = sector_num * size;
	OVERLAPPED over = { 0 };
	over.Offset = start;
	unsigned char buffer[16384] = { 0 };
	DWORD readsize;

	HANDLE handle = CreateFile(TEXT("\\\\.\\D:"), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		printf("invalid handle value");
		return 0;
	}
	if (ReadFile(handle, buffer, 16384, &readsize, &over) == 0)
	{
		CloseHandle(handle);
		printf("ReadFile error");
		return 0;
	}
	CloseHandle(handle);

	for (int i = 0; i < 16384; i++) {
		out[i] = buffer[i];
	}
	return size;
}
/* Can only write raw data to disk when it's locked...
* Input: char *path		the path of HANDLE
int sector_num	write raw data to this sector
char *data		write data[] to the sector
* Return: bool result
*/
static bool WriteSector(const char *path, int sector_num, const char *data)
{
	DWORD size = 512;
	DWORD start = sector_num * size;
	OVERLAPPED over = { 0 };

	over.Offset = start;

	bool result = false;

	HANDLE hFile = CreateFile(path,
		GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("invalid handle value");
		return false;
	}
	else {
		LPDWORD lpBytesReturned = 0;
		DeviceIoControl(
			hFile,            // handle to a volume

			FSCTL_LOCK_VOLUME,   // dwIoControlCode
			NULL,                        // lpInBuffer
			0,                           // nInBufferSize
			NULL,                        // lpOutBuffer
			0,                           // nOutBufferSize
			lpBytesReturned,   // number of bytes returned
			NULL // OVERLAPPED structure
		);
		DWORD bytesWritten;
		if (WriteFile(hFile, data, 512, &bytesWritten, &over))
		{
			printf("Write file success!\n");
			DeviceIoControl(
				hFile,            // handle to a volume
				FSCTL_UNLOCK_VOLUME,         // dwIoControlCode
				NULL,                        // lpInBuffer
				0,                           // nInBufferSize
				NULL,                        // lpOutBuffer
				0,                           // nOutBufferSize
				lpBytesReturned,   // number of bytes returned
				NULL  // OVERLAPPED structure
			);
			CloseHandle(hFile);
			result = true;
		}
		else {
			DWORD dwError = GetLastError();
			printf("write file error\n");
			printf("Error code: %x\n", dwError);
			CloseHandle(hFile);
			return false;
		}


	}

	return result;
}
//this function print the boot sector info
void printBootSectorInfo(){
	unsigned char s[512] = { 0 };
	DWORD len = ReadSector(s, 0, 512);
	fat32_bootsector *bpb = (fat32_bootsector *)s;

	char oem[9];
	char volumeLabel[12];
	char fileSystem[9];
	memcpy(oem, bpb->oem_name, sizeof(bpb->oem_name));
	memcpy(volumeLabel, bpb->volume_label, sizeof(bpb->volume_label));
	memcpy(fileSystem, bpb->fat_type_label, sizeof(bpb->fat_type_label));
	oem[8] = '\0';
	volumeLabel[11] = '\0';
	fileSystem[8] = '\0';

	printf("JMP instruction       		|  %02X %02X %02X\n", bpb->bootjmp[0], bpb->bootjmp[1], bpb->bootjmp[2]);
	printf("OEM                   		|  %s\n", oem);
	printf("Bytes per sector      		|  %d\n", bpb->bytes_per_sector);
	printf("Sectors per cluster   		|  %d\n", bpb->sectors_per_cluster);
	printf("Reserved sectors      		|  %d\n", bpb->reserved_sector_count);
	printf("Number of FATs       	 	|  %d\n", bpb->fat_num);
	printf("Root entries (unused)	 	|  %d\n", bpb->root_entry_count);
	printf("Sectors (on small volumes) 	|  %d\n", bpb->total_sectors_16);
	printf("Media descriptor (hex) 		|  %02X\n", bpb->media_type);
	printf("Sectors per FAT (small vol.)	|  %d\n", bpb->table_size_16);
	printf("Sectors per track 		|  %d\n", bpb->sectors_per_track);
	printf("Heads 				|  %d\n", bpb->head_side_count);
	printf("Hidden sectors 			|  %d\n", bpb->hidden_sector_count);
	printf("Sectors (on large volumes) 	|  %d\n", bpb->total_sectors_32);
	printf("FAT32 Section\n");
	printf("Sectors per FAT 		|  %d\n", bpb->table_size_32);
	printf("Extended flags 			|  %d\n", bpb->extended_flags);
	printf("Version (usually 0) 		|  %d\n", bpb->fat_version);
	printf("Root dir 1st cluster 		|  %d\n", bpb->root_cluster);
	printf("FSInfo sector 			|  %d\n", bpb->fat_info);
	printf("Backup boot sector 		|  %d\n", bpb->backup_BS_sector);
	printf("(Reserved) 			|  %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", bpb->reserved_0[0], bpb->reserved_0[1], bpb->reserved_0[2], bpb->reserved_0[3], bpb->reserved_0[4], bpb->reserved_0[5], bpb->reserved_0[6], bpb->reserved_0[7], bpb->reserved_0[8], bpb->reserved_0[9], bpb->reserved_0[10], bpb->reserved_0[11]);
	printf("BIOS drive (hex, HD=8x) 	|  %02X\n", bpb->drive_number);
	printf("(Unused) 			|  %d\n", bpb->reserved_1);
	printf("Ext. boot signature (29h) 	|  %02X\n", bpb->boot_signature);
	printf("Volume serial number (decimal) 	|  %u\n", bpb->volume_id);
	printf("Volume label	 		|  %s\n", volumeLabel);
	printf("File system	 		|  %s\n\n", fileSystem);
}
// this function print the sector in HEX
void printSectorHex(unsigned char a[], int sector_size) {
	for (int i = 0; i < sector_size; i++) {
		printf("%02X ", a[i]);
	}
}
/* this function get the root directory
* and returns a sector struct
*/
sector getRootDir() {
	sector sect;
	unsigned char s[512] = { 0 };
	DWORD len = ReadSector(s, 0, 512);
	fat32_bootsector *bpb = (fat32_bootsector *)s;
	sect.sectorNum = (bpb->table_size_32) * (bpb->fat_num) + bpb->reserved_sector_count;
	return sect;
}
/* this function read a dirEntry and returns it as a dirEntry struct
* Input: sector sect			the sector
DWORD size			the size of a dir entry
int offset			the number of the dir entry in this sector
* Return value: dirEntry directory
*
*/
// size should be 32
dirEntry readDirEntry(sector sect, DWORD size, int offset) {
	unsigned char out[512] = { 0 };

	DWORD len = ReadSector(out, sect.sectorNum, 512);
	dirEntry* dir = (dirEntry *)(out + offset * size * sizeof(out[0]));
	dirEntry directory = *dir;
	//printf("Name:		%s\n", directory->name);
	//getchar();
	return directory;
}
// this function returns the sector num of the subdirectory
sector getSubDirSect(dirEntry dir) {
	cluster c;
	sector s;

	// read the bpb
	unsigned char buffer[512] = { 0 };
	DWORD len = ReadSector(buffer, 0, 512);
	fat32_bootsector *bpb = (fat32_bootsector *)buffer;

	s = getRootDir();
	c.clusterNum = dir.clusterIndexLo + (dir.clusterIndexHi) * 16 * 16 * 16 * 16;
	c.sector.sectorNum = (c.clusterNum - 2)*(bpb->sectors_per_cluster) + s.sectorNum;
	return c.sector;
}
// this function prints the direntry'name on screen
void printDirEntryName(dirEntry dir) {
	char name[9];
	char extension[4];
	memcpy(name, dir.name, sizeof(dir.name));
	memcpy(extension, dir.ext, sizeof(dir.ext));
	for (int i = 0; i < 9; i++) {
		if (name[i] == 0x20) {
			name[i] = '\0';
			break;
		}
	}
	name[8] = '\0';
	for (int j = 0; j < 4; j++) {
		if (extension[j] == 0x20) {
			extension[j] = '\0';
			break;
		}
	}
	extension[3] = '\0';
	if (extension[0] == '\0') {
		printf("%s\n", name);
	}
	else {
		printf("%s.%s\n", name, extension);
	}
}
// another version used in printing search result
void printDirEntryName2(dirEntry dir) {
	char name[9];
	char extension[4];
	memcpy(name, dir.name, sizeof(dir.name));
	memcpy(extension, dir.ext, sizeof(dir.ext));
	for (int i = 0; i < 9; i++) {
		if (name[i] == 0x20) {
			name[i] = '\0';
			break;
		}
	}
	name[8] = '\0';
	for (int j = 0; j < 4; j++) {
		if (extension[j] == 0x20) {
			extension[j] = '\0';
			break;
		}
	}
	extension[3] = '\0';
	if (extension[0] == '\0') {
		printf("%s", name);
	}
	else {
		printf("%s.%s", name, extension);
	}
}
// constructor of dirItem_t
dirItem_t newDirItem() {
	dirItem_t dir;
	dir.dirListHead = (dirList_t*)malloc(sizeof(dirList_t));
	dir.fileListHead = (fileList_t*)malloc(sizeof(fileList_t));
	dir.fileListHead->next = NULL;
	dir.dirListHead->next = NULL;
	return dir;
}
// this function prints the dir entry's info
void printDirEntryInfo(dirEntry dir) {
	char name[9];
	char extension[4];
	memcpy(name, dir.name, sizeof(dir.name));
	memcpy(extension, dir.ext, sizeof(dir.ext));
	name[8] = '\0';
	extension[3] = '\0';

	printf("Name:			%s\n", name);
	printf("Extension:  		%s\n", extension);
	printf("Attributes: 		0x%02X\n", dir.attribs);
	printf("Reserved:   		%d\n", dir.reserved);
	printf("Create Time:	\n");
	printf("Cluster number (high):	%d\n", dir.clusterIndexHi);
	printf("Cluster number (low): 	%d\n", dir.clusterIndexLo);
	printf("Cluster number:		%d\n", dir.clusterIndexLo + (dir.clusterIndexHi) * 16 * 16 * 16 * 16);
	printf("File size (bytes): 	%d\n\n", dir.fileSize);
}
// this function prints the content inside a directory
static int depth = 0;
static char longname[100];
void printDirectory(sector sect) {

	//sector rootDir = getRootDir();

	bool end = false;
	while (end == false) {
		for (int offset = 0; offset < 16; offset++) {

			dirEntry dir = readDirEntry(sect, 32, offset);
			if ((dir.name[0] != 0xE5) && (dir.name[0] != 0x2E)) {
				if (dir.name[0] == 0x00) {
					depth--;
					end = true;
					break;
				}
				else {
					if (dir.attribs == 0x0F) {

					}
					else if (dir.attribs == 0x08) {
						printf("Volume ID: ");
						printDirEntryName(dir);
					}
					// if the dir entry is a directory
					else if (dir.attribs == 0x10) {
						for (int i = depth; i > 0; i--) {
							printf("   ");
						}
						printf("|--");
						//printf("Depth: %d\t", depth);
						// if the file has a long name
						//if ((dir.name[6] == '~') && (dir.name[7] == '1')) {

						//}
						//else {
						printDirEntryName(dir);
						//}

						// recurse to the sub directory
						depth++;
						printDirectory(getSubDirSect(dir));
					}
					// if the dir entry is a file
					else {
						for (int i = depth; i > 0; i--) {
							printf("   ");
						}
						printf(" --");
						// if the file has a long name
						//if ((dir.name[6] == '~') && (dir.name[7] == '1')) {
						//	
						//}
						//else {
						printDirEntryName(dir);
						//}
					}
				}
			}
		}
		sect.sectorNum++;
	}
}
/*
* hashTable is an array of fileEntry_T
*/
static int itemcount1 = 0;
fileEntry_t* setDirHashTable(sector sect, fileEntry_t* hashTable[], filePath_t* parentFolder) {

	bool end = false;
	int stringInt = 0;
	int hashIndex;


	while (end == false) {

		unsigned char out[16384] = { 0 };
		DWORD len = ReadCluster(out, sect.sectorNum, 512);
		for (int offset = 0; offset < 512; offset++) {
			dirEntry* dirct = (dirEntry *)(out + offset * 32 * sizeof(out[0]));
			dirEntry directory = *dirct;
			dirEntry dir = directory;

			if ((dir.name[0] != 0xE5) && (dir.name[0] != 0x2E)) {

				// if it is not a erased entry && not a . entry
				if (dir.name[0] == 0x00) {
					// if it is a unused entry
					// if reaches the end of the directory
					end = true;
					break;
				}
				else {

					if (dir.attribs == 0x0F) {
						// if this is a long file name
					}
					else if (dir.attribs == 0x08) {
						// if this is a volume id
						filePath_t* newPath = (filePath_t*)malloc(sizeof(filePath_t));
						newPath->nextPath = parentFolder;
						strcpy(newPath->path, dir.name);


						parentFolder = newPath;
					}
					else if (dir.attribs == 0x10) {
						// if this is a folder
						itemcount1++;
						// recurse to the sub directory
						filePath_t* newPath = (filePath_t*)malloc(sizeof(fileEntry_t));
						newPath->nextPath = parentFolder;
						strcpy(newPath->path, dir.name);


						hashTable[101] = setDirHashTable(getSubDirSect(dir), hashTable, newPath);
					}
					else {
						// if this is a file
						itemcount1++;
						fileEntry_t* newFile = (fileEntry_t*)malloc(sizeof(fileEntry_t));
						filePath_t* newPath = (filePath_t*)malloc(sizeof(fileEntry_t));
						newPath->nextPath = parentFolder;
						strcpy(newPath->path, dir.name);

						newFile->fileInfo = dir;
						newFile->filePath = newPath;
						stringInt = 0;
						for (int i = 0; i < 8; i++) {
							stringInt += (int)(dir.name[i]);
						}

						hashIndex = stringInt % 101;


						newFile->nextFile = hashTable[hashIndex];
						hashTable[hashIndex] = newFile;


					}

				}
			}
		}
		sect.sectorNum = sect.sectorNum + 32;
	}
	return *hashTable;
}
/*
* dirItem_t stores the directory infomation in linked list...
*/
static int itemcount2 = 0;
dirItem_t setDirItem(sector sect) {
	bool end = false;
	dirItem_t dirItem = newDirItem();
	while (end == false) {
		for (int offset = 0; offset < 16; offset++) {
			dirEntry dir = readDirEntry(sect, 32, offset);
			if ((dir.name[0] != 0xE5) && (dir.name[0] != 0x2E)) {
				// if it is not a erased entry && not a . entry
				if (dir.name[0] == 0x00) {
					// if it is a unused entry
					// if reaches the end of the directory
					end = true;
					break;
				}
				else {
					if (dir.attribs == 0x0F) {
						// if this is a long file name
					}
					else if (dir.attribs == 0x08) {
						// if this is a volume id
					}
					else if (dir.attribs == 0x10) {
						// if this is a folder
						itemcount2++;
						dirList_t* nextDir = (dirList_t*)malloc(sizeof(dirList_t));
						nextDir->next = dirItem.dirListHead;
						nextDir->directory = dir;
						// recurse to the sub directory
						nextDir->subDir = setDirItem(getSubDirSect(dir));
						dirItem.dirListHead = nextDir;
					}
					else {
						// if this is a file
						itemcount2++;
						fileList_t* nextFile = (fileList_t*)malloc(sizeof(fileList_t));
						nextFile->next = dirItem.fileListHead;
						nextFile->directory = dir;
						dirItem.fileListHead = nextFile;
					}
				}
			}
		}
		sect.sectorNum++;
	}
	return dirItem;
}
// this function returns the name used to search a direntry
char setSearchName(dirEntry dir) {

	char searchName[11];

	for (int i = 0; i < 11; i++) {
		if (i < 8) {
			searchName[i] = dir.name[i];
		}
		else {
			searchName[i] = dir.ext[i - 8];
		}
	}
	printf("%s\n", searchName);
	return *searchName;
}
// constructor
dirPrint_t newDirPrint() {
	dirPrint_t p;
	p.next = NULL;
	return p;
}
/* Search a dirEntry in hashTable
* Input: the hashTable that it searches in and the name of the dirEntry
*/
static int counta = 0;
int searchHashTable(fileEntry_t* hashTable[], unsigned char name[12]) {
	counta++;
	int index = -1;
	int stringInt = 0;
	bool found = false;
	for (int i = 0; i < 8; i++) {
		stringInt += (int)(name[i]);
	}
	int hashIndex;
	hashIndex = stringInt % 101;

	while (hashTable[hashIndex]->nextFile != NULL) {
		char dirName[12];

		for (int i = 0; i < 11; i++) {
			if (i < 8) {
				dirName[i] = (hashTable[hashIndex]->fileInfo).name[i];
			}
			else {
				dirName[i] = (hashTable[hashIndex]->fileInfo).ext[i - 8];
			}
		}
		dirName[11] = '\0';
		if (strcmp(dirName, name) == 0) {
			index = hashIndex;

			while (hashTable[index]->filePath->nextPath != NULL) {
				for (int i = 0; i < 8; i++) {
					if ((hashTable[index]->filePath->path)[i] != 0x20) {
						printf("%c", (hashTable[index]->filePath->path)[i]);
					}
					else {
						printf("/");
						break;
					}
				}
				hashTable[index]->filePath = hashTable[index]->filePath->nextPath;

			}
			return index;
		}
		hashTable[hashIndex] = hashTable[hashIndex]->nextFile;
	}
	return index;
}
/* this function searches a dirEntry
* Input: the directory that it searches and the name of the dirEntry
* Output: boolean value
*/
static int countb = 0;
bool searchDirItem(dirItem_t dirItem, unsigned char name[12]) {

	countb++;
	bool found = false;

	while (dirItem.fileListHead->next != NULL) {

		char dirName[12];

		for (int i = 0; i < 11; i++) {
			if (i < 8) {
				dirName[i] = (dirItem.fileListHead->directory).name[i];
			}
			else {
				dirName[i] = (dirItem.fileListHead->directory).ext[i - 8];
			}
		}
		dirName[11] = '\0';
		if (strcmp(dirName, name) == 0) {
			found = true;
			printDirEntryName2(dirItem.fileListHead->directory);
			printf("/");
			return found;
		}
		dirItem.fileListHead = dirItem.fileListHead->next;
	}
	while (dirItem.dirListHead->next != NULL) {

		found = searchDirItem(dirItem.dirListHead->subDir, name);
		if (found) {
			printDirEntryName2(dirItem.dirListHead->directory);
			printf("/");
			return found;
		}
		dirItem.dirListHead = dirItem.dirListHead->next;

	}
	return found;
}
/*
* delete a dirEntry
*/
bool deleteDirEntry(sector sect, int offset) {
	const char *path = "\\\\.\\D:";
	unsigned char tmp[512] = { 0 };
	DWORD size = 32;
	DWORD len = ReadSector(tmp, sect.sectorNum, 512);
	tmp[offset*size] = 0xE5;
	WriteSector(path, sect.sectorNum, tmp);
}
/*
* create a dirEntry and write it to disk as raw data.
*/
bool createDirEntry(sector sect, int offset, unsigned char name[8], unsigned char ext[3]) {
	const char *path = "\\\\.\\D:";
	unsigned char tmp[512] = { 0 };
	// read the bootsector and create bpb
	unsigned char s[512] = { 0 };
	DWORD len2 = ReadSector(s, 0, 512);
	fat32_bootsector *bpb = (fat32_bootsector *)s;
	// now sect is the start of FAT1
	sector fat;
	fat.sectorNum = bpb->reserved_sector_count;
	DWORD len3 = ReadSector(tmp, fat.sectorNum, 512);
	int k = 12;
	int clusterLo = 0X03;
	int clusterHi = 0X00;
	while (k < 509) {
		if (tmp[k] == 0) {
			tmp[k] = 0xFF;
			tmp[k + 1] = 0xFF;
			tmp[k + 2] = 0xFF;
			tmp[k + 3] = 0x0F;
			break;
		}
		k += 4;
		if (clusterLo < 0XFF) {
			clusterLo++;
		}
		else {
			clusterHi++;
		}
	}
	WriteSector(path, fat.sectorNum, tmp);
	// now sect is the start of FAT2
	fat.sectorNum = bpb->reserved_sector_count + bpb->table_size_32;
	WriteSector(path, fat.sectorNum, tmp);

	DWORD size = 32;
	DWORD len = ReadSector(tmp, sect.sectorNum, 512);
	for (int i = 0; i < 481; i++) {

	}
	int index = 32;
	while (index < 481) {
		if (tmp[index] != 0x00) {
			offset++;
			index += 32;
		}
		else {
			break;
		}
	}
	for (int i = 0; i < 8; i++) {
		tmp[i + offset * size] = name[i];
	}
	for (int i = 8; i < 11; i++) {
		tmp[i + offset * size] = ext[i - 8];
	}
	for (int i = 28; i < 32; i++) {
		tmp[i + offset * size] = 0x00;
	}
	// attrib
	tmp[11 + offset * size] = 0x20;
	// reserved
	tmp[12 + offset * size] = 0x18;
	// cluster
	tmp[26 + offset * size] = clusterLo;
	tmp[27 + offset * size] = clusterHi;
	// size
	tmp[28 + offset * size] = 0xFF;

	WriteSector(path, sect.sectorNum, tmp);
}
/*
* recover a deleted dirEntry in root directory (only works for jpg file)
* Input: const char* path			write the recovered file to path
* Return value: void
*/
void recoverDirEntry(const char* path) {
	int offset = 0;
	sector sect = getRootDir();
	bool end = false;
	unsigned char s[512] = { 0 };
	DWORD len = ReadSector(s, 0, 512);
	fat32_bootsector *bpb = (fat32_bootsector *)s;
	while (end == false) {
		for (int offset = 0; offset < 16; offset++) {
			unsigned char out[512] = { 0 };
			DWORD len = ReadSector(out, sect.sectorNum, 512);
			if (out[offset * 32] == 0xE5) {
				// read clusterNum and filesize 
				dirEntry* dir = (dirEntry *)(out + offset * 32 * sizeof(out[0]));
				dirEntry directory = *dir;
				if (directory.attribs == 0x20) {
					// if this is a file
					unsigned int size = directory.fileSize;
					unsigned int clusterIndexLo = directory.clusterIndexLo;
					unsigned int clusterIndexHi = 0; // 1 clusterIndexHi = 1 GB

				    /*
					 * check if this is the correct clusterIndexHi
					 * by checking the jpeg start of file marker.  
					 */
					unsigned int clusterIndex;
					sector tmp;
					while (1) {
						clusterIndex = clusterIndexLo + clusterIndexHi * 16 * 16 * 16 * 16;
						printf("try clusterIndexHi: %d\n", clusterIndex);
						// calculate the sectorNum from the clusterIndex
						tmp.sectorNum = 2 * bpb->table_size_32 + bpb->reserved_sector_count + (clusterIndex - 2)*bpb->sectors_per_cluster;
						// read the sector
						unsigned char tmp_sector[512] = { 0 };
						DWORD length = ReadSector(tmp_sector, tmp.sectorNum, 512);
						// check if this sector starts with FF D8 FF E0
						if (tmp_sector[0] == 0xFF || tmp_sector[1] == 0xD8 || tmp_sector[2] == 0xFF || tmp_sector[3] == 0xE0) {
							break;
						}
						else {
							// try next possible clusterIndexHi
							clusterIndexHi++;
						}
					}
					// now the clusterIndexHi is correct, copy the data to rawData array
					printf("cluster: %d\nstart sector: %d\n", clusterIndex, tmp.sectorNum);
					unsigned char* rawData = malloc(sizeof(unsigned char)*size);
					unsigned char tmp_sector[512] = { 0 };
					unsigned int count = 0;
					unsigned int sectorCount = size / 512 + 1;
					printf("sector count: %d\n", sectorCount);
					while (count < sectorCount) {
						ReadSector(tmp_sector, tmp.sectorNum, 512);
						for (int i = 0; i < 512; i++) {
							rawData[i + 512 * count] = tmp_sector[i];
						}
						tmp.sectorNum++;
						count++;
					}
					//fwrite it to new place
					FILE *fp;
					fp = fopen(path, "wb");
					fwrite(rawData, sizeof(rawData[1]), size, fp);
					fclose(fp);
					printf("recovered 1 jpg");
					end = true;
					break;
				}
				else if (directory.attribs == 0x10) {

				}


			}

		}
		sect.sectorNum++;
	}
}