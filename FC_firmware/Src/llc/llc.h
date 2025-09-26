#ifndef LLC_H
#define LLC_H

typedef struct [[nodiscard]] llc_ThrustVec {
    struct {
        float thrust, roll, pitch, yaw;
    };

    float arr[3];
} llc_ThrustVec;

void llc_set_consts(float roll_p,
                    float roll_i,
                    float roll_d,
                    float pitch_p,
                    float pitch_i,
                    float pitch_d,
                    float yaw_p,
                    float yaw_i,
                    float yaw_d);

void llc_init(void);

llc_ThrustVec llc_update(llc_ThrustVec);

#endif // LLC_H
