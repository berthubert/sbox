#include "client.hh"
#include <unistd.h>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

static std::string readFile(const std::string& fname)
{
  std::ifstream file(fname);
  if (!file.is_open())
    throw runtime_error("Error opening the file: " + fname);
  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();
  return buffer.str();
}

int main(int argc, char** argv)
{
  if(argc != 2) {
    cout<<"Syntax: example-client input.png > output.png\n";
    exit(EXIT_SUCCESS);
  }
  string png = readFile(argv[1]);
  string thumbnail = getThumbnailFromSandbox(png, {"./sbox"});

  write(1, thumbnail.c_str(), thumbnail.size());
}
