# 🚨 Emergency Service Messaging System

A cross-platform emergency event messaging system implemented as part of the *SPL - Systems Programming* course at Ben-Gurion University.  
The system enables users to connect, subscribe to topics, send and receive messages, and manage state in real-time using the STOMP protocol.

## 🔧 Technologies Used

-	**Java** (Server side)
-	**C++** (Client side, socket programming)
-	STOMP Protocol over TCP
-	JSON-based message structure
-	Multithreading, Reactor Pattern
-	Linux Sockets
- Git for version control
- Docker-compatible
  
## 💡 Project Structure

- **Client (C++)**  
  - Handles user input, message parsing, STOMP framing, connection lifecycle, and multithreaded I/O.
  - Supported commands: `login`, `subscribe`, `send`, `report`, `logout`, `summary`.

- **Server (Java)**  
  - Based on `bgu.spl.net.srv.Server`.
  - Supports two threading models: **Thread Per Client (TPC)** and **Reactor** using Java NIO.
  - Manages client sessions, topic subscriptions, and frame routing.

## ✨ Features

- Full support for the STOMP 1.2 protocol.
- Real-time event broadcasting to subscribers.
- Login & session tracking.
- Multithreaded client handling (mutex-safe).
- Graceful disconnection & logout.
- Custom event summary generation.


## How to Run

### Server

Navigate to the `server/` directory and run:

```bash
mvn clean compile
mvn exec:java -Dexec.mainClass="bgu.spl.net.impl.stomp.StompServer" -Dexec.args="7777 tpc"
# or for reactor:
mvn exec:java -Dexec.mainClass="bgu.spl.net.impl.stomp.StompServer" -Dexec.args="7777 reactor"
```

### Client

From the `client/` directory:

```bash
make
cd bin
./StompEMIClient
```

### Example Run

Each client should be started in a separate terminal.

#### Terminal 1 - Alice

```bash
login 127.0.0.1:7777 Alice 123
join police
```

#### Terminal 2 - Bob

```bash
login 127.0.0.1:7777 Bob abc
join police
report {PATH TO events1.json}
```

#### Terminal 1 - Alice again

```bash
summary police Bob {PATH TO OUTPUT FILE (e.g. events1_out.txt)}
logout
```

#### Terminal 2 - Bob

```bash
logout
```

## 🧪 Tests & Debugging

- All features were manually tested with simulated users.
- Memory safety ensured via `valgrind` and code reviews.
- Server supports debugging printouts under verbose mode.

## 🎓 Course Information

- **Course**: SPL - Systems Programming
- **Institution**: Ben-Gurion University of the Negev
- **Year**: 2025
- **Environment:** Linux CS Lab, Docker-compatible  

## How to Build

1. Navigate to `client/` and compile the C++ client using `make`.
2. Navigate to `server/` and run the Java server using your preferred build system (e.g., IntelliJ, Maven).
3. Communication follows the STOMP protocol using TCP sockets.
   

## 🧑‍💻 Authors

**Ben Kapon**  
Student at BGU  
[LinkedIn](https://www.linkedin.com/in/ben-kapon1/)

**Itay Shaul**  
Student at BGU  
[LinkedIn](https://www.linkedin.com/in/itay-shaul/)

## Important note
  **This project was designed and tested on a Docker-compatible environment.**

