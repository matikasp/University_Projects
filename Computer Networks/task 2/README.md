# The Great Approximator

## Project Description

This proje## Example Messages
- Client: `HELLO $player_id\r\n`
- Server: `COEFF $a_0 $a_1 ... $a_N\r\n`
- Client: `PUT $point $value\r\n`
- Server: `STATE $r_0 ... $r_K\r\n`
- Server: `SCORING $player_id_1 $result_1 ...\r\n`

## Diagnostic Information
Programs output information about game progress and errors to standard output as described in the assignment specification.

## Requirements
- Compilation and execution on Linux systems (e.g., students, laboratories)
- No external networking libraries except BSD sockets
- Code following best practices with error handling

## Author
Project for Computer Networks course.etwork game called "The Great Approximator" in C/C++ using BSD sockets. The project consists of two programs: a server (`approx-server`) and a client (`approx-client`). The goal of the game is for each player (client) to best approximate a polynomial received from the server.

## Files
- `server.cpp` — server implementation
- `client.cpp` — client implementation
- `common.cpp`, `common.h` — shared functions and definitions
- `Makefile` — project build file

## Compilation
To build the project, use the command:

```
make
```

This will create two executable files: `approx-server` and `approx-client`.

To remove files created during compilation, use:

```
make clean
```

## Usage

### Server

```
./approx-server -f <coefficients_file> [options]
```

**Options:**
- `-p <port>` — server port number (default 0)
- `-k <value>` — constant K value (default 100)
- `-n <value>` — polynomial degree N (default 4)
- `-m <value>` — number of additions M (default 131)
- `-f <file>` — file with COEFF messages (required)

### Client

```
./approx-client -u <player_id> -s <server> -p <port> [options]
```

**Options:**
- `-u <player_id>` — player identifier (required)
- `-s <server>` — server address/name (required)
- `-p <port>` — server port (required)
- `-4` — force IPv4
- `-6` — force IPv6
- `-a` — automatic mode (strategy better than random)

## Communication Protocol

Communication is text-based over TCP (IPv4/IPv6). Each message ends with `\r\n`. Protocol details, message formats, and error handling are described in the assignment specification.

## Example Messages
- Client: `HELLO $player_id\r\n`
- Server: `COEFF $a_0 $a_1 ... $a_N\r\n`
- Client: `PUT $point $value\r\n`
- Server: `STATE $r_0 ... $r_K\r\n`
- Server: `SCORING $player_id_1 $result_1 ...\r\n`

## Diagnostic Information
Programs output information about game progress and errors to standard output as described in the assignment specification.

## Requirements
- Compilation and execution on Linux systems (e.g., students, laboratories)
- No external networking libraries except BSD sockets
- Code following best practices with error handling
