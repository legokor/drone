
// Register addresses, field offsets and lengths

// [[REGISTER]] pressure data
#define _bar_REG__PSR_B2 0x00
#define _bar_REG__PSR_B1 0x01
#define _bar_REG__PSR_B0 0x02

// [[REGISTER]] temperature data
#define _bar_REG__TMP_B2 0x03
#define _bar_REG__TMP_B1 0x04
#define _bar_REG__TMP_B0 0x05

// [[REGISTER]] pressure config
#define _bar_REG__PRS_CFG 0x06

// [FIELD] precision
#define _bar_REG__PRS_CFG__PREC__OFFSET 0x00
#define _bar_REG__PRS_CFG__PREC__LENGTH 0x03

// [FIELD] rate
#define _bar_REG__PRS_CFG__RATE__OFFSET 0x04
#define _bar_REG__PRS_CFG__RATE__LENGTH 0x03

// [[REGISTER]] temperature config
#define _bar_REG__TMP_CFG 0x07

// [FIELD] precision
#define _bar_REG__TMP_CFG__PREC__OFFSET 0x00
#define _bar_REG__TMP_CFG__PREC__LENGTH 0x03

// [FIELD] rate
#define _bar_REG__TMP_CFG__RATE__OFFSET 0x04
#define _bar_REG__TMP_CFG__RATE__LENGTH 0x03

// [[REGISTER]] mode and status config
#define _bar_REG__MEAS_CFG 0x08

// [FIELD] mode and type
#define _bar_REG__MEAS_CFG__CTRL__OFFSET 0x00
#define _bar_REG__MEAS_CFG__CTRL__LENGTH 0x03

// [FIELD] pressure ready
#define _bar_REG__MEAS_CFG__PRS_READY__OFFSET 0x04
#define _bar_REG__MEAS_CFG__PRS_READY__LENGTH 0x01

// [FIELD] temperature ready
#define _bar_REG__MEAS_CFG__TMP_READY__OFFSET 0x05
#define _bar_REG__MEAS_CFG__TMP_READY__LENGTH 0x01

// [FIELD] sensor ready
#define _bar_REG__MEAS_CFG__SENS_READY__OFFSET 0x06
#define _bar_REG__MEAS_CFG__SENS_READY__LENGTH 0x01

// [FIELD] coefficients ready
#define _bar_REG__MEAS_CFG__COEFF_READY__OFFSET 0x07
#define _bar_REG__MEAS_CFG__COEFF_READY__LENGTH 0x01

// [[REGISTER]] irq and fifo config
#define _bar_REG__CFG_REG 0x09

// [FIELD] 3 / 4 wire mode
#define _bar_REG__CFG_REG__SPI_MODE__OFFSET 0x00
#define _bar_REG__CFG_REG__SPI_MODE__LENGTH 0x01

// [FIELD] fifo enable
#define _bar_REG__CFG_REG__FIFO_EN__OFFSET 0x01
#define _bar_REG__CFG_REG__FIFO_EN__LENGTH 0x01

// [FIELD] pressure result bit-shift
#define _bar_REG__CFG_REG__P_SHIFT__OFFSET 0x02
#define _bar_REG__CFG_REG__P_SHIFT__LENGTH 0x01

// [FIELD] temperature result bit-shift
#define _bar_REG__CFG_REG__T_SHIFT__OFFSET 0x03
#define _bar_REG__CFG_REG__T_SHIFT__LENGTH 0x01

// [FIELD] pressure interrupt
#define _bar_REG__CFG_REG__INT_PRS__OFFSET 0x04
#define _bar_REG__CFG_REG__INT_PRS__LENGTH 0x01

// [FIELD] temperature interrupt
#define _bar_REG__CFG_REG__INT_TMP__OFFSET 0x05
#define _bar_REG__CFG_REG__INT_TMP__LENGTH 0x01

// [FIELD] fifo full interrupt
#define _bar_REG__CFG_REG__INT_FIFO__OFFSET 0x06
#define _bar_REG__CFG_REG__INT_FIFO__LENGTH 0x01

// [FIELD] data ready interrupt
#define _bar_REG__CFG_REG__INT_HL__OFFSET 0x07
#define _bar_REG__CFG_REG__INT_HL__LENGTH 0x01

// [[REGISTER]] irq status
#define _bar_REG__INT_STS 0x0A

// [FIELD] pressure interrupt status
#define _bar_REG__INT_STS__PRS__OFFSET 0x00
#define _bar_REG__INT_STS__PRS__LENGTH 0x01

// [FIELD] temperature interrupt status
#define _bar_REG__INT_STS__TMP__OFFSET 0x01
#define _bar_REG__INT_STS__TMP__LENGTH 0x01

// [FIELD] fifo interrupt status
#define _bar_REG__INT_STS__FIFO__OFFSET 0x02
#define _bar_REG__INT_STS__FIFO__LENGTH 0x01

// [[REGISTER]] fifo status
#define _bar_REG__FIFO_STS 0x0B

// [FIELD] empty
#define _bar_REG__FIFO_STS__EMPTY__OFFSET 0x00
#define _bar_REG__FIFO_STS__EMPTY__LENGTH 0x01

// [FIELD] full
#define _bar_REG__FIFO_STS__FULL__OFFSET 0x01
#define _bar_REG__FIFO_STS__FULL__LENGTH 0x01

// [[REGISTER]] fifo flush or soft reset
#define _bar_REG__RESET 0x0C

// [FIELD] soft reset
#define _bar_REG__RESET__RST__OFFSET 0x00
#define _bar_REG__RESET__RST__LENGTH 0x04

// [FIELD] flush fifo
#define _bar_REG__RESET__FLUSH__OFFSET 0x01
#define _bar_REG__RESET__FLUSH__LENGTH 0x01
