#include <task.h>
#include <paging.h>
#include <kernel.h>
#include <stddef.h>
#include <stdint.h>

static Task *runningTask;
static Task mainTask;
static Task otherTask;
 
void yield() 
{
    Task *last = runningTask;
    runningTask = runningTask->next;
    switchTask(&last->regs, &runningTask->regs);
}

static void otherMain() 
{

    term_write("Hello!\n", 7);
    yield();
}
 
void initTasking() 
{
    // Get RFLAGS and CR3
    asm volatile("mov %%cr3, %%rax; mov %%rax, %0;":"=m"(mainTask.regs.cr3)::"%rax");
    asm volatile("pushfq; movl (%%rsp), %%eax; mov %%rax, %0; popfq;":"=m"(mainTask.regs.rflags)::"%rax");
 
    createTask(&otherTask, otherMain, mainTask.regs.rflags, (uint64_t*)mainTask.regs.cr3);
    mainTask.next = &otherTask;
    otherTask.next = &mainTask;
 
    runningTask = &mainTask;
}
 
void createTask(Task *task, void (*main)(), uint64_t flags, uint64_t* pml4)
{
    task->regs.rax = 0;
    task->regs.rbx = 0;
    task->regs.rcx = 0;
    task->regs.rdx = 0;
    task->regs.rsi = 0;
    task->regs.rdi = 0;
    task->regs.rflags = flags;
    task->regs.rip = (uint64_t) main;
    task->regs.cr3 = (uint64_t) pml4;
    task->regs.rsp = (uint64_t) malloc(4096) + 0x1000; // Not implemented here
    task->next = NULL;
}
