# C++ Terminal Text Editor Engine

A high-performance, multi-modal terminal text editor engineered from scratch in pure C++. This project bypasses standard GUI frameworks to interact directly with the console hardware, demonstrating advanced memory management, data structure implementation, and real-time algorithmic execution.

### ⚙️ Core Architecture & Features

* **O(1) Memory Engine (Gap Buffer):** Implemented a custom Gap Buffer data structure utilizing raw C++ pointers. This allows for instantaneous, O(1) time complexity for character insertion and deletion, matching the memory architecture used in professional editors like Emacs.
* **Command Design Pattern (Time Machine):** Built a fault-tolerant Undo/Redo history system utilizing dual `std::stack` architectures, logging user keystrokes for deep multi-layered state reversion.
* **Real-Time Lexical Scanner:** Engineered a Phase 1 compiler lexical analyzer that parses strings in real-time, injecting ANSI formatting codes to render dynamic C++ syntax highlighting directly into the terminal stream.
* **Algorithmic Search Engine:** Developed a real-time sliding-window string-matching algorithm to power a global "Find All" search feature (`Ctrl+F`). 
* **Systems Integration:** Implemented low-level Windows API hooks for dynamic hardware cursor tracking, UI overlays, and bare-metal File I/O operations.

### 🚀 Usage
* `Ctrl + O`: Open / Load File
* `Ctrl + S`: Save File
* `Ctrl + F`: Global Search
* `Ctrl + Z`: Undo 
* `Ctrl + Y`: Redo
