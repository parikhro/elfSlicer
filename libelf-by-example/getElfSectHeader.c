#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <bsd/vis.h>

int main (int argc, char** argv){
    int fd;
    Elf* e;
    char* name, *p , pc[4*sizeof(char)];
    Elf_Scn* scn;
    Elf_Data* data;
    GElf_Shdr shdr;
    size_t n, shstrndx, sz;

    if(argc != 2){
        errx(EX_USAGE, "usage: %s file-name", argv[0]);
    }

    
    if(elf_version(EV_CURRENT) == EV_NONE) {
        errx(EX_SOFTWARE , "ELF library initialization failed: %s", elf_errmsg(-1));
    }

    if( (fd = open(argv[1], O_RDONLY, 0)) < 0){
        err(EX_NOINPUT, "open \"%s\" failed", argv[1]); 
    }

    if( (e = elf_begin(fd, ELF_C_READ, NULL)) == NULL){
        errx(EX_SOFTWARE, "elf_begin() failed: %s", elf_errmsg(-1));
    }
    
    if(elf_kind(e) != ELF_K_ELF) {
        errx(EX_DATAERR, "\"%s\" is not an ELF object.", argv[1]);
    }

    if(elf_getshdrstrndx(e, &shstrndx) != 0){   // retrieves section header string table index and stores it for later use
        errx(EX_SOFTWARE, "elf_getshdrtstrndx() failed: %s", elf_errmsg(-1));
    }

    scn = NULL;

    while( (scn = elf_nextscn(e, scn)) != NULL){    // elf_nextscn returns null at end
        if(gelf_getshdr(scn, &shdr) != &shdr){
            errx(EX_SOFTWARE, "getshdr() failed %s", elf_errmsg(-1));
        }

        if( (name = elf_strptr(e, shstrndx, shdr.sh_name)) == NULL){
        // retrieves section name using string table found earlier
            //converts string offset in shdr.sh_name to a char*
            errx(EX_SOFTWARE, "elf_strptr() failed %s", elf_errmsg(-1));
        }

        printf("Section %-4.4jd %s\n", (uintmax_t) elf_ndxscn(scn), name);

    }

    if( (scn = elf_getscn(e, shstrndx)) == NULL){   // retrieve section descriptor for string table using the index that was found earlier
        errx(EX_SOFTWARE, "getscn() failed %s", elf_errmsg(-1));
    }

    if(gelf_getshdr(scn, &shdr) != &shdr){
        errx(EX_SOFTWARE, "getshdr(shstrndx) failed %s", elf_errmsg(-1));
    }
    printf(".shstrab: size=%jd\n", (uintmax_t) shdr.sh_size);

    data = NULL;
    n = 0;
    
    //cycle through Elf_Data descriptors associated with each section, printing characters in each buffer
    while(n < shdr.sh_size && (data = elf_getdata(scn, data)) != NULL){

        p = (char*) data->d_buf;
        
        while(p < (char*) data->d_buf + data->d_size){
            if(vis(pc, *p, VIS_WHITE, 0)){
                printf("%s", pc);
            }
            n++;
            p++;
            putchar((n % 16) ? ' ' : '\n');
        }
    }
    putchar('\n');

    elf_end(e);
    close(fd);
    
    return 0;
}
