#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <iostream>
#include <sys/syscall.h>
#include <stdlib.h>
#include <map>
#include <vector>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stb_image_write.h"

using namespace std;

auto readFromFDToVector(int fileDescriptor) {
    const int bufferSize = 4096;  // Adjust the buffer size according to your needs
    char buffer[bufferSize];
    std::vector<unsigned char> result;

    ssize_t bytesRead;
    while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0) {
        result.insert(result.end(), buffer, buffer + bytesRead);
    }
    return result;
}

/* Once in SECCOMP_MODE_STRICT, you can no longer ask the kernel for more memory, since
   neither sbrk or mmap are allowed in that state.

   A trick to still be able to do (limited) malloc after PR_SET_SECCOMP is to make sure 
   malloc has enough room in its arena to not need kernel help. To do so, you could 
   malloc a large enough block of memory, and then free it. 

   However, when compiling with optimization, you might find that this trick does not 
   work. It turns out modern compilers elide whole calls to malloc if they 
   can prove (after further optimization) that you are not actually using the pointer 
   value!

   In addition, malloc might return the released memory to the operating system, which 
   would defeat the plan.

   To fix both these problems, make sure you somehow tell the compiler the return value 
   of malloc could go somewhere or mean something. Especially clang takes some convincing!

   Secondly, use the mallopt lines below to prevent glibc from returning the memory.
*/

int main(int argc, char **argv) 
{
  mallopt(M_MMAP_MAX, 0); // only use sbrk
  mallopt(M_TRIM_THRESHOLD, -1); // don't return memory
  auto ptr = malloc(100000000);
  auto image = readFromFDToVector(0);

  if(ptr && time((time_t*)ptr)) // this is enough to make at least gcc and clang actually do the malloc
    prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);
  else {
    cerr<<"Out of memory\n";
    syscall(SYS_exit, 0); // regular exit calls exit_group, & SECCOMP strict doesn't allow that
  }
  free(ptr);
  
  int cols, rows, channels;
  unsigned const char* data = stbi_load_from_memory(&image[0], image.size(), & cols, &rows, &channels, 0);
  //  cerr<<"channels = "<< channels <<", cols = "<<cols<<", rows = "<<rows<<"\n";

  if(!data) {
    cerr << "Could not load: "<<stbi_failure_reason() << endl;
    syscall(SYS_exit, 0);
  }

  double factor = 200.0/cols;
  int ncols = factor * cols, nrows = factor * rows;
  auto output = stbir_resize_uint8_srgb( data,  cols,  rows,  0,
                                         0, ncols, nrows, 0,
                                         (stbir_pixel_layout) channels );

  if(!output) {
    cerr<<"Could not resize\n";
    syscall(SYS_exit, 0);
  }

  auto func=[](void*, void* data, int siz) {
    if(write(1, data, siz) != siz) {
      cerr << "Write error\n";
      syscall(SYS_exit, 0);
    }
  };
  
  int rc = stbi_write_png_to_func(func, 0, ncols, nrows, channels, output, 0);
  if(rc != 1)
    cerr<<"Could not write thumbnail correctly"<<endl;

  syscall(SYS_exit, 0);
}
