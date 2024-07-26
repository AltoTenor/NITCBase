# NITCbase: RDBMS Implementation

Welcome to my NITCbase project repository! This project is an implementation of a Relational Database Management System (RDBMS) designed for educational purposes. It follows an eight-layer architecture to cover essential RDBMS functionalities.

## Project Overview

NITCbase includes:

- **Basic RDBMS Operations**: Table creation, record insertion, selection queries.
- **Indexing**: B+ Tree-based indexing.
- **Supported Commands**: `CREATE`, `DROP`, `ALTER`, `INSERT`, `SELECT`, `PROJECT`, `JOIN`, `CREATE INDEX`, `DROP INDEX`.

## Project Structure

- **Physical Layer**: `Disk.cpp` - Low-level disk operations.
- **Buffer Layer**: `StaticBuffer.cpp`, `BlockBuffer.cpp` - Disk buffering.
- **B+ Tree Layer**: `BPlusTree.cpp` - Indexing.
- **Block Access Layer**: `BlockAccess.cpp` - Data manipulation operations.
- **Cache Layer**: `OpenRelTable.cpp`, `RelCacheTable.cpp`, `AttrCacheTable.cpp` - Runtime data structures.
- **Algebra Layer**: `Algebra.cpp` - DML commands.
- **Schema Layer**: `Schema.cpp` - DDL commands.
- **Frontend Interface**: `FrontendInterface.cpp`, `Frontend.cpp` - User interaction.

## Getting Started

1. **Clone the Repository**:
    ```bash
    git clone https://github.com/your-username/nitcbase.git
    ```

2. **Build the Project**:
    Follow the build instructions in the documentation.


## Roadmap

Refer to the [Roadmap](#) for a step-by-step guide to implementing each layer.


## Feedback

For any questions or feedback, please open an issue or contact me directly.
