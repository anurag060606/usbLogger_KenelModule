# Kernel-Level USB Device Monitoring and Logging System

Hi hello 

This project logs USB connect/disconnect events in Linux using a kernel module, exposes those events through `/dev/devlogger`, and stores them in a MySQL database using a Python automation script.

---

## File Structure

- `devlogger.c` — main loadable kernel module  
- `collector.py` — user-space automation script that reads events and logs them into MySQL  
- `Makefile` — builds the kernel module

---

## How It Works

```text
USB insert/remove
↓
Kernel module detects event
↓
Event exposed via /dev/devlogger
↓
Python collector reads event in user space
↓
Stored in MySQL database
```

---

# Setup

Install dependencies:

```bash
sudo apt install build-essential linux-headers-$(uname -r)
```

Compile kernel module:

```bash
make
```

Load the module:

```bash
sudo insmod devlogger.ko
```

Check if it loaded:

```bash
ls /dev/devlogger
```

---

# Setup Database

Open **Terminal 1** and start MySQL:

```bash
mysql -u root -p
```

Create database and table:

```sql
CREATE DATABASE usblogs;
USE usblogs;

CREATE TABLE logs(
 id INT AUTO_INCREMENT PRIMARY KEY,
 timestamp DOUBLE,
 action VARCHAR(20),
 vid VARCHAR(10),
 pid VARCHAR(10),
 latency_ms DOUBLE
);
```

---

# Important Change

Before running the collector:

Open `collector.py`

Update the database credentials (lines with `user=` and `password=`):

```python
user="your_mysql_user"
password="your_password"
```

(Using `root` is fine for testing.)

---

# Run the Project (Use Two Terminals)

## Terminal 1
Keep MySQL open for queries.

## Terminal 2
Run the collector:

```bash
python3 collector.py
```

Plug or unplug USB devices.

Events will be detected and stored automatically.

---

# View Data / Analytics

Go back to **Terminal 1** (told you not to close it >:/ )

## View all logs

```sql
SELECT * FROM logs;
```

## Average latency

```sql
SELECT AVG(latency_ms) FROM logs;
```

## Min and max latency

```sql
SELECT MIN(latency_ms), MAX(latency_ms)
FROM logs;
```

## Device frequency

```sql
SELECT vid,pid,COUNT(*)
FROM logs
GROUP BY vid,pid;
```

---

# Remove Module

When done:

```bash
sudo rmmod devlogger
```

---

# Features

- USB event detection  
- Kernel character device (`/dev/devlogger`)  
- MySQL logging  
- Latency / throughput metrics  
- Device activity analytics