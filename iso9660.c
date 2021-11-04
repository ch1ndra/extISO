#include <stddef.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stack.c"
#include "iso9660.h"


/* A Directory Entry is a subset of a Directory Record in the sense that each
 * Directory Record contains multiple Directory Entries. Each Directory Entry 
 * may either be a dummy entry, a file entry or may point to another Directory 
 * Record which again contains multiple Directory Entries */
 

static uint8_t * read_dir_record(size_t offset, size_t len, iso9660_t * iso)
{
uint8_t * rec;

    /* Seek to the directory record */
    fseek(iso->fp, offset, SEEK_SET);
    
    rec = malloc(len);  /* Must be freed by the caller */
    fread(rec, 1, len, iso->fp);
    
    return rec;
}


static Dir_Entry * find_dir_entry(uint8_t * rec, const char * dirname)
{
Dir_Entry * ent;
    
    for(ent = (Dir_Entry *)rec; ent->DirRecLen; ent = (Dir_Entry *)rec)
    {
        if( strncmp(&ent->FileIDfield, dirname, strlen(dirname)) == 0) {
            return ent;
        }
        rec += ent->DirRecLen;
    }
 
    return NULL;
}


file_t * iso9660_fopen(iso9660_t * iso, const char * filepath)
{
file_t  *   file;
uint8_t     level;
uint8_t *   rec;           /* Directory Record */
Dir_Entry * ent;           /* Directory Entry */
Dir_Entry * ptr;
char *      path;
char *      dir;
char *      rest;

                              
    if(!iso) {
      return NULL;
    }
    
    file = calloc(1, sizeof(file_t));
    if(!file)
      return NULL;
        
    path = strdup(filepath);
    
    /* Reset the pointer to the location of the root directory */
    fseek(iso->fp, iso->base, SEEK_SET);
    
    ent = malloc(sizeof(Dir_Entry));
    memcpy(ent, (Dir_Entry *)iso->root, sizeof(Dir_Entry));
        
    /* Recurse through each directory appearing in the file path */
    for(level = 1, dir = strtok_r(path, "/", &rest); level <= 8; level++, dir = strtok_r(NULL, "/", &rest) )
    {
        if(dir == NULL) {
          printf("Error: Specified path does not lead to a file!\n");
          break;
        }
        
        rec = read_dir_record(ent->ExtentLoc_LSB * CD_SECTOR_SIZE, ent->ExtentSz_LSB, iso);
        ptr = find_dir_entry(rec, dir);
        if(!ptr) {
          break;
        }
        memcpy(ent, ptr, sizeof(Dir_Entry));
        free(rec);
         
        /* Bit 1 of FileFlags is set if this entry corresponds to a directory (unset for a file).
         * We stop, without generating any errors, if the specified path leads to a file regardless of 
         * whether or not the entire path has been navigated */
        if(!(ent->FileFlags & 0x02)) {
            file->size = ent->ExtentSz_LSB;
            strncpy(file->name, dir, strlen(dir));
            /* Seek to the location of the file */
            fseek(iso->fp, (ent->ExtentLoc_LSB * CD_SECTOR_SIZE), SEEK_SET);
            free(ent);
            free(path);
            /* Save the offset to the location of the file */
            file->offset = ftell(iso->fp);
            return file;
         }    
    }
    
    free(ent);
    free(path);
    free(file);
    return NULL;      
}


void iso9660_fclose(iso9660_t * iso, file_t * file)
{
    /* Restore the file pointer to the location of the root directory */
    fseek(iso->fp, iso->base, SEEK_SET);
    
    free(file);
}


size_t iso9660_fread(char * ptr, size_t size, size_t n, iso9660_t * iso, file_t * file)
{
size_t bytes;

    if( !file || !iso->fp)
      return 0;
      
    /* Seek to the last offset */
    fseek(iso->fp, file->offset, SEEK_SET);

    bytes = (size*n < file->size)? size*n : file->size;
    fread(ptr, 1, bytes, iso->fp);
    
    /* Save the offset */
    file->offset = ftell(iso->fp);
    
    return bytes;  
}


size_t iso9660_fwrite(char * ptr, size_t size, size_t n, file_t * file)
{
FILE * fp;
size_t bytes;

    fp = fopen(file->name, "wb");
    if(!fp) {
      printf("Error: Unable to write to the output file!\n");
      return 0;
    }
    
    bytes=fwrite(ptr, size, n, fp);
    fclose(fp);
    
    return bytes;
}


iso9660_t * iso9660_open(const char * ISOfile)
{
iso9660_t * iso;
struct ISO_Volume_Descriptor * VolDescPtr;
int8_t      error = 0;


    iso = malloc(sizeof(iso9660_t));
    if(!iso)
       return NULL;
    
    iso->fp = fopen(ISOfile, "rb");
    if( !iso->fp ) {
      printf("Specified file '%s' not found!\n", ISOfile);
      free(iso);
      return NULL;
    }
  
    /* Seek to the Volume Descriptor and read the descriptor fields */
    fseek( iso->fp, 0x8000, SEEK_SET);
    VolDescPtr = malloc( sizeof(struct ISO_Volume_Descriptor));
    fread( (uint8_t *)VolDescPtr, sizeof(struct ISO_Volume_Descriptor), 1, iso->fp ); 
    
    /* Verify that this is the Primary Volume Descriptor */
    if( VolDescPtr->VolDescType != 0x01 ) {
        printf("Primary Volume Descriptor not found!\n");
        error = 1;                       
    }
   
    if( strncmp(VolDescPtr->VolDescID, "CD001", 5) ) {
        printf("Not an ISO9660 format file!\n");
        error = 2;
        
    }
    
    if(error) {
        free(VolDescPtr);
        fclose(iso->fp);
        free(iso);
        return NULL;
    }
    
    iso->root = malloc(sizeof(Dir_Entry));
    /* Read the root directory structure from the volume descriptor */
    memcpy(iso->root, VolDescPtr->RootDirEntry, sizeof(VolDescPtr->RootDirEntry) );
    
    /* Seek to the Root Directory contents */
    fseek(iso->fp, (iso->root->ExtentLoc_LSB * CD_SECTOR_SIZE), SEEK_SET);
    /* Save the offset of the root directory */
    iso->base = ftell(iso->fp);
    
    free(VolDescPtr);
    
    return iso;
}


void iso9660_close(iso9660_t * iso)
{
    fclose(iso->fp);
    free(iso->root);
    free(iso);
}


static void read_dir(uint8_t * rec, iso9660_t * iso)
{
uint8_t *   nextrec;    /* Next directory record */
Dir_Entry * ent;        /* Directory entry */


    for(ent = (Dir_Entry *)rec; ent->DirRecLen > 0; ent = (Dir_Entry *)rec)
    {
        /* Skip dummy entries */
        if( ent->FileIDsz == 1) {
            rec += ent->DirRecLen;
            continue;
        }
          
        /* Print the path to the current directory */  
        for(int8_t i = 0; i <= stack_top(); i++)
            printf("%s/", stack_peek(i));
        
        /* Print the name of the file (or the subdirectory) from the entry */
        printf("%.*s\n", ent->FileIDsz, (char *)&ent->FileIDfield);
        
        /* Bit 1 of FileFlags is set if this entry corresponds to a (sub)directory */
        if( ent->FileFlags & 0x02 ) {
            stack_push((char *)&ent->FileIDfield, ent->FileIDsz);
            nextrec = read_dir_record(ent->ExtentLoc_LSB * CD_SECTOR_SIZE, ent->ExtentSz_LSB, iso);
            read_dir(nextrec, iso);
            stack_pop();
            free(nextrec);
        }
        
        rec += ent->DirRecLen;
    }
    
}


void iso9660_peek(iso9660_t * iso)
{
uint8_t *   rec;    /* Directory record */

    /* Read the root directory record */
    rec = read_dir_record(iso->root->ExtentLoc_LSB * CD_SECTOR_SIZE, iso->root->ExtentSz_LSB, iso);
  
    /* A minimal stack is implemented just to store directory heirarchies when traversing through directories */
    /* The program should run without a problem if all lines referencing the stack is commented out */
    stack_init();
    
    /* Recursively read the content of the directory and all subdirectories */
    read_dir(rec, iso);
    free(rec);
    
}
