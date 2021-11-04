#ifndef __ISO9660_H
#define __ISO9660_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define CD_SECTOR_SIZE 2048


struct ISO_Volume_Descriptor
{
    uint8_t   VolDescType;
    char      VolDescID[5];
    uint8_t   VolDescVersion;
    uint8_t   flags;
    char      SystemID[32];
    char      VolID[32];
    uint8_t   unused1[8];
    uint32_t  SpaceSize;
    uint32_t  SpaceSize_MSB;
    uint8_t   unused2[32];
    uint32_t  VolSetSize;
    uint32_t  VolSeqNo;
    uint32_t  BlockSize;
    uint32_t  PathTblSz_LSB;
    uint32_t  PathTblSz_MSB;
    uint32_t  PathTblAddr_LSB;
    uint32_t  OptionalPathTblAddr_LSB;
    uint32_t  PathTblAddr_MSB;
    uint32_t  OptionalPathTblAddr_MSB;
    uint8_t   RootDirEntry[34];
    char      VolSetID[128];
    char      PublisherID[128];
    char      DataPreparerID[128];
    char      ApplicationID[128];
    char      CopyrightFileID[38];
    char      AbstractFileID[36];
    char      BibliographicFileID[37];
    uint8_t   CreationDateTime[17];
    uint8_t   ModificationDateTime[17];
    uint8_t   ExpirationDateTime[17];
    uint8_t   EffectiveDateTime[17];
    uint8_t   FileStructVer;
    uint8_t   unused3;
    uint8_t   ApplicationUsed[512];
    uint8_t   reserved[653];
} __attribute__((packed));

typedef struct
{
    uint8_t   DirRecLen;
    uint8_t   EAlen;
    uint32_t  ExtentLoc_LSB;
    uint32_t  ExtentLoc_MSB;
    uint32_t  ExtentSz_LSB;
    uint32_t  ExtentSz_MSB;
    uint8_t   DateTime[7];
    uint8_t   FileFlags;
    uint8_t   FileUnitSz;
    uint8_t   GapSz;
    uint32_t  VolSeqNo;
    uint8_t   FileIDsz;
    char      FileIDfield;
    /* File Identifier is of variable size */
} __attribute__((packed)) Dir_Entry;


typedef struct
{
    FILE *      fp;
    size_t      base;             /* Offset to the root directory */
    Dir_Entry * root;             /* The root directory structure */
} iso9660_t;


typedef struct
{
    size_t    offset;         /* Offset to the start of the file (after iso9600_fopen) or to the last pointer (after iso9660_fread) */
    uint32_t  size;           /* Size of the file */
    char      name[256];      /* Name of the file (Rock Ridge compatible) */
} file_t;

/* Note: Keeping track of the file offset and the base allows for opening multiples files concurrently */


iso9660_t * iso9660_open(const char * filename);
void        iso9660_close(iso9660_t * iso);
file_t *    iso9660_fopen(iso9660_t * iso, const char * filepath);
size_t      iso9660_fread(char * ptr, size_t size, size_t n, iso9660_t * iso, file_t * file);
size_t      iso9660_fwrite(char * ptr, size_t size, size_t n, file_t * file);
void        iso9660_fclose(iso9660_t * iso, file_t * file);
void        iso9660_peek(iso9660_t * iso);


#endif

