# Peer-to-Peer Clock Synchronization Network

## Overview

This program implements a peer-to-peer clock synchronization system. The synchronization network consists of equal peer nodes. Each node communicates with other nodes to synchronize its clock, taking into account network packet transmission delays.

## Network Operations

The clock synchronization network includes the following operations:

1. **Node joining** - A node joins the network by contacting another node
2. **Leader election** - Selection of a leader to which nodes synchronize their clocks
3. **Clock synchronization** - Synchronizing clocks while accounting for packet transmission delays

## Node Properties

### Natural Clock
Each node maintains its natural clock, which is the number of milliseconds since its startup.

### Synchronization Level
The synchronization level of a node can be:
- **255** - Node is not synchronized with any node
- **0** - Node is the synchronization source for other nodes (is the leader)
- **1** - Node is synchronized with the leader
- **2** - Node is synchronized with a node that is synchronized with the leader
- **...** - And so on, up to a maximum of **254**

Each node stores its synchronization level (initial value: 255) and remembers which node it is synchronized with.

### Communication Protocol
Nodes communicate using **UDP** and **IPv4** addressing.

## Command Line Parameters

The node program accepts the following command-line parameters:

- `-b bind_address` - IP address on which the node listens (optional, default: all host addresses)
- `-p port` - Port on which the node listens, number from 0 to 65535, zero means any available port (optional, default: 0)
- `-a peer_address` - IP address or hostname of another node to communicate with (optional)
- `-r peer_port` - Port of another node to communicate with, number from 1 to 65535 (optional)

**Notes:**
- Parameters can be provided in any order
- Parameters `-a` and `-r` must both be provided together
- If a parameter is provided multiple times, the behavior should be reasonable

## Message Format

Messages exchanged between nodes may contain the following fields:

- `message` - 1 octet, message type
- `count` - 2 octets, number of known nodes
- `peer_address_length` - 1 octet, number of octets in peer_address field
- `peer_address` - peer_address_length octets, node IP address in IP header format
- `peer_port` - 2 octets, port number on which the node listens
- `timestamp` - 8 octets, time, clock value
- `synchronized` - 1 octet, node synchronization level

**All multi-octet binary field values are written in network byte order.**

Nodes send messages from the port on which they listen. Nodes obtain information about the sender's IP address and port number from the network layer. The IP address and port number uniquely identify a node.

## Network Join Protocol

A node can join the network in two ways:

1. **Without `-a` and `-r` parameters** - The node listens for messages and waits for new participants
2. **With `-a` and `-r` parameters** - The node sends a HELLO message to the specified node

### Join Messages

- **HELLO** - `message = 1`
  - Informs another node of the desire to establish communication

- **HELLO_REPLY** - `message = 2`, `count`, followed by records containing `peer_address_length`, `peer_address`, `peer_port`
  - Response to HELLO message
  - Informs the new node about other active nodes in the network
  - The list does not include the sender or receiver of this message
  - New node sends CONNECT to each learned node

- **CONNECT** - `message = 3`
  - Informs about the desire to establish communication with another node
  - Receiving node adds the sender to its node list

- **ACK_CONNECT** - `message = 4`
  - Response to CONNECT message
  - Confirms communication establishment
  - Node adds nodes that confirmed communication to its list

**Note:** Nodes that exchanged HELLO and HELLO_REPLY have established communication and do not exchange CONNECT and ACK_CONNECT messages.

## Clock Synchronization Protocol

The clock synchronization process involves exchanging three messages:

- **SYNC_START** - `message = 11`, `synchronized`, `timestamp`
- **DELAY_REQUEST** - `message = 12`
- **DELAY_RESPONSE** - `message = 13`, `synchronized`, `timestamp`

### Synchronization Process

1. **SYNC_START** is sent periodically (every 5-10 seconds) by all nodes with synchronization level < 254 to all known nodes. Contains the sender's synchronization level and current clock value **T1**.

2. A node responds to SYNC_START only when:
   - The sender is known
   - The sender has synchronization level < 254
   - The sender's synchronization level is less than its own level (if sender is the node it's synchronized with)
   - The sender's synchronization level is at least 2 less than its own level (if sender is not the node it's synchronized with)

3. Node continues synchronization only with the first node that meets the above conditions.

4. If a node doesn't receive SYNC_START from its synchronized node for 20-30 seconds, or receives one with `synchronized` ≥ its own level, it changes its synchronization level to 255.

5. When continuing synchronization:
   - Node records its clock value **T2** when receiving SYNC_START
   - Sends DELAY_REQUEST
   - Records its clock value **T3** when sending DELAY_REQUEST

6. The synchronizing node, upon receiving DELAY_REQUEST, sends DELAY_RESPONSE containing its synchronization level and clock value **T4** when receiving DELAY_REQUEST.

7. The synchronized node calculates:
   ```
   offset = (T2 - T1 + T3 - T4) / 2
   ```

8. The node is now synchronized at a level one greater than the `synchronized` value received in SYNC_START and DELAY_RESPONSE (these values must be identical).

**Timeouts:**
- If no DELAY_REQUEST or DELAY_RESPONSE is received within 5-10 seconds, the synchronization procedure is abandoned and such messages received later are ignored.

**Synchronized Time:**
If the node sending T1 and T4 has a clock synchronized with another node, it sends the synchronized time values.

## Leader Election

At least one node must become a leader for synchronization to begin.

- **LEADER** - `message = 21`, `synchronized`

### Leader Message Behavior

- If `synchronized = 0`:
  - Node becomes leader (sets synchronization level to 0)
  - After 2 seconds, starts sending synchronization messages to all known nodes

- If `synchronized = 255` and node is a leader:
  - Node stops being leader (sets synchronization level to 255)
  - Stops sending synchronization messages (but completes ongoing exchanges)

**Notes:**
- Other values in the `synchronized` field are invalid
- A message with value 255 received by a non-leader node is invalid
- A node should respond to every LEADER message from startup without verifying the sender

## Time Information Service

Every node should provide current time information using:

- **GET_TIME** - `message = 31`
  - Request for current time

- **TIME** - `message = 32`, `synchronized`, `timestamp`
  - Response to GET_TIME
  - Contains synchronization level and natural clock value if unsynchronized
  - Contains clock value corrected by offset if synchronized

**Note:** A node should respond to every GET_TIME message from startup without verifying the sender.

## Error Handling

### Parameter Validation
The program should thoroughly validate parameters. Detailed error information should be written to standard error output, and the program should exit with code 1.

### System/Library Function Errors
Errors from system or library function calls should be handled by writing appropriate information to standard error output. If an error prevents further program operation, exit with code 1. Errors that allow continued operation (e.g., invalid messages, ignored messages) should not interrupt program execution.

### Invalid Messages
A message is considered invalid when:
- The sender is an unknown node (except for exceptions described above)
- It's unexpected in the current communication state
- It contains an invalid or unexpected field value

### Error Output Format
- Error messages to stderr should start with `ERROR`
- Invalid message errors should start with `ERROR MSG` followed by a space and up to 10 initial bytes of the message in hexadecimal

**Example:**
```
ERROR MSG 7a12c534
```

## Additional Clarifications

1. The `count` field size in HELLO_REPLY limits the number of nodes to 65535. Messages that would exceed this limit should be ignored. If a HELLO_REPLY message is too large to send, it should be discarded and treated like other ignored messages.

2. A HELLO_REPLY message should be considered entirely invalid if:
   - It doesn't contain `count` records
   - Any `peer_address_length` or `peer_port` field has an invalid value
   - The node list includes the sender or receiver of the message

## Implementation Requirements

### Technology Stack
- Language: **C** or **C++**
- Network API: **Socket interface** only (no other networking libraries)
- Architecture: **Single-threaded** (but communication with one node should not block communication with others)

### Code Quality
Programs should be written according to best practices. Obvious expectations (e.g., not formatting the disk, checking return values of system functions) are assumed even if not explicitly stated. The code quality and protocol implementation will be rigorously tested.

## Building and Running

### Build
```bash
make
```
This creates the executable `peer-time-sync`.

### Clean
```bash
make clean
```
This removes all files created during compilation.

### Running Examples

Start a node as initial peer (listens on all interfaces, any port):
```bash
./peer-time-sync
```

Start a node connecting to an existing peer:
```bash
./peer-time-sync -a 192.168.1.100 -r 5000
```

Start a node on specific address and port:
```bash
./peer-time-sync -b 127.0.0.1 -p 5001
```

## Submission Requirements

Submit an archive containing files necessary to build the solution:
- **Do not include** binary files or other unnecessary files
- **Archive formats:** .zip, .rar, .7z, or .tgz
- After unpacking, all files should be in the directory where the archive is located
- **No subdirectories** in the archive
- Must contain `makefile` or `Makefile`
- `make` should create executable `peer-time-sync`
- `make clean` should remove all compilation artifacts

## Project Structure

```
.
├── main.cpp           # Main program entry point
├── network.cpp        # Network communication implementation
├── network.hpp        # Network communication header
├── message.hpp        # Message format definitions
├── Makefile          # Build configuration
└── README.MD         # This file
```

## License

This project is part of a Computer Networks university course assignment.
