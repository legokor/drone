#ifndef TEL_H
#define TEL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Data flow of the system:
 *
 *   +----------+--------+--------+---------+----------+
 *   | What     | From   | To     | Through | Priority |
 *   +----------+--------+--------+---------+----------+
 * 1 | Commands | ground | drone  | LoRa    | High     |
 *   +----------+--------+--------+---------+----------+
 * 2 | Sensors  | drone  | ground | LoRa    | Medium   |
 *   +----------+--------+--------+---------+----------+
 * 3 | Sensors  | drone  | drone  | SD card | Medium   |
 *   +----------+--------+--------+---------+----------+
 * 4 | Logs     | drone  | ground | LoRa    | Low      |
 *   +----------+--------+--------+---------+----------+
 * 5 | Logs     | drone  | drone  | SD card | Low      |
 *   +----------+--------+--------+---------+----------+
 * 6 | Logs     | drone  | ground | USB     | Very Low |
 *   +----------+--------+--------+---------+----------+
 * 7 | Sensors  | drone  | ground | USB     | Very Low |
 *   +----------+--------+--------+---------+----------+
 *
 * Usecases:
 *  - critical: sending / recieving commands from the ground station
 *  - non-critical: logging
 *
 * The telemetry system works by sending / writing messages. Messages can be of different datatypes and topics.
 * Timestamps are always attached.
 *
 * Format:
 *   +-----+----------------+----------+--------------+
 *   | Idx | Name           | Type     | Size (bytes) |
 *   +-----+----------------+----------+--------------+
 *   |  0  | timestamp (us) | uint     | 4            |
 *   +-----+----------------+----------+--------------+
 *   |  1  | datatype       | uint     | 1            |
 *   +-----+----------------+----------+--------------+
 *   |  2  | topic          | uint     | 1            |
 *   +-----+----------------+----------+--------------+
 *   |  3  | data           | [???]    | [???]        |
 *   +-----+----------------+----------+--------------+
 *
 * Datatypes:
 *   +------+----------+----------+--------------+
 *   | Code | Name     | C Type   | Size (bytes) |
 *   +------+----------+----------+--------------+
 *   | 0x00 | ping     | void     | 0            |
 *   +------+----------+----------+--------------+
 *   | 0x01 | integer  | uint64_t | 4            |
 *   +------+----------+----------+--------------+
 *   | 0x02 | float    | double   | 8            |
 *   +------+----------+----------+--------------+
 *   | 0x03 | char     | char     | 1            |
 *   +------+----------+----------+--------------+
 *   | 0x04 | boolean  | bool     | 1            |
 *   +------+----------+----------+--------------+
 *   | 0x05 | array    | ???[]    | 1 + 1 + ?    | (*)
 *   +------+----------+----------+--------------+
 *
 * (*) if the datatype is array, then [data] contains
 *  - [0-7] the datatype of the elements of the array
 *  - [8-15] and the length of the array
 */

typedef uint8_t tel_Topic;

typedef enum tel_DataType { tel_TYPE_PING, tel_TYPE_INTEGER, tel_TYPE_FLOAT, tel_TYPE_CHAR, tel_TYPE_BOOLEAN, tel_TYPE_ARRAY } tel_DataType;

typedef void (*tel_WriteFn)(const void*, size_t);

void tel_Init(void);

void tel_AddSource(tel_WriteFn writeFn);
void tel_RemoveSource(tel_WriteFn writeFn);

void tel_WritePing(tel_Topic topic);
void tel_WriteInteger(tel_Topic topic, uint32_t i);
void tel_WriteFloat(tel_Topic topic, double d);
void tel_WriteChar(tel_Topic topic, char c);
void tel_WriteBoolean(tel_Topic topic, bool b);

void tel_WriteArray(tel_Topic topic, const void* arr, uint8_t len, tel_DataType type);

void tel_WriteString(tel_Topic topic, const char* str);

#endif // TEL_H
