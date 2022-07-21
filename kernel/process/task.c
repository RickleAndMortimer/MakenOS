#include <task.h>
#include <paging.h>
#include <pmm.h>
#include <kernel.h>
#include <stddef.h>
#include <stdint.h>

static Task *runningTask;
static Task mainTask;
static Task otherTask;

extern PageTable pml4;
extern void switchTask(Registers* from, Registers* to);
extern void switchTask2(Registers* from, Registers* to);
 
void yield() 
{
    Task *last = runningTask;
    runningTask = runningTask->next;
    switchTask(&last->regs, &runningTask->regs);
}

void doIt() {
	term_write("Switching to otherTask... \n", 27);
	yield();
	term_write("good\n",5);
}

void createTask(Task *task, void (*main)(), uint64_t flags, uint64_t cr3)
{
    task->regs.rax = 0;
    task->regs.rbx = 0;
    task->regs.rcx = 0;
    task->regs.rdx = 0;
    task->regs.rsi = 0;
    task->regs.rdi = 0;
    task->regs.rflags = flags;
    task->regs.rip = (uint64_t) main;
    task->regs.cr3 = cr3;
    task->regs.rsp = (uint64_t) k_malloc(getMemoryMapBase(), getMemoryMapLength(), 4096) + 0x1000;
    task->next = NULL;
}

static void otherMain() 
{

    term_write("Hello!\n", 7);
    term_write("Going back to main...\n", 22);
    yield();
}
 
void initTasking() 
{
    // Get RFLAGS and CR3
    asm volatile("movq %%cr3, %%rax; movq %%rax, %0;":"=m"(mainTask.regs.cr3)::"%rax");
    asm volatile("pushfq; movq (%%rsp), %%rax; movq %%rax, %0; popfq;":"=m"(mainTask.regs.rflags)::"%rax");
 
    createTask(&otherTask, otherMain, mainTask.regs.rflags, mainTask.regs.cr3);
    mainTask.next = &otherTask;
    otherTask.next = &mainTask;
 
    runningTask = &mainTask;
}
