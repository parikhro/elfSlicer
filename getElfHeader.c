#include <err.h>
#include <fcntl.h>

#include <gelf.h>   // generic elf library works on 32/64 bit
#include <libelf.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <bsd/vis.h>

int main(int argc, char** argv){

    int i, fd;
    Elf* e;
    char* id;
    char bytes[5];
    size_t n;
    
    GElf_Ehdr ehdr; // Elf executable header generic representation
                 
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
    

    if( gelf_getehdr(e, &ehdr) == NULL){    // gelf_getehdr(Elf* elf, GElf_Ehdr *dst)
        errx(EX_SOFTWARE, "getehdr() failed %s.", elf_errmsg(-1));
    }

    if( (i = gelf_getclass(e)) == ELFCLASSNONE){    // returns elf class (1 for 32 bit, 2 for 64 bit)
        errx(EX_SOFTWARE, "getclass() failed: %s.", elf_errmsg(-1));
    }
    
    printf("32: %d, 64: %d\n", ELFCLASS32, ELFCLASS64);
    printf("%s: %d-bit ELF object\n", argv[1], i == ELFCLASS32 ? 32 : 64);


    if( (id = elf_getident(e, NULL)) == NULL){  // retrieves bytes of e_ident
        errx(EX_SOFTWARE, "get_ident() failed %s.", elf_errmsg(-1));
    }
    
    printf("%3s e_ident[0..%1d] %7s", " ", EI_ABIVERSION, " "); // ABI version depends on OS

    for(int i = 0; i <= EI_ABIVERSION; i++){
        vis(bytes, id[1], VIS_WHITE, 0);
        printf(" ['%s' %X]", bytes, id[i]);
    }
    printf("\n");
    
    // sets up macro to print a field of the elf header
    #define PRINT_FMT "    %-20s 0x%jx\n"
    #define PRINT_FIELD(N) do { \
                            printf(PRINT_FMT, #N, (uintmax_t) ehdr.N); \
                            } while (0)

    PRINT_FIELD(e_type);
    PRINT_FIELD(e_machine);
    PRINT_FIELD(e_version);
    PRINT_FIELD(e_entry);
    PRINT_FIELD(e_phoff);
    PRINT_FIELD(e_shoff);
    PRINT_FIELD(e_flags);
    PRINT_FIELD(e_ehsize);
    PRINT_FIELD(e_phentsize);
    PRINT_FIELD(e_shentsize);

    if( elf_getshdrnum(e, &n) != 0){    // retrieves number of sections 
        errx(EX_SOFTWARE, "getshdrnum() failed %s.", elf_errmsg(-1));
    }
    printf(PRINT_FMT, "(shnum)", (uintmax_t) n);
    
    if( elf_getshdrstrndx(e, &n) != 0){    // retrieves section name string table index
        errx(EX_SOFTWARE, "getshdrstrndx() failed %s.", elf_errmsg(-1));
    }
    printf(PRINT_FMT, "(shstrndx)", (uintmax_t) n);

    if( elf_getphdrnum(e, &n) != 0){    // retrieves number of program header table entries
        errx(EX_SOFTWARE, "getphdrnum() failed %s.", elf_errmsg(-1));
    }
    printf(PRINT_FMT, "(phnum)", (uintmax_t) n);

    

    elf_end(e); // release handle resources
    close(fd);

    return EXIT_SUCCESS;
}
