#ifndef TASK_H
#define TASK_H

#include "libs/stdint.hpp"

struct task_t {
    uint64_t pid;
    uint64_t ppid;

    enum class state_t {
        RUNNING,
        BLOCKED,
        ZOMBIE,
        TERMINATED
    } state;

    uint64_t time_slice;

    paddr_t cr3;
    vaddr_t kernel_stack;
    vaddr_t user_stack;

    struct context_t {
        uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
        uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
        uint64_t rip, cs, flags, rsp, ss;
    } context;

    task_t *next;
};

class task_manager {
public:
    task_manager();
    ~task_manager();

    void init();
    void create_task(uint64_t pid, uint64_t ppid);
    void destroy_task(uint64_t pid);
    void schedule();
    void switch_context(task_t *old_task, task_t *new_task);
};

#endif // TASK_H