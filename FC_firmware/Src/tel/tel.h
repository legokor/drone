#ifndef TEL_H 
#define TEL_H

typedef enum tel_Destination {
    usb = 0b1,
    lora = 0b10,
    sd = 0b100,
} tel_LogDestination;

typedef struct tel_Tel {
    tel_LogDestination destination;
} tel_Tel;

void tel_Init(tel_Tel* tel);

#endif // TEL_H
