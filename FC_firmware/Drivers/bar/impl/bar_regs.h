#ifndef BAR_REGS_H
#define BAR_REGS_H

// [[REGISTER]] pressure data byte 2
#define _bar_REG_PSR_B2 0x0
// [[REGISTER]] pressure data byte 1
#define _bar_REG_PSR_B1 0x1
// [[REGISTER]] pressure data byte 0
#define _bar_REG_PSR_B0 0x2

// [[REGISTER]] temperature data byte 2
#define _bar_REG_TMP_B2 0x3
// [[REGISTER]] temperature data byte 1
#define _bar_REG_TMP_B1 0x4
// [[REGISTER]] temperature data byte 0
#define _bar_REG_TMP_B0 0x5

// [[REGISTER]] pressure config
#define _bar_REG_PRS_CFG 0x6

// [[FIELD]] precision
#define _bar_REG__PRS_CFG__PREC__OFFSET 0x0
#define _bar_REG__PRS_CFG__PREC__LENGTH 3

// [[FIELD]] rate
#define _bar_REG__PRS_CFG__RATE__OFFSET 0x4
#define _bar_REG__PRS_CFG__RATE__LENGTH 3

// [[REGISTER]] temperature config
#define _bar_REG_TMP_CFG 0x6

// [[FIELD]] precision
#define _bar_REG__TMP_CFG__PREC__OFFSET 0x0
#define _bar_REG__TMP_CFG__PREC__LENGTH 3

// [[FIELD]] rate
#define _bar_REG__TMP_CFG__RATE__OFFSET 0x4
#define _bar_REG__TMP_CFG__RATE__LENGTH 3

// [[REGISTER]] mode and status config
#define _bar_REG_MEAS_CFG 0x8

// [[FIELD]] mode and type
#define _bar_REG__MEAS_CFG__CTRL__OFFSET 0x0
#define _bar_REG__MEAS_CFG__CTRL__LENGTH 3

// [[FIELD]] pressure ready
#define _bar_REG__MEAS_CFG__PRS_READY__OFFSET 0x4
#define _bar_REG__MEAS_CFG__PRS_READY__LENGTH 1

// [[FIELD]] temperature ready
#define _bar_REG__MEAS_CFG__TMP_READY__OFFSET 0x5
#define _bar_REG__MEAS_CFG__TMP_READY__LENGTH 1

// [[FIELD]] sensor ready
#define _bar_REG__MEAS_CFG__SENS_READY__OFFSET 0x6
#define _bar_REG__MEAS_CFG__SENS_READY__LENGTH 1

// [[FIELD]] coefficients ready
#define _bar_REG__MEAS_CFG__COEFF_READY__OFFSET 0x7
#define _bar_REG__MEAS_CFG__COEFF_READY__LENGTH 1

// [[REGISTER]] irq and fifo config
#define _bar_REG_CFG_REG 0x9

// [[FIELD]] 3 / 4 wire mode
#define _bar_REG__CFG_REG__SPI_MODE__OFFSET 0x0
#define _bar_REG__CFG_REG__SPI_MODE__LENGTH 1

// [[FIELD]] fifo enable
#define _bar_REG__CFG_REG__FIFO_EN__OFFSET 0x1
#define _bar_REG__CFG_REG__FIFO_EN__LENGTH 1

// [[FIELD]] pressure result bit-shift
#define _bar_REG__CFG_REG__P_SHIFT__OFFSET 0x2
#define _bar_REG__CFG_REG__P_SHIFT__LENGTH 1

// [[FIELD]] temperature result bit-shift
#define _bar_REG__CFG_REG__T_SHIFT__OFFSET 0x3
#define _bar_REG__CFG_REG__T_SHIFT__LENGTH 1

// [[FIELD]] pressure interrupt
#define _bar_REG__CFG_REG__INT_PRS__OFFSET 0x4
#define _bar_REG__CFG_REG__INT_PRS__LENGTH 1

// [[FIELD]] temperature interrupt
#define _bar_REG__CFG_REG__INT_TMP__OFFSET 0x5
#define _bar_REG__CFG_REG__INT_TMP__LENGTH 1

// [[FIELD]] fifo full interrupt
#define _bar_REG__CFG_REG__INT_FIFO__OFFSET 0x6
#define _bar_REG__CFG_REG__INT_FIFO__LENGTH 1

// [[FIELD]] data ready interrupt
#define _bar_REG__CFG_REG__INT_HL__OFFSET 0x7
#define _bar_REG__CFG_REG__INT_HL__LENGTH 1

// [[REGISTER]] irq status
#define _bar_REG_INT_STS 0xa

// [[FIELD]] pressure interrupt status
#define _bar_REG__INT_STS__PRS__OFFSET 0x0
#define _bar_REG__INT_STS__PRS__LENGTH 1

// [[FIELD]] temperature interrupt status
#define _bar_REG__INT_STS__TMP__OFFSET 0x1
#define _bar_REG__INT_STS__TMP__LENGTH 1

// [[FIELD]] fifo interrupt status
#define _bar_REG__INT_STS__FIFO__OFFSET 0x2
#define _bar_REG__INT_STS__FIFO__LENGTH 1

// [[REGISTER]] fifo status
#define _bar_REG_FIFO_STS 0xb

// [[FIELD]] empty
#define _bar_REG__FIFO_STS__EMPTY__OFFSET 0x0
#define _bar_REG__FIFO_STS__EMPTY__LENGTH 1

// [[FIELD]] full
#define _bar_REG__FIFO_STS__FULL__OFFSET 0x1
#define _bar_REG__FIFO_STS__FULL__LENGTH 1

// [[REGISTER]] fifo flush or soft reset
#define _bar_REG_RESET 0xc

// [[FIELD]] soft reset
#define _bar_REG__RESET__RST__OFFSET 0x0
#define _bar_REG__RESET__RST__LENGTH 4

// [[FIELD]] flush fifo
#define _bar_REG__RESET__FLUSH__OFFSET 0x1
#define _bar_REG__RESET__FLUSH__LENGTH 1

// [[REGISTER]] fifo flush or soft reset
#define _bar_REG_TEST_ENUM 0xcc

// [[FIELD]] soft reset
#define _bar_REG__TEST_ENUM__RST__OFFSET 0x0
#define _bar_REG__TEST_ENUM__RST__LENGTH 4

// [[FIELD]] flush fifo
#define _bar_REG__TEST_ENUM__FLUSH__OFFSET 0x1
#define _bar_REG__TEST_ENUM__FLUSH__LENGTH 1

#endif // BAR_REGS_H
