diff --git a/GNUmakefile b/GNUmakefile
index 804a19c..c2a0d46 100644
--- a/GNUmakefile
+++ b/GNUmakefile
@@ -8,11 +8,11 @@ all-hdd: barebones.hdd
 debug: barebones.iso
 	objcopy --only-keep-debug kernel/kernel.elf kernel.sym
 	objcopy --strip-debug kernel/kernel.elf
-	qemu-system-x86_64 -s -S -M q35 -m 2G -cdrom barebones.iso -boot d  
+	qemu-system-x86_64 -s -S -M q35 -m 2G -drive file=test.img format=raw -cdrom barebones.iso -boot d  
 
 .PHONY: run
 run: barebones.iso
-	qemu-system-x86_64 -M q35 -m 2G -cdrom barebones.iso -boot d 
+	qemu-system-x86_64 -M q35 -m 2G -drive file=test.img,format=raw -cdrom barebones.iso -boot d 
 
 .PHONY: run-uefi
 run-uefi: ovmf-x64 barebones.iso
diff --git a/kernel/devices/pci.c b/kernel/devices/pci.c
index 1223501..5d11607 100644
--- a/kernel/devices/pci.c
+++ b/kernel/devices/pci.c
@@ -3,6 +3,7 @@
 #include "pci.h"
 #include "ioapic.h"
 #include "../lib/print.h"
+#include "../kernel.h"
 #include <stdbool.h>
 
 uint16_t pciConfigReadWord(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) 
@@ -73,16 +74,43 @@ void checkAllBuses(void)
     }
 }
 
-void pciInterruptHandler(InterruptFrame* frame) 
+static void pciInterruptHandler(InterruptFrame* frame) 
 {
     printNumber(frame->int_no);
 }
 
 void enablePCIInterrupts(uint8_t bus, uint8_t device, uint8_t function, size_t ioapicaddr) 
 {
-    uint16_t interrupt = pciConfigReadWord(bus, device, function, 0x4C);
+    uint16_t interrupt = pciConfigReadWord(bus, device, function, 0x3E);
     uint16_t interrupt_line = interrupt & 0xFF;
     uint16_t interrupt_pin = interrupt >> 8;
+    // TODO: use AML to figure out which interrupt line to use
     writeIOAPIC(ioapicaddr, interrupt_line * 2 + 0x10, 0x20 + interrupt_line);
     registerInterruptHandler(0x20 + interrupt_line, &pciInterruptHandler);
 }
+
+void checkMSI(uint8_t bus, uint8_t device, uint8_t func)
+{
+    uint16_t status = pciConfigReadWord(bus, device, func, 0x4);
+    printNumber(pciConfigReadWord(bus, device, func, 0x20));
+    printNumber(pciConfigReadWord(bus, device, func, 0x22));
+    printNumber(pciConfigReadWord(bus, device, func, 0x24));
+    printNumber(pciConfigReadWord(bus, device, func, 0x26));
+    if (status & 0x10) 
+    {
+        uint16_t capabilities_pointer = pciConfigReadWord(bus, device, func, 0x36);
+        uint16_t capability = pciConfigReadWord(bus, device, func, capabilities_pointer + 0x2);
+        if ((capability & 0xFF) == 5) 
+        {
+            term_write("MSI Enabled!", 13);
+        }
+        else 
+        {
+            printNumber(capability >> 8);
+        }
+    }
+    else
+    {
+        term_write("No MSI!", 8);
+    }
+}
diff --git a/kernel/devices/pci.h b/kernel/devices/pci.h
index b00a328..51bd883 100644
--- a/kernel/devices/pci.h
+++ b/kernel/devices/pci.h
@@ -2,7 +2,10 @@
 #define PCI_H
 
 #include <stdint.h>
+#include <stddef.h>
 
 uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
+void enablePCIInterrupts(uint8_t bus, uint8_t device, uint8_t function, size_t ioapicaddr);
+void checkMSI(uint8_t bus, uint8_t device, uint8_t func);
 
 #endif
diff --git a/kernel/devices/ps2_keyboard.c b/kernel/devices/ps2_keyboard.c
index 590071b..0761085 100644
--- a/kernel/devices/ps2_keyboard.c
+++ b/kernel/devices/ps2_keyboard.c
@@ -5,7 +5,7 @@ const char* set1_scancodes[] = {
                             "", "<ESC>", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
                             "-", "=", "\b \b", "\t", "q", "w", "e", "r", "t", "y", "u", "i", 
                             "o", "p", "[", "]", "\n", "^", "a", "s", "d", "f", "g", "h", 
-                            "j", "k", "l", ";", "\"", "`", "<L-SHIFT>", "\\", "z", "x", "c", "v", 
+                            "j", "k", "l", ";", "'", "`", "<L-SHIFT>", "\\", "z", "x", "c", "v", 
                             "b", "n", "m", ",", ".", "/", "<R-SHIFT>", "*", "<L-ALT>", " ", "<C-LOCK>", "<F1>", "<F2>",
                             "<F3>", "<F4>", "<F5>", "<F6>", "<F7>", "<F8>", "<F9>", "<F10>", "<N-LOCK>", "<S-LOCK>", "7", "8",
                             "9", "-", "4", "5", "6", "+", "1", "2", "3", "0", ".", "",
@@ -23,24 +23,14 @@ const char* shift_set1_scancodes[] = {
                             "", "", "", ""
                         };
 
+// TODO: Finish adding scancodes
 const char* set2_scancodes[] = { 
-                            "", "<F9>", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
-                            "-", "=", "\b \b", "\t", "q", "w", "e", "r", "t", "y", "u", "i", 
-                            "o", "p", "[", "]", "\n", "^", "a", "s", "d", "f", "g", "h", 
-                            "j", "k", "l", ";", "\"", "`", "", "\\", "z", "x", "c", "v", 
-                            "b", "n", "m", ",", ".", "/", "", "*", "", " ", "", "", "",
-                            "", "", "", "", "", "", "", "", "", "", "", "",
-                            "", "", "", "", "", "", "", "", "", "", "", "",
-                            "", "", "", ""
-                        };
-
-const char* shift_set2_scancodes[] = { 
-                            "", "", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
-                            "_", "+", "\b \b", "\t", "Q", "W", "E", "R", "T", "Y", "U", "I", 
-                            "O", "P", "{", "}", "\n", "^", "A", "S", "D", "F", "G", "H", 
-                            "J", "K", "L", ":", "\"", "~", "", "|", "Z", "X", "C", "V", 
-                            "B", "N", "M", "<", ">", "?", "", "*", "", " ", "", "", "",
-                            "", "", "", "", "", "", "", "", "", "", "", "",
-                            "", "", "", "", "", "", "", "", "", "", "", "",
-                            "", "", "", ""
+                            "", "<F9>", "", "<F5>", "<F3>", "<F1>", "<F2>", "<F12>", "", "<F10>", "<F8>", "<F6>",
+                            "<F4>", "\t", "`", "", "", "<L-ALT>", "<L-SHIFT>", "", "<L-CTRL>", "q", "1", "", 
+                            "", "", "z", "s", "a", "w", "2", "", "", "c", "x", "d", 
+                            "e", "4", "3", "", "", " ", "v", "f", "t", "r", "5", "", 
+                            "g", "y", "6", "", "", "", "m", "j", "u", "7", "8", "", "",
+                            "", ",", "k", "i", "o", "0", "9", "", ".", "/", "l", ";",
+                            "p", "-", "", "", "", "'", "", "[", "=", "", "", "<C-LOCK>",
+                            "<R-SHIFT>", "\n", "]", "", "\\"
                         };
diff --git a/kernel/filesystem/file.c b/kernel/filesystem/file.c
index b948659..097f347 100644
--- a/kernel/filesystem/file.c
+++ b/kernel/filesystem/file.c
@@ -47,15 +47,15 @@ int fread(inode* node, char* buffer, size_t blocks, size_t length)
     return 1; 
 }
 
-void* f_malloc(inode* node, size_t index) 
+void* f_malloc(inode* node, size_t block_index) 
 {
     for (size_t i = 0; i < 56; i++) 
     {
-        if ((table.dnode_bitmap >> i) & 1) 
+        if (!((table.dnode_bitmap >> i) & 1)) 
         {
             table.dnode_bitmap |= (1 << i);
-            node->block[index] = k_malloc();
-            return node->block[index];
+            node->block[block_index] = k_malloc();
+            return node->block[block_index];
         }
     }
     return NULL;
diff --git a/kernel/filesystem/file.h b/kernel/filesystem/file.h
index 9415232..67fea1d 100644
--- a/kernel/filesystem/file.h
+++ b/kernel/filesystem/file.h
@@ -50,6 +50,7 @@ typedef struct inode_table {
 inode* fopen(char* filename);
 int fwrite(inode* node, char* data, size_t len);
 int fread(inode* i, char* buffer, size_t blocks, size_t length);
+void* f_malloc(inode* node, size_t block_index);
 inode_table* initRamFS();
 
 #endif
diff --git a/kernel/kernel.c b/kernel/kernel.c
index b86b7e7..06a3500 100644
--- a/kernel/kernel.c
+++ b/kernel/kernel.c
@@ -12,6 +12,7 @@
 #include "devices/ps2.h"
 #include "stivale2.h"
 #include "devices/serial.h"
+#include "devices/ahci.h"
 #include "filesystem/file.h"
 
 extern RSDPDescriptor20 *rsdp_descriptor;
@@ -27,7 +28,6 @@ extern IOAPICNonMaskableInterrupt* ioapic_interrupts[];
 extern LAPICAddressOverride* lapic_address_overrides[];
 extern x2LAPIC* x2_lapics[];
 
-
 void (*term_write)(const char *string, size_t length);
 struct stivale2_struct_tag_memmap* memmap_tag;
 
@@ -132,17 +132,14 @@ void _start(struct stivale2_struct *stivale2_struct) {
     }
 
     void *term_write_ptr = (void *)term_str_tag->term_write;
-    
     term_write = term_write_ptr;
 
-    term_write("Hello World\n", 13);
-    printNumber(rsdp_tag->rsdp);
-
     rsdp_descriptor = (RSDPDescriptor20*) rsdp_tag->rsdp;
     if (rsdp_descriptor->descriptor10.revision == 2) 
     {
         xsdt = (XSDT*)rsdp_descriptor->xsdt_address;
     }
+
     rsdt = (RSDT*)(uintptr_t)rsdp_descriptor->descriptor10.rsdt_address;
 
     if ((validateRSDPChecksum() & 0xFF) == 0) 
@@ -193,7 +190,8 @@ void _start(struct stivale2_struct *stivale2_struct) {
 
     memmap_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);
 
-    PageTable* pml4 = initPML4();
+    // Initialize paging and memory management
+    initPML4();
     printMemoryMaps();
     setMemoryMap(4);
 
@@ -207,7 +205,8 @@ void _start(struct stivale2_struct *stivale2_struct) {
 
 	initTasking();
 
-    /*
+    /* test that paging works
+   
     uint64_t* p = getPhysicalAddress((void*) 0x9000);
     uint64_t* f = getPhysicalAddress((void*) 0xA000);
 
@@ -229,12 +228,22 @@ void _start(struct stivale2_struct *stivale2_struct) {
     printNumber(f);
 
     */
+
+    /* Test basic filesystem code
+
     inode_table* f = initRamFS();
     inode* i = fopen("neighbor");
-    fwrite(i, "hello", 6);
-    char jk[10];
-    fread(i, jk, 1, 6);
-    term_write(jk, 10);
+    
+    char jk[18];
+    f_malloc(i, 0);
+    fwrite(i, "jerry seinfelding\n", 19);
+
+    fread(i, jk, 1, 19);
+    term_write(jk, 19);
+
+    fwrite(i, "jerry seinfelded\n", 18);
+    fread(i, jk, 1, 18);
+    term_write(jk, 18);
     
     uint16_t rose = pciConfigReadWord(0, 31, 3, 0x20);
     printNumber(rose);
@@ -242,7 +251,21 @@ void _start(struct stivale2_struct *stivale2_struct) {
     rose = pciConfigReadWord(0, 31, 3, 0x22);
     printNumber(rose);
 
+    */
+
 	enableSerialCOM1(ioapics[0]->address);
+    checkMSI(0, 31, 2);
+    
+    // Test AHCI drivers for a successful read
+    HBA_MEM* host = (HBA_MEM*) 0xFEBD5000;
+    probePort(host);
+
+    uint16_t* s = k_malloc();
+    if (read(&host->ports[0], 0, 0, 1, s))
+    {
+        term_write((char*) s, 13);
+        term_write("File successfully read!\n", 26);
+    }
 
     for (;;) 
     {
diff --git a/kernel/memory/heap.c b/kernel/memory/heap.c
index 66a50e3..05a7406 100644
--- a/kernel/memory/heap.c
+++ b/kernel/memory/heap.c
@@ -6,7 +6,7 @@
 
 void* malloc(size_t size) 
 {
-
+     
 }
 
 
diff --git a/kernel/memory/pmm.c b/kernel/memory/pmm.c
index b904452..f32fbf6 100644
--- a/kernel/memory/pmm.c
+++ b/kernel/memory/pmm.c
@@ -9,7 +9,8 @@ static struct stivale2_mmap_entry* memmap;
 
 static const char* getMemoryMapType(uint32_t type) 
 {
-    switch (type) {
+    switch (type) 
+    {
         case 0x1:
             return "Usable RAM";
         case 0x2:
@@ -33,8 +34,10 @@ static const char* getMemoryMapType(uint32_t type)
 
 void printMemoryMaps() 
 {
-    for (size_t i = 0; i < memmap_tag->entries; i++) {
-		switch (memmap_tag->memmap[i].type) {
+    for (size_t i = 0; i < memmap_tag->entries; i++) 
+    {
+		switch (memmap_tag->memmap[i].type) 
+        {
 			case 1:	
 			case 0x1000:
 			case 3:
@@ -49,7 +52,7 @@ void printMemoryMaps()
 
 				term_write("Length ", 7);
 				printNumber(memmap_tag->memmap[i].length);
-			break;
+                break;
 		}
     }
 }
@@ -71,16 +74,19 @@ uint64_t getMemoryMapLength()
 
 static void* b_malloc(uint64_t* base, size_t length, size_t size) 
 {
-    if (length <= BLOCK_SIZE && *base != 0) {
+    if (length <= BLOCK_SIZE && *base != 0) 
+    {
         return NULL;
     }
     size_t half = length / 2;
     // Allocate if the current length is enough and unallocated
-    if (half <= size && *base == 0) {
+    if (half <= size && *base == 0) 
+    {
 		return base;
     }
     // Try to find another block
-    else if (half > size) {
+    else if (half > size) 
+    {
 		uint64_t* left = b_malloc(base, half, size);
 		return left ? left : b_malloc(base + half, half, size);
     }
@@ -97,7 +103,7 @@ void* k_malloc()
 void k_free(void* base) 
 {
 	uint64_t* ptr = base;
-	for (size_t i = 0; i < BLOCK_SIZE; i++) 
+	for (size_t i = 0; i < BLOCK_SIZE / 64; i++) 
     {
 		ptr[i] = 0;
 	}
diff --git a/kernel/memory/pmm.h b/kernel/memory/pmm.h
index 3a459c1..0c1b6de 100644
--- a/kernel/memory/pmm.h
+++ b/kernel/memory/pmm.h
@@ -6,6 +6,17 @@
 
 #define BLOCK_SIZE 4096
 
+typedef struct Node {
+    void* base;
+    size_t length;
+    struct Node* next;
+} Node;
+
+typedef struct List {
+    size_t size;
+    Node* head;
+} List;
+
 void printMemoryMaps();
 void setMemoryMap(uint8_t selection);
 void* getMemoryMapBase();
