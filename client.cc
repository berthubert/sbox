#include "client.hh"
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdexcept>

using namespace std;


static auto readFromFD(int fileDescriptor) {
    const int bufferSize = 4096;  // Adjust the buffer size according to your needs
    char buffer[bufferSize];
    string result;

    ssize_t bytesRead;
    while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0) {
        result.insert(result.end(), buffer, buffer + bytesRead);
    }
    return result;
}


std::string getThumbnailFromSandbox(const std::string& in, vector<const char*> args)
{
  if(args.empty())
    throw runtime_error("Need to pass a binary that contains the worker");
  if(args.back() != nullptr)
    args.push_back(nullptr);
  
  // [0] = read, [1] = write
  int downfd[2], upfd[2];
  if (pipe(downfd) < 0 || pipe(upfd) < 0)
    throw runtime_error("Unable to open pipe for coprocess: " + string(strerror(errno)));

  int res = fork();
  if(res < 0) {
    close(downfd[0]); close(downfd[1]);
    close(upfd[0]); close(upfd[1]);
    throw runtime_error("Could not fork: "+string(strerror(errno)));
  }

  if(!res) { // child
    close(downfd[1]);
    close(upfd[0]);

    dup2(downfd[0], 0);    close(downfd[0]);
    dup2(upfd[1], 1);      close(upfd[1]);

    // close all other FDs
    int fdlimit = (int)sysconf(_SC_OPEN_MAX);
    for(int fd = 2; fd < fdlimit; ++fd)
      close(fd);


    // launch the sandboxed converter
    if(execv(args[0], const_cast<char* const*>(args.data())) < 0) {
      // we could return status somehow perhaps, this won't do anything:
      throw runtime_error("Unable to execv: "+string(strerror(errno)));
    }
  }
  close(downfd[0]);
  close(upfd[1]);
  //  cerr<<"Going to write to downstream\n";
  res = write(downfd[1], in.c_str(), in.size());
  if(res != (int)in.size()) {
    close(downfd[1]); close(upfd[0]);
    throw runtime_error("Could not write all the things, " + to_string(res)+" != "+ to_string(in.size()));
  }
  close(downfd[1]); // signals EOF to the downstream
  //  cerr<<"Done writing, waiting for response"<<endl;
  string ret = readFromFD(upfd[0]);
  close(upfd[0]);
  return ret;
}


