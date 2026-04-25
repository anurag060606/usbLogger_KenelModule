Hi hello 
This project is a kernel level USB device monitoring and logging system
which means it logs usb connect/disconnect events in linux using a kernel module, exposes them through /dev/devlogger and stores them into a mysql database using python automation script

## File structure
- `devlogger.c` : the main loadable kernel module
- `collector.py` : the automation script that reads events and logs them in the mysql database
- `Makefile` : builds the kernel module

---

##How it works

```text
usb insert/remove -> kernel module detects event -> event exposed via /dev/devlogger file -> python collector reads events from the user space -> stores in mysql database
```
---

##How to make it run in your linux 
```bash
sudo apt install build-essential linux-headers-$(uname -r)
```

Compile:
```bash 
make 
```

Load the Loadable Kernel Module
```bash
sudo insmod devlogger.ko
```

Check if it's installed properly
```bash 
ls /dev/devlogger
```

Setup the database using mysql
```bash
mysql -u root -p
```

Create database and table in mysql:

```sql
create database usblogs;
use usblogs;

create table logs(
	id int auto_increment primary key, 
	timestamp double,
	action varchar(20),
	vid varchar(10),
	pid varchar(10), 
	latency_ms double
);
---

##Important changes to make

You must change the username and password in the collector.py script so the script can access the mySQL
Open the collector.py in your editor and come to lines 6 and 7
Set your user and password (Tip: You can use root as user if you are not sure) 
---

## Run collector.py script

Open a new terminal (let the mySQL terminal keep running) and run the following command: 
```bash
python3 collector.py
```

Plug or unplug USBs 
These events will get logged into the database 

---

## View data and analytics

Go to the mySQL terminal (I told you not to close that >:( )and type the following queries for analytics:
### View all logs
```sql

select * from logs;
```
### Get average latency

```sql
select avg(latency_ms) from logs;
```
---

## Remove module
```bash 
sudo rmmod devlogger
```

---

##Features

- USB event detection
- Kernel Character Device
- mySQL logging
- Latency and throughput metrics
- Device activity analytics

