// FAT32_GTEST.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "gtest\gtest.h"  
extern "C" {
#include "fat32.h"
#include"fat32functions.h"
}



TEST(ReadSector, sector)
{
	unsigned char out[512];
	EXPECT_EQ(512, ReadSector(out, 24576, 512)) << "return value: size";	
}

TEST(ReadSector, cluster) {
	unsigned char out[16384];
	EXPECT_EQ(512, ReadCluster(out, 24576, 512)) << "return value: size";
}

TEST(getRootDir, foo) {
	sector s = getRootDir();
	EXPECT_EQ(24576, s.sectorNum);
}

TEST(readDirEntry, foo) {
	sector s;
	s.sectorNum = 24576;
	readDirEntry(s, 32, 0);
}

TEST(getSubDirSect, foo) {
	sector s;
	s.sectorNum = 24576;
	getSubDirSect(readDirEntry(s, 32, 0));

}

TEST(printDirEntryName, foo) {
	sector s;
	s.sectorNum = 24576;
	printDirEntryName(readDirEntry(s, 32, 0));
}

TEST(setDirHashTable) {

	//setDirHashTable(sector sect, fileEntry_t* hashTable[], filePath_t* parentFolder)

}


/*TEST(WriteSector, sector) {
	const char content[512] = "This is a test file...";
	const char *path = "\\\\.\\D:";
	WriteSector(path, 24608, content);
}*/


