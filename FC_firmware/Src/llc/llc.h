#ifndef LLC_H
#define LLC_H

typedef struct [[nodiscard]] llc_ThrustVec {
    struct {
        float thrust, roll, pitch, yaw;
    };

    float arr[3];
} llc_ThrustVec;

// TODO: const setters

void llc_init(void);

[[nodiscard]]
llc_ThrustVec llc_update(llc_ThrustVec);

#endif // LLC_H
