#include <stdint.h>

// options1:     R P/W U/S PWT PCD A D PS(1) G AVL
// Bits (0-11):  1 1   1   1   1   1 1 1     1 3   
   
// bits (12-51): address (significant bits are 0 by default,)

// options2:        AVL PK XD
// Bits (52-63):    7   4  1 

typedef struct PageEntry {
    uint8_t present : 1;
    uint8_t writable : 1;
    uint8_t user_accessible : 1;
    uint8_t write_through_caching : 1;
    uint8_t disable_cache : 1;
    uint8_t accessed : 1;
    uint8_t dirty : 1;
    uint8_t null : 1;
    uint8_t global : 1;
    uint8_t avl1 : 3;
    uintptr_t physical_address : 40;
    uint16_t avl2 : 11;
    uint8_t no_execute : 1;
} PageEntry;

typedef struct PageTable {
    PageEntry entries[512];
} PageTable;

void* getPhysicalAddress(void* virtual_address); 
PageTable* initPML4(void); 
void mapPage(void* physical_address, void* virtual_address, uint8_t flags, uint16_t available);
