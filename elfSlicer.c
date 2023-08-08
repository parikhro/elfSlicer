#include <err.h>
#include <fcntl.h>

# include <libelf.h> 
# include <stdio.h>
# include <stdlib.h>
# include <sysexits.h>
# include <unistd.h>

int main(int argc, char** argv){

    int fd;
    Elf* e;  // elf object descriptor, used to read and write data to an elf file
    char* k;
    
    Elf_Kind ek;  // enum used to determine ELF file type
                  // whether it is an archive or ELF file
    if(argc != 5){
        errx(EX_USAGE, "usage: %s file-name section increase/decrease bytes", argv[0]);
    }

    // will not accept version unknown to libelf's 
    if(elf_version(EV_CURRENT) == EV_NONE) {
        errx(EX_SOFTWARE , "ELF library initialization failed: %s", elf_errmsg(-1));
    }

    if( (fd = open(argv[1], O_RDONLY, 0)) < 0){
        err(EX_NOINPUT, "open \"%s\" failed", argv[1]); 
    }
    

    if( (e = elf_begin(fd, ELF_C_READ, NULL)) == NULL){     // elf_begin() is essentially open() for ELF files for further processing
        // opens ELF file in read mode
        // returns pointer to elf object descriptor

        errx(EX_SOFTWARE, "elf_begin() failed: %s", elf_errmsg(-1));
    }
    
    ek = elf_kind(e);   // determines what type of elf file (ar archive vs. ELF)



    
    elf_end(e); // release handle resources
    close(fd);

    return EXIT_SUCCESS;
}
