#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "iso9660.h"


int main(int argc, char *argv[])
{
iso9660_t * iso;
file_t    * file;
char      * buf;
uint16_t    i;

    if( argc < 2 ) {
      printf("Usage: extiso <isofile> <path_to_file1> <path_to_file2> ...\n");
      return -1;
    }
    
    iso = iso9660_open(argv[1]);
    if(!iso) {
      printf("Error: iso9660_open(%s) failed!\n", argv[1]);
      return -1;
    }
    
    if(argc == 2) {
      iso9660_peek(iso);
    }
    
    else {
      for(i = 2; i < argc; i++)
      {
        file = iso9660_fopen(iso, argv[i]);
        if(!file) {
          printf("Error: failed to open '%s'!\n", argv[i]);
          continue;
        }
        
        buf = malloc(file->size);
        iso9660_fread(buf, file->size, 1, iso, file);
        iso9660_fwrite(buf, file->size, 1, file);
        iso9660_fclose(iso, file);
        free(buf);
      }
    }
    
    iso9660_close(iso);
    
    return 0;
}
