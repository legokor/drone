#ifndef CTRL_H
#define CTRL_H

typedef enum [[nodiscard]] ctrl_Mode {
    ctrl_Idle,
    ctrl_RC,

    //  (normal / abort)
    ctrl_SoftLanding,
    ctrl_GPS_Waypoint

} ctrl_Mode;

void ctrl_init(void);

void ctrl_setMode(ctrl_Mode mode);
[[nodiscard]] ctrl_Mode ctrl_getMode(void);

#endif // CTRL_H
