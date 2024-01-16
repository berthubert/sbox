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
#define STBI_FAILURE_USERMSG
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
void setupSeccomp(void*);

int main(int argc, char **argv) {
  mallopt(M_MMAP_MAX, 0);
  mallopt(M_TRIM_THRESHOLD, -1);
  auto ptr = malloc(100000000);
  auto image = readFromFDToVector(0);
  free(ptr);

  if(ptr)
    prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);
  else {
    cerr<<"Out of memory\n";
    syscall(SYS_exit, 0);
  }
  int cols, rows, channels;
  unsigned const char* data = stbi_load_from_memory(&image[0], image.size(), & cols, &rows, &channels, 0);
  cerr<<"channels = "<< channels <<", cols = "<<cols<<", rows = "<<rows<<"\n";
  if(!data) {
    cerr << "Could not load: "<<stbi_failure_reason() << endl;
    syscall(SYS_exit, 0);
  }
  cerr<<"Loaded!"<<endl;
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
    write(1, data, siz);
  };
  
  // int stbi_write_png_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data, int stride_in_bytes);
  int rc = stbi_write_png_to_func(func, 0, ncols, nrows, channels, output, 0);
  if(rc != 1)
    cerr<<"Could not write thumbnail correctly"<<endl;

  syscall(SYS_exit, 0);
}
