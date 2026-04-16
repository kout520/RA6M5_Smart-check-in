/* generated vector header file - do not edit */
        #ifndef VECTOR_DATA_H
        #define VECTOR_DATA_H
        #ifdef __cplusplus
        extern "C" {
        #endif
                /* Number of interrupts allocated */
        #ifndef VECTOR_DATA_IRQ_COUNT
        #define VECTOR_DATA_IRQ_COUNT    (29)
        #endif
        /* ISR prototypes */
        void iic_master_rxi_isr(void);
        void iic_master_txi_isr(void);
        void iic_master_tei_isr(void);
        void iic_master_eri_isr(void);
        void sci_uart_rxi_isr(void);
        void sci_uart_txi_isr(void);
        void sci_uart_tei_isr(void);
        void sci_uart_eri_isr(void);
        void gpt_counter_overflow_isr(void);

        /* Vector table allocations */
        #define VECTOR_NUMBER_IIC1_RXI ((IRQn_Type) 0) /* IIC1 RXI (Receive data full) */
        #define IIC1_RXI_IRQn          ((IRQn_Type) 0) /* IIC1 RXI (Receive data full) */
        #define VECTOR_NUMBER_IIC1_TXI ((IRQn_Type) 1) /* IIC1 TXI (Transmit data empty) */
        #define IIC1_TXI_IRQn          ((IRQn_Type) 1) /* IIC1 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_IIC1_TEI ((IRQn_Type) 2) /* IIC1 TEI (Transmit end) */
        #define IIC1_TEI_IRQn          ((IRQn_Type) 2) /* IIC1 TEI (Transmit end) */
        #define VECTOR_NUMBER_IIC1_ERI ((IRQn_Type) 3) /* IIC1 ERI (Transfer error) */
        #define IIC1_ERI_IRQn          ((IRQn_Type) 3) /* IIC1 ERI (Transfer error) */
        #define VECTOR_NUMBER_SCI5_RXI ((IRQn_Type) 4) /* SCI5 RXI (Receive data full) */
        #define SCI5_RXI_IRQn          ((IRQn_Type) 4) /* SCI5 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI5_TXI ((IRQn_Type) 5) /* SCI5 TXI (Transmit data empty) */
        #define SCI5_TXI_IRQn          ((IRQn_Type) 5) /* SCI5 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI5_TEI ((IRQn_Type) 6) /* SCI5 TEI (Transmit end) */
        #define SCI5_TEI_IRQn          ((IRQn_Type) 6) /* SCI5 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI5_ERI ((IRQn_Type) 7) /* SCI5 ERI (Receive error) */
        #define SCI5_ERI_IRQn          ((IRQn_Type) 7) /* SCI5 ERI (Receive error) */
        #define VECTOR_NUMBER_SCI3_RXI ((IRQn_Type) 8) /* SCI3 RXI (Receive data full) */
        #define SCI3_RXI_IRQn          ((IRQn_Type) 8) /* SCI3 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI3_TXI ((IRQn_Type) 9) /* SCI3 TXI (Transmit data empty) */
        #define SCI3_TXI_IRQn          ((IRQn_Type) 9) /* SCI3 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI3_TEI ((IRQn_Type) 10) /* SCI3 TEI (Transmit end) */
        #define SCI3_TEI_IRQn          ((IRQn_Type) 10) /* SCI3 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI3_ERI ((IRQn_Type) 11) /* SCI3 ERI (Receive error) */
        #define SCI3_ERI_IRQn          ((IRQn_Type) 11) /* SCI3 ERI (Receive error) */
        #define VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW ((IRQn_Type) 12) /* GPT0 COUNTER OVERFLOW (Overflow) */
        #define GPT0_COUNTER_OVERFLOW_IRQn          ((IRQn_Type) 12) /* GPT0 COUNTER OVERFLOW (Overflow) */
        #define VECTOR_NUMBER_SCI7_RXI ((IRQn_Type) 13) /* SCI7 RXI (Receive data full) */
        #define SCI7_RXI_IRQn          ((IRQn_Type) 13) /* SCI7 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI7_TXI ((IRQn_Type) 14) /* SCI7 TXI (Transmit data empty) */
        #define SCI7_TXI_IRQn          ((IRQn_Type) 14) /* SCI7 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI7_TEI ((IRQn_Type) 15) /* SCI7 TEI (Transmit end) */
        #define SCI7_TEI_IRQn          ((IRQn_Type) 15) /* SCI7 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI7_ERI ((IRQn_Type) 16) /* SCI7 ERI (Receive error) */
        #define SCI7_ERI_IRQn          ((IRQn_Type) 16) /* SCI7 ERI (Receive error) */
        #define VECTOR_NUMBER_SCI2_RXI ((IRQn_Type) 17) /* SCI2 RXI (Receive data full) */
        #define SCI2_RXI_IRQn          ((IRQn_Type) 17) /* SCI2 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI2_TXI ((IRQn_Type) 18) /* SCI2 TXI (Transmit data empty) */
        #define SCI2_TXI_IRQn          ((IRQn_Type) 18) /* SCI2 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI2_TEI ((IRQn_Type) 19) /* SCI2 TEI (Transmit end) */
        #define SCI2_TEI_IRQn          ((IRQn_Type) 19) /* SCI2 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI2_ERI ((IRQn_Type) 20) /* SCI2 ERI (Receive error) */
        #define SCI2_ERI_IRQn          ((IRQn_Type) 20) /* SCI2 ERI (Receive error) */
        #define VECTOR_NUMBER_SCI8_RXI ((IRQn_Type) 21) /* SCI8 RXI (Receive data full) */
        #define SCI8_RXI_IRQn          ((IRQn_Type) 21) /* SCI8 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI8_TXI ((IRQn_Type) 22) /* SCI8 TXI (Transmit data empty) */
        #define SCI8_TXI_IRQn          ((IRQn_Type) 22) /* SCI8 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI8_TEI ((IRQn_Type) 23) /* SCI8 TEI (Transmit end) */
        #define SCI8_TEI_IRQn          ((IRQn_Type) 23) /* SCI8 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI8_ERI ((IRQn_Type) 24) /* SCI8 ERI (Receive error) */
        #define SCI8_ERI_IRQn          ((IRQn_Type) 24) /* SCI8 ERI (Receive error) */
        #define VECTOR_NUMBER_SCI9_RXI ((IRQn_Type) 25) /* SCI9 RXI (Receive data full) */
        #define SCI9_RXI_IRQn          ((IRQn_Type) 25) /* SCI9 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI9_TXI ((IRQn_Type) 26) /* SCI9 TXI (Transmit data empty) */
        #define SCI9_TXI_IRQn          ((IRQn_Type) 26) /* SCI9 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI9_TEI ((IRQn_Type) 27) /* SCI9 TEI (Transmit end) */
        #define SCI9_TEI_IRQn          ((IRQn_Type) 27) /* SCI9 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI9_ERI ((IRQn_Type) 28) /* SCI9 ERI (Receive error) */
        #define SCI9_ERI_IRQn          ((IRQn_Type) 28) /* SCI9 ERI (Receive error) */
        #ifdef __cplusplus
        }
        #endif
        #endif /* VECTOR_DATA_H */