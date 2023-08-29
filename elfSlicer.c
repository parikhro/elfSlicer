#include <err.h>
#include <fcntl.h>

#include <libelf.h> 
#include <gelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <string.h>

int decreaseSection(const char* file_name, const char* section_name, int bytes){
    int fd;
    Elf* elf; 
    
    Elf_Scn* scn;
    GElf_Shdr shdr;

    char* name; 
    
    size_t shstrndx;
    off_t section_start_offset, section_end_offset;

    if(elf_version(EV_CURRENT) == EV_NONE) {
        errx(EX_SOFTWARE , "ELF library initialization failed: %s", elf_errmsg(-1));
        return -1;
    }

    if( (fd = open(file_name, O_RDWR, 0)) < 0){
        err(EX_NOINPUT, "open \"%s\" failed", file_name); 
        return -1;
    }
    

    if( (elf = elf_begin(fd, ELF_C_RDWR, NULL)) == NULL){  
        errx(EX_SOFTWARE, "elf_begin() failed: %s", elf_errmsg(-1));
        return -1;
    }

    if( elf_kind(elf) != ELF_K_ELF){
        errx(EX_DATAERR, "\"%s\" is not an ELF object.", file_name);
        return -1;
    }

    if(elf_getshdrstrndx(elf, &shstrndx) != 0){
        errx(EX_SOFTWARE, "elf_getshdrstrndx() failed: %s", elf_errmsg(-1));
        return -1;
    }
    
    scn = NULL;

    while( (scn = elf_nextscn(elf, scn)) != NULL){
        if(gelf_getshdr(scn, &shdr) != &shdr){
            errx(EX_SOFTWARE, "getshdr() failed %s", elf_errmsg(-1));
            return -1;
        }
        
        if( (name = elf_strptr(elf, shstrndx, shdr.sh_name)) == NULL){
            errx(EX_SOFTWARE, "elf_strptr() failed %s", elf_errmsg(-1));
            return -1;
        }
        if(strcmp(name, section_name) == 0){
            printf("Found matching section\n");

            // found matching section
            if(shdr.sh_size < bytes){
                errx(EX_USAGE, "cannot reduce section by more bytes than its size");
                return -1;
            }
            
            // store original start and end pos
            section_start_offset = shdr.sh_offset;
            section_end_offset = shdr.sh_offset + shdr.sh_size;

            shdr.sh_size = shdr.sh_size - bytes;

            if(!gelf_update_shdr(scn, &shdr)){
                errx(EX_SOFTWARE, "failed to update section header %s", elf_errmsg(-1));
                return -1;
            }
            
            // shift the rest of the program backwards to remove the gap
            char buffer[1024];
            off_t current_offset = section_end_offset;
            ssize_t bytes_read;

            while( (bytes_read = pread(fd, buffer, sizeof(buffer), current_offset)) > 0){ // stores bytes into buffer from after offset
                if (pwrite(fd, buffer, bytes_read, current_offset - bytes) != bytes_read) { // writes bytes into file into offset - bytes
                    perror("Failed to shift data");
                    return -1;
                }
            
                current_offset += bytes_read;
            
            }

            
            // adjust for offset in every section header after edited section
            Elf_Scn* iter_scn = NULL;
            GElf_Shdr iter_shdr;

            while( (iter_scn = elf_nextscn(elf, iter_scn)) != NULL) {
                if(gelf_getshdr(scn, &iter_shdr) != &iter_shdr){
                    errx(EX_SOFTWARE, "getshdr() failed %s", elf_errmsg(-1));
                    return -1;
                }
                
                // only edit those after section edited
                if(iter_shdr.sh_offset > section_end_offset){
                    iter_shdr.sh_offset = iter_shdr.sh_offset - bytes;
                    
                    if(!gelf_update_shdr(iter_scn, &iter_shdr)){
                        errx(EX_SOFTWARE, "failed to update section header %s", elf_errmsg(-1));
                        return -1;
                    }

                }

            }
            
            //also adjust global program header offset
            GElf_Ehdr ehdr;

            gelf_getehdr(elf, &ehdr);
            for(int i = 0; i < ehdr.e_phnum; i++){ 
                GElf_Phdr phdr;
                gelf_getphdr(elf, i, &phdr);
                if (phdr.p_offset > section_end_offset) { // change offset of each phdr entry if affected
                    phdr.p_offset = phdr.p_offset - bytes;
                    if(!gelf_update_phdr(elf, i, &phdr)){
                        errx(EX_SOFTWARE, "failed to update program header %s", elf_errmsg(-1));
                        return -1;
                    }
                }
            }

            // Decrease file size
            if (ftruncate(fd, lseek(fd, 0, SEEK_END) - bytes) == -1) {
                perror("Failed to truncate file");
            }
        }
    }

    if(!elf_update(elf, ELF_C_WRITE)){
        errx(EX_SOFTWARE, "failed to write changes");
    }

    elf_end(elf); // release handle resources
    close(fd);

    return 0;
}

int main(int argc, char** argv){

    if(argc != 5){
        errx(EX_USAGE, "usage: %s file-name section increase/decrease bytes", argv[0]);
    }
    
    if(strcmp(argv[3], "decrease") == 0){
        if(decreaseSection(argv[1], argv[2], atoi(argv[4])) == -1){
            fprintf(stderr, "decrease section failed\n");
            return -1;
        }
    }
    
    return 0;
}
