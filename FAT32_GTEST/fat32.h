#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <WinBase.h>
#include <stdbool.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;





#include "packed.h"
typedef struct
{
	unsigned char       bootjmp[3];
	unsigned char       oem_name[8];
	unsigned short      bytes_per_sector;
	unsigned char       sectors_per_cluster;
	unsigned short      reserved_sector_count;
	unsigned char       fat_num;
	unsigned short      root_entry_count;
	unsigned short      total_sectors_16;
	unsigned char       media_type;
	unsigned short      table_size_16;
	unsigned short      sectors_per_track;
	unsigned short      head_side_count;
	unsigned int        hidden_sector_count;
	unsigned int        total_sectors_32;
	//extended fat32 stuff
	unsigned int        table_size_32;
	unsigned short      extended_flags;
	unsigned short      fat_version;
	unsigned int        root_cluster;
	unsigned short      fat_info;
	unsigned short      backup_BS_sector;
	unsigned char       reserved_0[12];
	unsigned char       drive_number;
	unsigned char       reserved_1;
	unsigned char       boot_signature;
	unsigned int        volume_id;
	unsigned char       volume_label[11];
	unsigned char       fat_type_label[8];
}PACKED fat32_bootsector;
#include "endpacked.h"

#include "packed.h"
typedef struct
{
	u8 		name[8];
    u8 		ext[3];
    u8 		attribs;
    u8 		reserved;
    u8 		createTimeMs;
    u16 	createTime;
    u16 	createDate;
    u16 	accessDate;
    u16 	clusterIndexHi;
    u16 	mTime;
    u16 	mDate;
    u16 	clusterIndexLo;
    u32 	fileSize;
}PACKED dirEntry;
#include "endpacked.h"


typedef struct sector
{
	unsigned int	sectorNum;
}sector;

typedef struct dirItem
{
	struct fileList* fileListHead;
	struct dirList* dirListHead;
}dirItem_t;

typedef struct dirList
{
	struct dirList* next;
	dirItem_t subDir;
	dirEntry directory;
}dirList_t;

typedef struct dirPrint
{
	struct dirPrint* next;
	dirEntry directory;
}dirPrint_t;

typedef struct fileList
{
	struct fileList* next;
	dirEntry directory;
}fileList_t;

typedef struct cluster
{
	unsigned int		clusterNum;
	sector				sector; 	//means this cluster start from that sector
}cluster;

typedef struct fileEntry
{
	struct fileEntry* nextFile;
	dirEntry fileInfo;
	struct filePath* filePath;
}fileEntry_t;

typedef struct filePath
{
	struct filePath* nextPath;
	u8 path[8];
}filePath_t;