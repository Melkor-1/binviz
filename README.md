[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://https://github.com/Melkor-1/binviz/edit/main/LICENSE)

# Reverse Engineering Data Files

Binary Visualization based on Christopher Domas talk: [youtube link](https://www.youtube.com/watch?v=4bM3Gut1hIk) and this Russian programmer where I discovered this talk: [youtube link](https://www.youtube.com/watch?v=AUWxl0WdiNI&pp=ygUmcmV2ZXJzZSBlbmdpbmVlcmluZyBkYXRhIGZpbGVzIHRzb2Rpbmc%3D).

## Algorithm:

* Scan the pairs of bytes of a file with a sliding window. (AA BB CC DD ... -> (AA, BB) (BB, CC) (CC, DD) ...);
* Interpret the pairs of bytes as coordinates on a 256x256 2D plain;
* Place a dot for each pair on the plain;
* The more frequent the dot, the brighter it is;
* Different patterns emerge depending on the type of the data of the file.

## Sample outputs:

#### Executables: (gdb - Intel x86_64, binviz - ARM64, git-bash - Windows PE 32+, pandoc - Intel x86_64)

[![Intel x86_64, gdb][1]][1] 
[![ARM64, this program's executable][2]][2]
[![Windows PE 32+, git-bash][3]][3]
[![Intel x86_64, pandoc][4]][4]



#### Text Files: (war and peace, stb_image_write.h)

[![war_and_peace.txt][5]][5]
[![stb_image_write.h][6]][6]


#### Audio File: (streets.wav)

[![streets.wav][7]][7]

## Building

To build, simply do:

```bash
make binviz
```

  [1]: https://i.stack.imgur.com/93tUP.png
  [2]: https://i.stack.imgur.com/N53z3.png
  [3]: https://i.stack.imgur.com/rdWQq.png
  [4]: https://i.stack.imgur.com/wE3uk.png
  [5]: https://i.stack.imgur.com/G7NgT.png
  [6]: https://i.stack.imgur.com/27vij.png
  [7]: https://i.stack.imgur.com/UD6tO.png
