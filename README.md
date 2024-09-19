# Multi-threaded-Data-Storage-Engine

# Introduction
This project is a multi-threaded data storage engine implemented in C, utilizing a log-structured merge (LSM) tree for data storage. It supports concurrent operations for adding and retrieving key-value pairs using multiple threads, ensuring thread-safe access and performance measurement for each operation. The implementation is based on the Pthreads library in Linux.

# How It Works
The storage engine manages data using an in-memory data structure (memtable) and persistent data on disk organized in Sorted String Tables (SSTs). The system ensures efficient read and write operations using multi-threading. Key-value pairs are first added to the memtable, and once the memtable is full, it is flushed to disk as an SST. The engine supports the add and get operations for key-value storage and retrieval, respectively.

- LSM Tree Structure: Data is kept sorted in memory, and once it is moved to disk, the engine merges overlapping key ranges in SSTs to maintain efficiency.
- Concurrency: Multiple threads can perform read and write operations concurrently, with synchronization mechanisms ensuring consistency.
- Performance Measurements: Execution times for the add and get operations are tracked, and the system supports both random and sequential key access patterns.

  # Usage
  To use the multi-threaded data storage engine, compile the provided source code and run the kiwi-bench executable with different command-line options.

  # Example Commands:
    - ./kiwi-bench write <count>: Perform count write operations.
    - ./kiwi-bench read <count>: Perform count read operations.
    - ./kiwi-bench readwrite <count> <threads> <percentage>: Perform both read and write operations with the specified number of threads and a percentage of write operations.


# Implementation Details
The key functions and components of the system include:
- Memtable: An in-memory sorted data structure (skiplist) used to store key-value pairs before flushing them to disk.
- SST: Sorted String Tables stored on disk after flushing the memtable.
- Thread Management: Utilizes Pthreads for creating multiple threads, ensuring thread-safe operations using locks and condition variables.
- Performance Metrics: Tracks the time taken for each operation and reports statistics on the number of operations performed and the time per operation.
