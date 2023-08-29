#include <err.h>
#include <fcntl.h>

#include <gelf.h>   // generic elf library works on 32/64 bit
#include <libelf.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <bsd/vis.h>

void print_ptype(size_t pt){

    char *s;
    #define C(V) case PT_##V:   s = #V; break
        switch (pt){
            C(NULL); 		C(LOAD); 			C(DYNAMIC);
            C(INTERP); 		C(NOTE); 			C(SHLIB);
            C(PHDR); 		C(TLS); 			
            C(LOOS); 	    C(HIOS); 		    C(LOPROC);
            C(HIPROC);

            //translates to case PT_##NULL
        default:
            s = "unknown";
            break;
        }
        printf(" \"%s\"", s);
    #undef C
}


int main(int argc, char** argv){

    int i, fd;
    Elf* e;
    char* id;
    char bytes[5];
    size_t n;
    
    GElf_Phdr phdr; // Elf program header generic representation
                 
    if(argc != 2){
        errx(EX_USAGE, "usage: %s file-name", argv[0]);
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
    
    if( elf_kind(e) != ELF_K_ELF) {
        errx(EX_DATAERR, "\"%s\" is not an ELF object.", argv[1]);
    }
    

    if( elf_getphdrnum(e, &n) != 0){    // retrieves number of program header table entries and writes into n
        errx(EX_SOFTWARE, "getphdrnum() failed %s.", elf_errmsg(-1));
    }
    
    for(i = 0; i < n; i++){         // iterate over each index of program header 
        if(gelf_getphdr(e, i, &phdr) != &phdr){     // retrieve table entry at each index returns pointer (null on fail)
            errx(EX_SOFTWARE, "getphdr() failed: %s.", elf_errmsg(-1));
        }
    }
    
    printf("PHDR %d:\n", i);

    // sets up macro to print a field of the elf header
    #define PRINT_FMT "    %-20s 0x%jx\n"
    #define PRINT_FIELD(N) do { \
                            printf(PRINT_FMT, #N, (uintmax_t) phdr.N); \
                            } while (0)
    #define     NL() do { printf("\n"); } while(0)
    PRINT_FIELD(p_type);        
    print_ptype(phdr.p_type);   NL();
    PRINT_FIELD(p_offset);      NL();
    PRINT_FIELD(p_vaddr);       NL();
    PRINT_FIELD(p_paddr);       NL();
    PRINT_FIELD(p_filesz);      NL();
    PRINT_FIELD(p_memsz);       NL();
    PRINT_FIELD(p_flags);
    printf(" [");
    if(phdr.p_flags & PF_X) printf(" execute");

    if(phdr.p_flags & PF_R) printf(" read");

    if(phdr.p_flags & PF_W) printf(" write");
    printf(" ]");   NL();
    PRINT_FIELD(p_align);

    

    elf_end(e); // release handle resources
    close(fd);

    return EXIT_SUCCESS;
}
