

#include "console.h"
//
#include "rxtx_irq.h"

// process any incoming "command" message
void task_console_rx(void) {
    uint_fast8_t rpos = readb(&receive_pos), pop_count;
    int_fast8_t ret = rx_find_block(receive_buf, rpos, &pop_count);
    if (ret > 0)
        rx_run_cmds(receive_buf, pop_count);
    if (ret) {
        rx_buffer_clean(pop_count);
        //if (ret > 0)
            // TODO command_send_ack();
    }
}

