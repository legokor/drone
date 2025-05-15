#ifndef CTRL_H
#define CTRL_H

typedef enum ctrl_Mode {
    ctrl_Idle,
    ctrl_RC,

    //  (normal / abort)
    ctrl_SoftLanding,
    ctrl_GPS_Waypoint

} ctrl_Mode;

void ctrl_Init(void);

void ctrl_SetMode(ctrl_Mode mode);
ctrl_Mode ctrl_GetMode(void);

#endif // CTRL_H
