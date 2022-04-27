#!/sh/bash

HOME=$PWD
TARGET=$HOME
DIFF_DIR=$HOME/diff-result
#mkdir $DIFF_DIR

#git diff v4.4.12-rt20 v5.15.34-rt40 mm/memory.c

GIT_PREV_VER="v4.4.12-rt20-kml"
GIT_COMP_VER="v5.15.34-rt40"

COMPARE_FILES=(
"Documentation/kml.txt"              
"MAINTAINERS"                        
"Makefile"                          
"arch/arm/Kconfig"                   
"arch/arm/include/asm/processor.h"   
"arch/arm/include/asm/ptrace.h"      
"arch/arm/include/asm/thread_info.h" 
"arch/arm/kernel/entry-armv.S"       
"arch/arm/lib/uaccess_with_memcpy.c" 
"arch/arm/vfp/vfphw.S"               
"drivers/pnp/pnpbios/bioscalls.c"    
"fs/binfmt_elf.c"                    
"fs/compat_binfmt_elf.c"            
"init/do_mounts.c"                   
"kernel/Kconfig.kml"                 
"kernel/tracepoint.c"                
"lib/iov_iter.c"                     
"mm/memory.c"                        
"security/tomoyo/network.c"
"arch/arm/include/asm/vdso_datapage.h"
"arch/arm/kernel/vdso.c"
"arch/arm/vdso/vdso.lds.S"
"arch/arm/vdso/vgettimeofday.c"
"kernel/time/time.c"
"kernel/time/timekeeping.c"
)

DIR_FILE_DIR=

for COMPARE_FILE in ${COMPARE_FILES[@]}; do
    mkdir $(dirname $DIFF_DIR/$COMPARE_FILE)
    git diff --output=$DIFF_DIR/$COMPARE_FILE.diff $GIT_PREV_VER $GIT_COMP_VER $COMPARE_FILE
done
