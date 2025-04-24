# 🚨 Emergency Service Messaging System

A cross-platform emergency event messaging system implemented as part of the *SPL - Systems Programming* course at Ben-Gurion University.  
The system enables users to connect, subscribe to topics, send and receive messages, and manage state in real-time using the STOMP protocol.

---

## 🔧 Technologies Used

- **Java** (Server side)
- **C++** (Client side, socket programming)
- **STOMP Protocol over TCP**
- **JSON-based message structure**
- **Multithreading**, **Reactor Pattern**
- **Linux Sockets**
- **Git** for version control
- **Docker-compatible**

---

## 💡 Project Structure

### `client/`
- `src/`, `include/`, `bin/`, `data/` – C++ code and headers, build artifacts, and input/output files
- `makefile` – Used to compile the client

### `server/`
- `src/main/java/bgu/spl/net/` – Java server code implementing STOMP server
- `pom.xml` – Maven build file for server

### Root directory
- `events1.json` – Sample input file
- `events1_out.txt` – Output summary file
- `.devcontainer/` – Development environment configuration

```
.
├── client/
│   ├── bin/
│   ├── data/
│   ├── include/
│   ├── src/
│   └── makefile
├── server/
│   └── src/main/java/bgu/spl/net/
│   └── pom.xml
├── events1.json
├── events1_out.txt
├── README.md
```

---

## ✨ Features

- Full support for the **STOMP 1.2 protocol**
- Real-time event broadcasting to subscribers
- Login & session tracking
- Multithreaded client handling (mutex-safe)
- Graceful disconnection & logout
- Custom event summary generation

---

## 🚀 How to Run

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
report ../events1.json
```

#### Terminal 1 - Alice
```bash
summary police Bob ../events1_out.txt
logout
```

#### Terminal 2 - Bob
```bash
logout
```

---

## 🖥️ Client Command Reference

| Command                                | Description |
|----------------------------------------|-------------|
| `login <host>:<port> <user> <pass>`   | Connects to the server and authenticates the client. |
| `join <topic>`                        | Subscribes to a topic (e.g., police, fire, etc.). |
| `exit <topic>`                        | Unsubscribes from a topic. |
| `report <path>`                       | Sends events to the server from a JSON file. |
| `summary <topic> <user> <path>`       | Requests a summary of all events reported by a user on a topic. Output saved to path. |
| `logout`                              | Gracefully disconnects the client from the server. |

### 📝 Example
```bash
report ../events1.json
summary police Bob ../events1_out.txt
```

---

## 📊 Event File Format
Event report files should be in JSON format with the following structure:

```json
{
  "channel_name": "<topic_name>",
  "events": [
    {
      "event_name": "<string>",
      "city": "<string>",
      "date_time": <unix_timestamp>,
      "description": "<string>",
      "general_information": {
        "active": <boolean>,
        "forces_arrival_at_scene": <boolean>
      }
    },
    ...
  ]
}
```

- `channel_name` defines the topic to which the events will be published.
- Each event includes metadata such as name, location, and time.
- The `date_time` field should be a UNIX timestamp (in seconds).
- `general_information.active` indicates whether the event is ongoing.
- `general_information.forces_arrival_at_scene` indicates whether emergency forces arrived at the scene.

You may include multiple events in the `events` array, as shown above.

> Example files can be found under the project root, such as `events1.json`.
Multiple such events can be sent as a JSON array.

---

## 🧪 Testing & Debugging
- All features were manually tested with simulated users.
- Memory safety ensured via `valgrind` and code reviews.
- Server supports debugging printouts under verbose mode.

---

## 🎓 Course Information
- **Course**: SPL - Systems Programming  
- **Institution**: Ben-Gurion University of the Negev  
- **Year**: 2025  
- **Environment**: Linux CS Lab, Docker-compatible  

---

## 🧑‍💻 Authors

**Ben Kapon**  
Student at BGU  
[LinkedIn](https://www.linkedin.com/in/ben-kapon1/)

**Itay Shaul**  
Student at BGU  
[LinkedIn](https://www.linkedin.com/in/itay-shaul/)

---

## 📝 Important Note
This project was designed and tested on a **Docker-compatible environment**.  
Ensure file paths and JSON formats are valid when running locally or in CI environments.

> This project demonstrates a real-time messaging architecture using both low-level socket programming in C++ and scalable Java server design.

