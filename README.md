# MakenOS
A WIP hobby x86-64 OS using the Limine bootloader for me to learn about OS development. Written with C and a bit of NASM
Currently, it features 4-level paging, interrupt handling with the PIC and APIC, PS2 keyboard support, task switching, ACPI table parsing, and data fetching from the SATA drive.

Current Roadmap:
- Read and writes to AHCI drives 
- Implement a working filesystem (ext2, FAT32, etc.)
- Add a shell that can load and run programs
- USB Drivers?
