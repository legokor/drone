#ifndef LLC_H
#define LLC_H

typedef struct [[nodiscard]] llc_ThrustVec {
    struct {
        float thrust, roll, pitch, yaw;
    };

    float arr[3];
} llc_ThrustVec;

void llc_init(void);

llc_ThrustVec llc_update(llc_ThrustVec);

#endif // LLC_H
