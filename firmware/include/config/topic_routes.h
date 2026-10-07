#ifndef TOPIC_ROUTES_H
#define TOPIC_ROUTES_H

// The MQTT topics the lines travel on, one for each line type that goes through the broker.
// The shape is weather/<device>/<type>. The protocol is described in docs/system/protocol.md.

// Up: published by the bridge when the Nucleo sends the line
#define TOPIC_READINGS "weather/nucleo-01/readings"  // PROTOCOL_LETTER_READING
#define TOPIC_REPLIES  "weather/nucleo-01/replies"   // PROTOCOL_LETTER_REPLY

// Down: the bridge subscribes to these and writes what arrives to the Nucleo
#define TOPIC_ACKS     "weather/nucleo-01/acks"
#define TOPIC_COMMANDS "weather/nucleo-01/commands"  // PROTOCOL_LETTER_COMMAND

#endif
