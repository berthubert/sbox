# sbox
Demo of how to resize PNG images using the excellent [STB image manipulation routines](https://github.com/nothings/stb) 
from within a Linux SECCOMP_STRICT sandbox.

Based on a suggestion by [Thomas Dullien](https://github.com/thomasdullien). The STB authors freely admit that they might have security problems, and that they'll discuss their security issues in public. For this reason, a sandbox is nice to have.

SECCOMP_STRICT really allows almost nothing, not even asking the kernel for more memory. Very secure of course, but this messes with later calls to malloc(). 

The technique used here is to instruct malloc not to be clever, and to first allocate a chunk of memory, then enable SECCOMP_STRICT, and then freeing that memory. This means the allocator has enough space in its arena to work without asking the kernel for more memory.

In the code you'll find some fun notes on how compilers make it hard to actually do this trick, because they figure out that aren't actually using that large chunk of memory. So they never allocate it.

## What the sbox program does
You can pipe a PNG image (and only a PNG image) to the process, and it will return a 200 pixel wide version of it. That's it. But it does all the exciting things from within SECCOMP_STRICT, which is nice.

## Why?
This code might end up in [Trifecta](https://berthub.eu/articles/trifecta), an image sharing site mean to be secure & an example of how to write simple, compact but useful software.
