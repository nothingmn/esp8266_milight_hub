#include <V2PacketFormatter.h>
#include <V2RFEncoding.h>

#define GROUP_COMMAND_ARG(status, groupId, numGroups) ( groupId + (status == OFF ? (numGroups + 1) : 0) )

V2PacketFormatter::V2PacketFormatter(uint8_t protocolId, uint8_t numGroups)
  : PacketFormatter(9),
    protocolId(protocolId),
    numGroups(numGroups)
{ }

bool V2PacketFormatter::canHandle(const uint8_t *packet, const size_t packetLen) {
  uint8_t packetCopy[V2_PACKET_LEN];
  memcpy(packetCopy, packet, V2_PACKET_LEN);
  V2RFEncoding::decodeV2Packet(packetCopy);
  return packetCopy[V2_PROTOCOL_ID_INDEX] == protocolId;
}

void V2PacketFormatter::initializePacket(uint8_t* packet) {
  size_t packetPtr = 0;

  // Always encode with 0x00 key. No utility in varying it.
  packet[packetPtr++] = 0x00;

  packet[packetPtr++] = protocolId;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = sequenceNum++;
  packet[packetPtr++] = groupId;
  packet[packetPtr++] = 0;
}

void V2PacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  if (held) {
    command |= 0x80;
  }
  currentPacket[V2_COMMAND_INDEX] = command;
  currentPacket[V2_ARGUMENT_INDEX] = arg;
}

void V2PacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(0x01, GROUP_COMMAND_ARG(status, groupId, numGroups));
}

void V2PacketFormatter::unpair() {
  for (size_t i = 0; i < 5; i++) {
    updateStatus(ON, 0);
  }
}

void V2PacketFormatter::finalizePacket(uint8_t* packet) {
  V2RFEncoding::encodeV2Packet(packet);
}

void V2PacketFormatter::format(uint8_t const* packet, char* buffer) {
  buffer += sprintf_P(buffer, PSTR("Raw packet: "));
  for (int i = 0; i < packetLength; i++) {
    buffer += sprintf_P(buffer, PSTR("%02X "), packet[i]);
  }

  uint8_t decodedPacket[packetLength];
  memcpy(decodedPacket, packet, packetLength);

  V2RFEncoding::decodeV2Packet(decodedPacket);

  buffer += sprintf_P(buffer, PSTR("\n\nDecoded:\n"));
  buffer += sprintf_P(buffer, PSTR("Key      : %02X\n"), decodedPacket[0]);
  buffer += sprintf_P(buffer, PSTR("b1       : %02X\n"), decodedPacket[1]);
  buffer += sprintf_P(buffer, PSTR("ID       : %02X%02X\n"), decodedPacket[2], decodedPacket[3]);
  buffer += sprintf_P(buffer, PSTR("Command  : %02X\n"), decodedPacket[4]);
  buffer += sprintf_P(buffer, PSTR("Argument : %02X\n"), decodedPacket[5]);
  buffer += sprintf_P(buffer, PSTR("Sequence : %02X\n"), decodedPacket[6]);
  buffer += sprintf_P(buffer, PSTR("Group    : %02X\n"), decodedPacket[7]);
  buffer += sprintf_P(buffer, PSTR("Checksum : %02X"), decodedPacket[8]);
}

uint8_t V2PacketFormatter::groupCommandArg(MiLightStatus status, uint8_t groupId) {
  return GROUP_COMMAND_ARG(status, groupId, numGroups);
}

// helper method to return a bulb to the prior state
void V2PacketFormatter::switchMode(GroupState currentState, BulbMode desiredMode) {
  // revert back to the prior mode
  switch (desiredMode) {
    case BulbMode::BULB_MODE_COLOR:
      updateHue(currentState.getHue());
      break;
    case BulbMode::BULB_MODE_NIGHT:
      enableNightMode();
      break;
    case BulbMode::BULB_MODE_SCENE:
      updateMode(currentState.getMode());
      break;
    case BulbMode::BULB_MODE_WHITE:
      updateColorWhite();
      break;
    default:
      Serial.printf_P(PSTR("V2PacketFormatter::switchMode: Request to switch to unknown mode %d\n"), desiredMode);
      break;
  }
  
}
