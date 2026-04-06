# 🚀 Secure Cloud-Sync Terminal Editor (v2.3)

A high-performance, **Multi-threaded** C++ Text Editor featuring a custom **Gap Buffer** data structure and a secure **Socket-based** Cloud Backup system.



## 🛠️ Key Technical Features
* **Gap Buffer Engine:** Efficient text manipulation with $O(1)$ cursor insertions and deletions.
* **Concurrency:** Multi-threaded C++ Backend (Winsock2) handling multiple simultaneous client uploads without blocking.
* **Custom Protocol:** Implemented a secure application-layer protocol using **Secret-Key Handshaking** (`SATI_PROJ_2026`).
* **Security:** Integrated **Path Traversal Protection**, **Buffer Overflow Guards**, and **Socket Timeouts** (Anti-Slowloris).
* **Undo/Redo System:** Command-pattern implementation using dual-stack architecture for robust state management.



## 📂 Project Structure
```text
├── Editor_Client/          # C++ Terminal UI + Gap Buffer Logic
├── SystemsLabServer/       # Multi-threaded C++ Backend (Secure)
├── storage/                # Cloud-synced files with versioned timestamps
└── README.md               # Documentation
