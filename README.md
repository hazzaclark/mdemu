# mdemu

![Untitled-1](https://github.com/user-attachments/assets/d3352cca-1971-4504-9eed-ab09d24adf20)

```
Sonic the Hedgehog (PAL 50) on Wayland (KDE Plasma)
```  

# Motive:

The aim with this project is to provide myself with my own surrogates means of emulating one of my favourite games consoles.

Being my major breakout into the ever-growing, enigmatic world of Reverse Engineering and Low Level Programming, this project aims to showcase my expertise in said field.

# Building:

Built using the ANSI C99 standard

```
git clone

make clean

make
```

the following output should be an executable should ``mdemu`` 

For Wayland support, drag and drop doesn't work in the conventional means
So I have added command line support 

Simply refer to the exe ``./mdemu`` followed by a path to your ROM

``./mdemu /your/rom/path/here/rom.bin``

## Documentation used:

● ```Motorolla 68000 Programmer Manual:``` https://www.nxp.com/files-static/archives/doc/ref_manual/M68000PRM.pdf

● ```Motorolla 68000 User Manual:``` https://www.nxp.com/docs/en/reference-manual/MC68000UM.pdf

● ```M68000 Assembly Programming:``` https://www.inf.pucrs.br/~calazans/undergrad/orgcomp_EC/mat_microproc/MC6800-AssemblyLProg.pdf

● ```RasterScroll for useful documentation on MD Graphics:``` https://rasterscroll.com/mdgraphics/

● ```PlutieDev's Assembly Programming documentation for the MD:``` https://plutiedev.com/

● ```SEGA Mega Drive Technical Manual, this is just a great all rounder for understanding the Mega Drive's Hardware:``` https://segaretro.org/images/a/a2/Genesis_Software_Manual.pdf

● ```Zilog Z80 CPU Manual:``` http://www.z80.info/zip/um0081.pdf

● ```Sonic Retro ASM to Hex Opcode reference:``` http://info.sonicretro.org/SCHG:68000_ASM-to-Hex_Code_Reference

● ```VDP:``` https://md.railgun.works/index.php?title=VDP


