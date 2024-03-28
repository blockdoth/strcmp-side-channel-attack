#define NOBUILD_IMPLEMENTATION
#include "./nob.h"

#define CFLAGS "-Wall", "-Wextra", "-pedantic", "-O0", "lm"

int main(int argc, char **argv){
    GO_REBUILD_URSELF(argc, argv);


    FOREACH_FILE_IN_DIR(src, "src", {
        if (ENDS_WITH(src, ".c")) {
            Cstr src_path = PATH("src", src);
            CMD("cc", CFLAGS, "-o", NOEXT(src_path), src_path);
        }
    });
}
