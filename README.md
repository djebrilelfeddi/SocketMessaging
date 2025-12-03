<div align="center">

<!-- Logo -->
<img src="https://iili.io/fI3HFM7.png" alt="ThreadSocket Logo" width="900"/>

<br/>

### *Real-time messaging. Raw sockets. Pure C++.*

<br/>

[![C++](https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![Linux](https://img.shields.io/badge/Linux-POSIX-FCC624?style=for-the-badge&logo=linux&logoColor=black)](https://www.kernel.org/)
[![TCP](https://img.shields.io/badge/TCP-Sockets-4A90D9?style=for-the-badge&logo=socket.io&logoColor=white)](/)
[![ThreadPool](https://img.shields.io/badge/ThreadPool-Concurrent-9B59B6?style=for-the-badge&logo=threads&logoColor=white)](/)
[![Make](https://img.shields.io/badge/Make-Build-FF6B6B?style=for-the-badge&logo=gnu&logoColor=white)](https://www.gnu.org/software/make/)

<br/>

[Quick Start](#build-and-run) · [Architecture](#architecture-overview) · [Protocol](#message-pipeline) · [Admin](#administrative-shell)

---

</div>

<br/>

# SocketMessaging

SocketMessaging is a C++17 client/server stack that combines a thread-pooled TCP listener, a queued message dispatcher, and an administrative control surface for orchestrating multi-user messaging sessions. The code base targets Linux/POSIX environments and relies on raw sockets and lightweight utilities instead of heavyweight networking frameworks.

## Key Capabilities

- Concurrent server runtime with acceptor, dispatcher, heartbeat, and admin command threads managed through a shared `ThreadPool`.
- Queue-centric message delivery (`Dispatcher`) with configurable back-pressure policies (reject, drop-oldest, drop-newest) and delivery telemetry.
- Runtime administrative shell (`AdminCommandHandler`) that exposes broadcast, targeted messaging, live stats, banlist management, and hot configuration toggles.
- Structured message protocol (`Utils::MessageParser`) that enforces command-delimited payloads and normalises responses (OK, ERROR, USERS, MESSAGE, LOG).
- Built-in heartbeat loop for liveness detection, automatic disconnect handling, and retention of banlists across restarts.
- Verbose/quiet logging pipeline (`Utils::Logger`) with rotating levels, per-process log files, and optional tail retrieval by clients (`GET_LOG`).

## Architecture Overview

- **Server** (`Server::start`) initialises sockets, loads configuration, spins up the dispatcher, thread pool, heartbeat monitor, and admin command loop. Client sockets are tracked with timestamps to back the heartbeat timeout logic.
- **Dispatcher** drains a thread-safe message queue, applying delay policies and ensuring that failed deliveries notify the sender. Broadcasting is implemented by queueing per-recipient messages.
- **Command handler** (`CommandHandler`) validates and routes protocol commands: CONNECT, DISCONNECT, SEND, LIST_USERS, GET_LOG, PING/PONG. It sanitises input, applies banlist checks, and forwards payloads to the dispatcher.
- **Client runtime** (`Client` and `MessageHandler`) wraps POSIX sockets, handles connection negotiation, maintains a listener thread for server events, and exposes callbacks for UI layers (`ClientUI`).
- **Utilities** provide shared services: structured logging, runtime configuration (`RuntimeConfig`), command-line constants, message parsing, string sanitation, and network stream framing with length-prefix and newline delimiters.

## Message Pipeline

1. Clients connect using the `CONNECT;username` request. Authentication enforces unique, validated usernames and checks the banlist.
2. Once registered, SEND commands (`SEND;recipient;subject;body`) are queued via the dispatcher. When `recipient` is `all`, the handler expands the broadcast into one queued message per user.
3. The dispatcher wakes when messages arrive, applies the configured queue policy, and formats the final `MESSAGE;from;subject;body;timestamp` payload for the destination socket.
4. Heartbeat threads issue periodic `PING` frames. Lack of `PONG` responses triggers a timeout path that delegates disconnection workflows to the command handler.
5. Administrator commands (prefixed with `/`) run in a dedicated stdin loop, allowing real-time broadcasts, user management, and runtime configuration adjustments.

## Build and Run

### Prerequisites

- g++ with C++17 support (tested with GCC 12+)
- POSIX sockets (Linux, macOS, WSL)
- Make

### Build targets

```bash
make            # builds bin/server and bin/client
make clean      # removes obj/ and bin/
```

### Launching the server

```bash
./bin/server -p 9000 -c 200 -v
```

- `-p/--port`: override listen port (default 8080)
- `-c/--connections`: cap simultaneous clients (default 100)
- `-v/--verbose`: emit DEBUG-level logs to stdout and `server.log`

The server spawns four background threads: client acceptor, dispatcher, heartbeat monitor, and admin shell. Use `Ctrl+C` to exit gracefully.

### Launching a client

```bash
./bin/client --host 127.0.0.1 --port 9000 --user alice
```

The console client negotiates a username, starts a listener thread, and exposes interactive commands for sending direct or broadcast messages.

## Runtime Administration

The admin console reads commands from standard input (prefix with `/`). Key commands:

- `/help` – list commands and usage
- `/broadcast <message>` – push a system message to all clients
- `/send <user> <message>` – target a specific user
- `/list` – display connected clients
- `/kick <user>` and `/ban <user>` – disconnect or permanently ban a user (persists to `banlist`)
- `/unban <user>` – remove bans
- `/stats` – uptime, counts, and per-minute message rate
- `/config` and `/set <key> <value>` – inspect or adjust runtime settings backed by `RuntimeConfig`
- `/reset` – restore runtime settings to defaults
- `/stop` – request an orderly shutdown

## Configuration Surface

Runtime configuration keys are defined in `RuntimeConfig` and validated against constraints from `Constants`. Examples include:

- `heartbeat.interval` – seconds between PING frames (min 5)
- `heartbeat.timeout` – seconds before a client is considered offline (min 10)
- `dispatcher.delay` – inter-message delay in milliseconds
- `queue.policy` – behaviour when the dispatcher queue is at capacity (`REJECT`, `DROP_OLDEST`, `DROP_NEWEST`)

Changes via `/set` take effect immediately and survive until `/reset` or server restart.

## Logging and Monitoring

- Server logs are written to `server.log` by default; clients log to `client.log`.
- Clients may request the trailing 50 lines of the server log with `GET_LOG`.
- Verbose mode (`-v`) mirrors DEBUG output to the console, aiding local debugging.
- Heartbeat warnings, queue overflows, and dispatcher errors are surfaced through the logger for quick diagnosis.

## Project Layout

```
include/
├─ Client/                 # Client-facing APIs (Client, UI, MessageHandler)
├─ Server/                 # Server core, dispatcher, admin handlers, configuration
└─ Utils/                  # Logging, runtime config, message parsing, thread pool, sockets

src/
├─ main_server.cpp         # CLI entry point for the server daemon
├─ main_client.cpp         # CLI entry point for the console client
├─ Server/                 # Server-side implementations
├─ Client/                 # Client-side implementations
└─ Utils/                  # Shared utilities
```

## Extensibility Notes

- **Transport adapters**: `Network::NetworkStream` encapsulates socket IO; add TLS or framing by extending this layer without rewriting server/client logic.
- **Queue policies**: `DispatcherConfig` defines queue strategies; introduce custom policies (e.g., priority queues) by extending the enum and switch logic.
- **Persistence**: Banlists currently live in a flat file (`banlist`). Swap in a database or remote store by reimplementing `loadBanlist`/`saveBanlist`.
- **UI layers**: `ClientUI` demonstrates a text-based shell. Integrate GUIs or web front-ends by reusing `Client` and `MessageHandler`.
- **Metrics**: Hook into `Server::incrementMessages*` to export data to Prometheus or other monitoring systems.

## Operational Guidance

- Run the server under a service manager (systemd, supervisor) to guarantee restart behaviour and log rotation.
- Expose the admin shell carefully; by default it binds to stdin of the server process—wrap it with SSH or limit shell access in production.
- Harden deployments by running behind a TLS terminator or VPN, and by enforcing stronger authentication in the CONNECT handshake.
- Monitor `banlist` and `server.log` for repeated connection attempts, which may indicate abuse or misconfiguration.

## License

PipelineSocket is released under the MIT License. Review `LICENSE` before redistribution and update copyright notices as needed.
