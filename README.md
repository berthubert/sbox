# sbox
Demo of how to resize PNG images using the excellent [STB image manipulation routines](https://github.com/nothings/stb) 
from within [a Linux SECCOMP_STRICT sandbox](https://book.hacktricks.xyz/linux-hardening/privilege-escalation/docker-security/seccomp), with an easy to use interface from your regular process. Based on a suggestion by [Thomas Dullien](https://github.com/thomasdullien), implemented with help from [Otto Moerbeek](https://github.com/omoerbeek) ([who knows malloc](https://www.openbsd.org/papers/eurobsdcon2023-otto-malloc.pdf)).

The STB authors freely admit that they might have security problems, and that they'll discuss their security issues in public before they fix them. For this reason, a sandbox is nice to have. 

SECCOMP_STRICT really allows almost nothing, not even asking the kernel for more memory. Very secure of course, but this messes with later calls to malloc(). 

The technique used here is to instruct malloc not to be clever, and to first allocate a chunk of memory, then enable SECCOMP_STRICT, and then freeing that memory. This means the allocator has enough space in its arena to work without asking the kernel for more memory.

In the code you'll find some fun notes on how compilers make it hard to actually do this trick, because they often figure out that aren't actually using that large chunk of memory. And then they never allocate it.

## What the sbox program does
You can pipe a PNG image (and only a PNG image) to the process, and it will return a 200 pixel wide version of it. That's it. But it does all the exciting things from within SECCOMP_STRICT, which is nice.

## How to use this from your code
A demo is in [example-client.cc](example-client.cc), but the gist is: 

```C++
#include "client.hh"
int main(int argc, char** argv)
{
  string png = readFile(argv[1]);
  string thumbnail = getThumbnailFromSandbox(png, {"./sbox"});
}
```
This will fork() and setup connectivity to the sandboxed process, and then
launches it for you.

You could also add more parameters to getThumbnailFromSandbox, and perhaps
teach the sandbox to use these paramters (so you could specify the desired
dimensions of the thumbnail, for example).

## Why?
This code might end up in [Trifecta](https://berthub.eu/articles/trifecta), an image sharing site mean to be secure & an example of how to write simple, compact but useful software.
