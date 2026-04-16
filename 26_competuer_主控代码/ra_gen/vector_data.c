/* generated vector source file - do not edit */
        #include "bsp_api.h"
        /* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
        #if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = iic_master_rxi_isr, /* IIC1 RXI (Receive data full) */
            [1] = iic_master_txi_isr, /* IIC1 TXI (Transmit data empty) */
            [2] = iic_master_tei_isr, /* IIC1 TEI (Transmit end) */
            [3] = iic_master_eri_isr, /* IIC1 ERI (Transfer error) */
            [4] = sci_uart_rxi_isr, /* SCI5 RXI (Receive data full) */
            [5] = sci_uart_txi_isr, /* SCI5 TXI (Transmit data empty) */
            [6] = sci_uart_tei_isr, /* SCI5 TEI (Transmit end) */
            [7] = sci_uart_eri_isr, /* SCI5 ERI (Receive error) */
            [8] = sci_uart_rxi_isr, /* SCI3 RXI (Receive data full) */
            [9] = sci_uart_txi_isr, /* SCI3 TXI (Transmit data empty) */
            [10] = sci_uart_tei_isr, /* SCI3 TEI (Transmit end) */
            [11] = sci_uart_eri_isr, /* SCI3 ERI (Receive error) */
            [12] = gpt_counter_overflow_isr, /* GPT0 COUNTER OVERFLOW (Overflow) */
            [13] = sci_uart_rxi_isr, /* SCI7 RXI (Receive data full) */
            [14] = sci_uart_txi_isr, /* SCI7 TXI (Transmit data empty) */
            [15] = sci_uart_tei_isr, /* SCI7 TEI (Transmit end) */
            [16] = sci_uart_eri_isr, /* SCI7 ERI (Receive error) */
            [17] = sci_uart_rxi_isr, /* SCI2 RXI (Receive data full) */
            [18] = sci_uart_txi_isr, /* SCI2 TXI (Transmit data empty) */
            [19] = sci_uart_tei_isr, /* SCI2 TEI (Transmit end) */
            [20] = sci_uart_eri_isr, /* SCI2 ERI (Receive error) */
            [21] = sci_uart_rxi_isr, /* SCI8 RXI (Receive data full) */
            [22] = sci_uart_txi_isr, /* SCI8 TXI (Transmit data empty) */
            [23] = sci_uart_tei_isr, /* SCI8 TEI (Transmit end) */
            [24] = sci_uart_eri_isr, /* SCI8 ERI (Receive error) */
            [25] = sci_uart_rxi_isr, /* SCI9 RXI (Receive data full) */
            [26] = sci_uart_txi_isr, /* SCI9 TXI (Transmit data empty) */
            [27] = sci_uart_tei_isr, /* SCI9 TEI (Transmit end) */
            [28] = sci_uart_eri_isr, /* SCI9 ERI (Receive error) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_IIC1_RXI,GROUP0), /* IIC1 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_IIC1_TXI,GROUP1), /* IIC1 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_IIC1_TEI,GROUP2), /* IIC1 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_IIC1_ERI,GROUP3), /* IIC1 ERI (Transfer error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_SCI5_RXI,GROUP4), /* SCI5 RXI (Receive data full) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_SCI5_TXI,GROUP5), /* SCI5 TXI (Transmit data empty) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_SCI5_TEI,GROUP6), /* SCI5 TEI (Transmit end) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_SCI5_ERI,GROUP7), /* SCI5 ERI (Receive error) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_SCI3_RXI,GROUP0), /* SCI3 RXI (Receive data full) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_SCI3_TXI,GROUP1), /* SCI3 TXI (Transmit data empty) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_SCI3_TEI,GROUP2), /* SCI3 TEI (Transmit end) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_SCI3_ERI,GROUP3), /* SCI3 ERI (Receive error) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_GPT0_COUNTER_OVERFLOW,GROUP4), /* GPT0 COUNTER OVERFLOW (Overflow) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_SCI7_RXI,GROUP5), /* SCI7 RXI (Receive data full) */
            [14] = BSP_PRV_VECT_ENUM(EVENT_SCI7_TXI,GROUP6), /* SCI7 TXI (Transmit data empty) */
            [15] = BSP_PRV_VECT_ENUM(EVENT_SCI7_TEI,GROUP7), /* SCI7 TEI (Transmit end) */
            [16] = BSP_PRV_VECT_ENUM(EVENT_SCI7_ERI,GROUP0), /* SCI7 ERI (Receive error) */
            [17] = BSP_PRV_VECT_ENUM(EVENT_SCI2_RXI,GROUP1), /* SCI2 RXI (Receive data full) */
            [18] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TXI,GROUP2), /* SCI2 TXI (Transmit data empty) */
            [19] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TEI,GROUP3), /* SCI2 TEI (Transmit end) */
            [20] = BSP_PRV_VECT_ENUM(EVENT_SCI2_ERI,GROUP4), /* SCI2 ERI (Receive error) */
            [21] = BSP_PRV_VECT_ENUM(EVENT_SCI8_RXI,GROUP5), /* SCI8 RXI (Receive data full) */
            [22] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TXI,GROUP6), /* SCI8 TXI (Transmit data empty) */
            [23] = BSP_PRV_VECT_ENUM(EVENT_SCI8_TEI,GROUP7), /* SCI8 TEI (Transmit end) */
            [24] = BSP_PRV_VECT_ENUM(EVENT_SCI8_ERI,GROUP0), /* SCI8 ERI (Receive error) */
            [25] = BSP_PRV_VECT_ENUM(EVENT_SCI9_RXI,GROUP1), /* SCI9 RXI (Receive data full) */
            [26] = BSP_PRV_VECT_ENUM(EVENT_SCI9_TXI,GROUP2), /* SCI9 TXI (Transmit data empty) */
            [27] = BSP_PRV_VECT_ENUM(EVENT_SCI9_TEI,GROUP3), /* SCI9 TEI (Transmit end) */
            [28] = BSP_PRV_VECT_ENUM(EVENT_SCI9_ERI,GROUP4), /* SCI9 ERI (Receive error) */
        };
        #endif
        #endif