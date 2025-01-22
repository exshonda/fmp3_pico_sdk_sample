/*
 *  TOPPERS/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2005-2016 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 * 
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 */

/*
 * ターゲット依存モジュール（RaspberryPi Pico用）
 */
#include "kernel_impl.h"
#include "target_syssvc.h"
#include <sil.h>
#ifdef TOPPERS_OMIT_TECS
#include "chip_serial.h"
#endif
#include "hardware/exception.h"
#include "hardware/irq.h"
#include "pico/runtime_init.h"

#include "hardware/claim.h"
#include "pico/mutex.h"
#include "pico/assert.h"
#include "pico/runtime.h"
#include "pico/multicore.h"

extern const void (**p_exc_tbl[])(void);
extern const FP __vectors[];
extern const FP _kernel_c_exc_tbl_prc1[];
extern const FP _kernel_c_exc_tbl_prc2[];
extern uint32_t __StackTop;
extern uint32_t __StackBottom;
extern uint32_t __StackOneTop;
extern uint32_t __StackOneBottom;

STK_T *const _kernel_istk_table[TNUM_PRCID] = {
	(STK_T *)&__StackBottom,
	(STK_T *)&__StackOneBottom
};

STK_T *const _kernel_istkpt_table[TNUM_PRCID] = {
	(STK_T *)&__StackTop,
	(STK_T *)&__StackOneTop
};

const size_t _kernel_istksz_table[TNUM_PRCID] = {
	PICO_STACK_SIZE,
	PICO_CORE1_STACK_SIZE
};

static bool exception_is_compile_time_default(exception_handler_t handler) {
    return handler == (irq_handler_t)_kernel_default_exc_handler;
}

static inline exception_handler_t *get_exception_table(int prcidx) {
    return (exception_handler_t *) p_exc_tbl[prcidx];
}

static void set_raw_exception_handler_and_restore_interrupts(enum exception_number num, exception_handler_t handler, uint32_t save) {
    // update vtable (vtable_handler may be same or updated depending on cases, but we do it anyway for compactness)
    get_exception_table(0)[num] = handler;
    get_exception_table(1)[num] = handler;
    __dmb();
    restore_interrupts_from_disabled(save);
}

static inline void check_exception_param(__unused enum exception_number num) {
    invalid_params_if(HARDWARE_EXCEPTION, num < MIN_EXCEPTION_NUM || num > MAX_EXCEPTION_NUM);
}

exception_handler_t __wrap_exception_get_vtable_handler(enum exception_number num) {
    check_exception_param(num);
    return get_exception_table(0)[num];
}

exception_handler_t __wrap_exception_set_exclusive_handler(enum exception_number num, exception_handler_t handler) {
    check_exception_param(num);
    uint32_t save = save_and_disable_interrupts();
    exception_handler_t current = exception_get_vtable_handler(num);
    hard_assert(handler == current || exception_is_compile_time_default(current));
    set_raw_exception_handler_and_restore_interrupts(num, handler, save);
    return current;
}

void __wrap_exception_restore_handler(enum exception_number num, exception_handler_t original_handler) {
    hard_assert(exception_is_compile_time_default(original_handler));
    uint32_t save = save_and_disable_interrupts();
    set_raw_exception_handler_and_restore_interrupts(num, original_handler, save);
}

static inline irq_handler_t *get_vtable(int prcidx) {
    return (irq_handler_t *) p_exc_tbl[prcidx];
}

static inline void *add_thumb_bit(void *addr) {
    return (void *) (((uintptr_t) addr) | 0x1);
}

static inline void *remove_thumb_bit(void *addr) {
    return (void *) (((uintptr_t) addr) & (uint)~0x1);
}

static void set_raw_irq_handler_and_unlock(uint num, irq_handler_t handler, uint32_t save) {
    // update vtable (vtable_handler may be same or updated depending on cases, but we do it anyway for compactness)
    get_vtable(0)[VTABLE_FIRST_IRQ + num] = handler;
    get_vtable(1)[VTABLE_FIRST_IRQ + num] = handler;
    __dmb();
    spin_unlock(spin_lock_instance(PICO_SPINLOCK_ID_IRQ), save);
}

extern void irq_handler_chain_first_slot(void);
extern void irq_handler_chain_remove_tail(void);

extern struct irq_handler_chain_slot {
    uint16_t inst1;
    uint16_t inst2;
    uint16_t inst3;
    union {
        // On Arm, when a handler is removed while executing, it needs a 32-bit instruction at
        // inst3, which overwrites the link and the priority; this is ok because no one else is
        // modifying the chain, as the chain is effectively core-local, and the user code which
        // might still need this link disables the IRQ in question before updating, which means we
        // aren't executing!
        struct {
            int8_t link;
            uint8_t priority;
        };
        uint16_t inst4;
    };
    irq_handler_t handler;
} irq_handler_chain_slots[PICO_MAX_SHARED_IRQ_HANDLERS];

static int8_t irq_handler_chain_free_slot_head;

static inline bool is_shared_irq_raw_handler(irq_handler_t raw_handler) {
    return (uintptr_t)raw_handler - (uintptr_t)irq_handler_chain_slots < sizeof(irq_handler_chain_slots);
}

irq_handler_t __wrap_irq_get_vtable_handler(uint num) {
    check_irq_param(num);
    return get_vtable(0)[VTABLE_FIRST_IRQ + num];
}

void __wrap_irq_set_exclusive_handler(uint num, irq_handler_t handler) {
    check_irq_param(num);
    spin_lock_t *lock = spin_lock_instance(PICO_SPINLOCK_ID_IRQ);
    uint32_t save = spin_lock_blocking(lock);
    __unused irq_handler_t current = __wrap_irq_get_vtable_handler(num);
    hard_assert(current == (irq_handler_t)_kernel_default_int_handler || current == handler);
    set_raw_irq_handler_and_unlock(num, handler, save);
}

static uint16_t make_j_16(uint16_t *from, void *to) {
    uint32_t ui_from = (uint32_t)from;
    uint32_t ui_to = (uint32_t)to;
    int32_t delta = (int32_t)(ui_to - ui_from - 4);
    assert(delta >= -2048 && delta <= 2046 && !(delta & 1));
    return (uint16_t)(0xe000 | ((delta >> 1) & 0x7ff));
}

static void insert_bl_32(uint16_t *from, void *to) {
    uint32_t ui_from = (uint32_t)from;
    uint32_t ui_to = (uint32_t)to;
    uint32_t delta = (ui_to - ui_from - 4) / 2;
    assert(!(delta >> 11u));
    from[0] = (uint16_t)(0xf000 | ((delta >> 11u) & 0x7ffu));
    from[1] = (uint16_t)(0xf800 | (delta & 0x7ffu));
}

static inline void *resolve_j_16(uint16_t *inst) {
    assert(0x1c == (*inst)>>11u);
    int32_t i_addr = (*inst) << 21u;
    i_addr /= (int32_t)(1u<<21u);
    return inst + 2 + i_addr;
}

static inline int8_t slot_diff(struct irq_handler_chain_slot *to, struct irq_handler_chain_slot *from) {
    static_assert(sizeof(struct irq_handler_chain_slot) == 12, "");
    int32_t result = 0xaaaa;
    // return (to - from);
    // note this implementation has limited range, but is fine for plenty more than -128->127 result
    pico_default_asm (
         "subs %1, %2\n"
         "adcs %1, %1\n" // * 2 (and + 1 if negative for rounding)
         "muls %0, %1\n"
         "lsrs %0, %0, #20\n"
         : "+l" (result), "+l" (to)
         : "l" (from)
         : "cc"
         );
    return (int8_t)result;
}

static const uint16_t inst16_return_from_last_slot = 0xbd01; // pop {r0, pc}
static inline int8_t get_slot_index(struct irq_handler_chain_slot *slot) {
    return slot_diff(slot, irq_handler_chain_slots);
}

void __wrap_irq_add_shared_handler(uint num, irq_handler_t handler, uint8_t order_priority) {
    check_irq_param(num);
    spin_lock_t *lock = spin_lock_instance(PICO_SPINLOCK_ID_IRQ);
    uint32_t save = spin_lock_blocking(lock);
    hard_assert(irq_handler_chain_free_slot_head >= 0); // we must have a slot
    struct irq_handler_chain_slot *slot = &irq_handler_chain_slots[irq_handler_chain_free_slot_head];
    int8_t slot_index = irq_handler_chain_free_slot_head;
    irq_handler_chain_free_slot_head = slot->link;
    irq_handler_t vtable_handler = get_vtable(0)[VTABLE_FIRST_IRQ + num];
    if (!is_shared_irq_raw_handler(vtable_handler)) {
        // start new chain
        hard_assert(vtable_handler == (irq_handler_t)_kernel_default_int_handler);
        struct irq_handler_chain_slot slot_data = {
                .inst1 = 0xa100,                                                             // add r1, pc, #0
                .inst2 = make_j_16(&slot->inst2, (void *) irq_handler_chain_first_slot),     // b irq_handler_chain_first_slot
                .handler = handler,
                .inst3 = inst16_return_from_last_slot,
                .link = -1,
                .priority = order_priority
        };
        *slot = slot_data;
        vtable_handler = (irq_handler_t)add_thumb_bit(slot);
    } else {
        assert(!((((uintptr_t)remove_thumb_bit(vtable_handler)) - ((uintptr_t)irq_handler_chain_slots)) % sizeof(struct irq_handler_chain_slot)));
        struct irq_handler_chain_slot *prev_slot = NULL;
        struct irq_handler_chain_slot *existing_vtable_slot = remove_thumb_bit((void *) vtable_handler);
        struct irq_handler_chain_slot *cur_slot = existing_vtable_slot;
        while (cur_slot->priority > order_priority) {
            prev_slot = cur_slot;
            if (cur_slot->link < 0) break;
            cur_slot = &irq_handler_chain_slots[cur_slot->link];
        }
        if (prev_slot) {
            // insert into chain
            struct irq_handler_chain_slot slot_data = {
                    .inst1 = 0x4801,                                                   // ldr r0, [pc, #4]
                    .inst2 = 0x4780,                                                   // blx r0
                    .handler = handler,
                    .inst3 = prev_slot->link >= 0 ?
                            make_j_16(&slot->inst3, resolve_j_16(&prev_slot->inst3)) : // b next_slot
                            inst16_return_from_last_slot,
                    .link = prev_slot->link,
                    .priority = order_priority
            };
            // update code and data links
            prev_slot->inst3 = make_j_16(&prev_slot->inst3, slot),
            prev_slot->link = slot_index;
            *slot = slot_data;
        } else {
            // update with new chain head
            struct irq_handler_chain_slot slot_data = {
                    .inst1 = 0xa100,                                                           // add r1, pc, #0
                    .inst2 = make_j_16(&slot->inst2, (void *) irq_handler_chain_first_slot),   // b irq_handler_chain_first_slot
                    .handler = handler,
                    .inst3 = make_j_16(&slot->inst3, existing_vtable_slot),                    // b existing_slot
                    .link = get_slot_index(existing_vtable_slot),
                    .priority = order_priority,
            };
            *slot = slot_data;
            // fixup previous head slot
            existing_vtable_slot->inst1 = 0x4801; // ldr r0, [pc, #4]
            existing_vtable_slot->inst2 = 0x4780; // blx r0
            vtable_handler = (irq_handler_t)add_thumb_bit(slot);
        }
    }
    set_raw_irq_handler_and_unlock(num, vtable_handler, save);
}

static inline irq_handler_t handler_from_slot(struct irq_handler_chain_slot *slot) {
    return slot->handler;
}

void __wrap_irq_remove_handler(uint num, irq_handler_t handler) {
    spin_lock_t *lock = spin_lock_instance(PICO_SPINLOCK_ID_IRQ);
    uint32_t save = spin_lock_blocking(lock);
    irq_handler_t vtable_handler = get_vtable(0)[VTABLE_FIRST_IRQ + num];
    if (vtable_handler != (irq_handler_t)_kernel_default_int_handler && vtable_handler != handler) {
        if (is_shared_irq_raw_handler(vtable_handler)) {
            // This is a bit tricky, as an executing IRQ handler doesn't take a lock.

            // First thing to do is to disable the IRQ in question; that takes care of calls from user code.
            // Note that a irq handler chain is local to our own core, so we don't need to worry about the other core
            bool was_enabled = irq_is_enabled(num);
            irq_set_enabled(num, false);
            __dmb();

            // It is possible we are being called while an IRQ for this chain is already in progress.
            // The issue we have here is that we must not free a slot that is currently being executed, because
            // inst3 is still to be executed, and inst3 might get overwritten if the slot is re-used.

            // By disallowing other exceptions from removing an IRQ handler (which seems fair)
            // we now only have to worry about removing a slot from a chain that is currently executing.

            // Note we expect that the slot we are deleting is the one that is executing.
            // In particular, bad things happen if the caller were to delete the handler in the chain
            // before it. This is not an allowed use case though, and I can't imagine anyone wanting to in practice.
            // Sadly this is not something we can detect.

            uint exception = __get_current_exception();
            hard_assert(!exception || exception == num + VTABLE_FIRST_IRQ);

            struct irq_handler_chain_slot *prev_slot = NULL;
            struct irq_handler_chain_slot *existing_vtable_slot = remove_thumb_bit((void *) vtable_handler);
            struct irq_handler_chain_slot *to_free_slot = existing_vtable_slot;
            while (handler_from_slot(to_free_slot) != handler) {
                prev_slot = to_free_slot;
                if (to_free_slot->link < 0) break;
                to_free_slot = &irq_handler_chain_slots[to_free_slot->link];
            }
            if (handler_from_slot(to_free_slot) == handler) {
                int8_t next_slot_index = to_free_slot->link;
                if (next_slot_index >= 0) {
                    // There is another slot in the chain, so copy that over us, so that our inst3 points at something valid
                    // Note this only matters in the exception case anyway, and it that case, we will skip the next handler,
                    // however in that case its IRQ cause should immediately cause re-entry of the IRQ and the only side
                    // effect will be that there was potentially brief out of priority order execution of the handlers
                    struct irq_handler_chain_slot *next_slot = &irq_handler_chain_slots[next_slot_index];
                    to_free_slot->handler = next_slot->handler;
                    to_free_slot->priority = next_slot->priority;
                    to_free_slot->link = next_slot->link;
                    to_free_slot->inst3 = next_slot->link >= 0 ?
                            make_j_16(&to_free_slot->inst3, resolve_j_16(&next_slot->inst3)) : // b next_>slot->next_slot
                            inst16_return_from_last_slot,

                    // add old next slot back to free list
                    next_slot->link = irq_handler_chain_free_slot_head;
                    irq_handler_chain_free_slot_head = next_slot_index;
                } else {
                    // Slot being removed is at the end of the chain
                    if (!exception) {
                        // case when we're not in exception, we physically unlink now
                        if (prev_slot) {
                            // chain is not empty
                            prev_slot->link = -1;
                            prev_slot->inst3 = inst16_return_from_last_slot;
                        } else {
                            // chain is not empty
                            vtable_handler = (irq_handler_t)_kernel_default_int_handler;
                        }
                        // add slot back to free list
                        to_free_slot->link = irq_handler_chain_free_slot_head;
                        irq_handler_chain_free_slot_head = get_slot_index(to_free_slot);
                    } else {
                        // since we are the last slot we know that our inst3 hasn't executed yet, so we change
                        // it to bl to irq_handler_chain_remove_tail which will remove the slot.

                        // NOTE THAT THIS TRASHES PRIORITY AND LINK SINCE THIS IS A 4 BYTE INSTRUCTION
                        //      BUT THEY ARE NOT NEEDED NOW
                        insert_bl_32(&to_free_slot->inst3, (void *) irq_handler_chain_remove_tail);
                    }
                }
            } else {
                assert(false); // not found
            }
            irq_set_enabled(num, was_enabled);
        }
    } else {
        vtable_handler = (irq_handler_t)_kernel_default_int_handler;
    }
    set_raw_irq_handler_and_unlock(num, vtable_handler, save);
}

// used by irq_handler_chain.S to remove the last link in a handler chain after it executes
// note this must be called only with the last slot in a chain (and during the exception)
void __wrap_irq_add_tail_to_free_list(struct irq_handler_chain_slot *slot) {
    irq_handler_t slot_handler = (irq_handler_t) add_thumb_bit(slot);
    assert(is_shared_irq_raw_handler(slot_handler));

    uint exception = __get_current_exception();
    assert(exception);
    spin_lock_t *lock = spin_lock_instance(PICO_SPINLOCK_ID_IRQ);
    uint32_t save = spin_lock_blocking(lock);
    int8_t slot_index = get_slot_index(slot);
    if (slot_handler == get_vtable(0)[exception]) {
        get_vtable(0)[exception] = (irq_handler_t)_kernel_default_int_handler;
        get_vtable(1)[exception] = (irq_handler_t)_kernel_default_int_handler;
    } else {
        bool __unused found = false;
        // need to find who points at the slot and update it
        for(uint i=0;i<count_of(irq_handler_chain_slots);i++) {
            if (irq_handler_chain_slots[i].link == slot_index) {
                irq_handler_chain_slots[i].link = -1;
                irq_handler_chain_slots[i].inst3 = inst16_return_from_last_slot;
                found = true;
                break;
            }
        }
        assert(found);
    }
    // add slot to free list
    slot->link = irq_handler_chain_free_slot_head;
    irq_handler_chain_free_slot_head = slot_index;
    spin_unlock(lock, save);
}

void target_kernel_init_install_ram_vector_table(void) {
    __builtin_memcpy(p_exc_tbl[0], _kernel_c_exc_tbl_prc1, sizeof(void*) * TMAX_INTNO);
    __builtin_memcpy(p_exc_tbl[1], _kernel_c_exc_tbl_prc2, sizeof(void*) * TMAX_INTNO);
}

PICO_RUNTIME_INIT_FUNC_RUNTIME(target_kernel_init_install_ram_vector_table, PICO_RUNTIME_INIT_INSTALL_RAM_VECTOR_TABLE);

void core1_entory(void)
{
    Asm("cpsid f":::"memory");

    sta_ker();
}

void target_mprc_initialize(void) {
    multicore_launch_core1(core1_entory);
}

uintptr_t __used __attribute__((section(".init_array"))) __init_target_mprc_initialize = (uintptr_t)(void (*)(void)) (target_mprc_initialize);

/*
 * エラー時の処理
 */
extern void Error_Handler(void);

void software_term_hook()
{
}

#ifndef TOPPERS_OMIT_TECS
/*
 *  システムログの低レベル出力のための初期化
 *
 */
extern void tPutLogSIOPort_initialize(void);
#endif

extern const ID tmax_spnid;

/*
 * ターゲット依存部 初期化処理
 */
void target_initialize(PCB *p_my_pcb)
{
    int i;

    if (p_my_pcb->prcid == PRC1) {
        /* Reset SIO (hardware spinlock) */
        sil_wrw_mem(RP2350_SIO_SPINLOCKn(PICO_SPINLOCK_ID_OS1), 0);
        sil_wrw_mem(RP2350_SIO_SPINLOCKn(PICO_SPINLOCK_ID_OS2), 0);
        for (i = 0; i < tmax_spnid; i++) {
            sil_wrw_mem(RP2350_SIO_SPINLOCKn(i), 0);
        }
    }
    /*
     * コア依存部の初期化
     */
    core_initialize(p_my_pcb);
    /*
     * SIOを初期化
     */
#ifndef TOPPERS_OMIT_TECS
    tPutLogSIOPort_initialize();
#endif /* TOPPERS_OMIT_TECS */
}

/*
 * ターゲット依存部 終了処理
 */
void target_exit(void)
{
    /* チップ依存部の終了処理 */
    core_terminate();
    while(1) ;
}

/*
 * エラー発生時の処理
 */
void Error_Handler(void)
{
    while (1) ;
}
