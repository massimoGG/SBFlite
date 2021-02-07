# SBFlite

### **Introduction**
SBFlite is a stripped down version of [SBFspot](https://github.com/SBFspot/SBFspot/).
Most components have been removed in order to keep it simple to be kept in one C file.

### **What it does**
SBFlite connects via Ethernet to your SMA® solar/battery inverter and reads actual (spot) data. The collected data is stored in a MariaDB SQL database.

### **Why**
I used SBFspot for a while, which successfully exported the data to my MySQL database. Unfortunately this was slow and requested extra data which I didn't need causing it to be slow. When I had some free time, I decided to "redo"/convert this to a single C file while also removing all unnecessarry code (Archival code like Day Data, Month Data, Event Data). 

### **Requirements**
* A C compiler
* libbluetooth-dev
* [libmariadbclient](https://downloads.mariadb.org/connector-odbc/) installed as /usr/lib/mariadb/mysql.h

### **Compilation/Installation**
1. Create a database with two tables "Inverters" and "spotData"
```sql
CREATE Table Inverters (
	Name varchar(32),
	Type varchar(32),
   Serial int(4) NOT NULL,
	TimeStamp timestamp,
	TotalPac int(4),
	EToday int(8),
	ETotal int(8),
	OperatingTime double,
	FeedInTime double,
   Temperature float,
	Status varchar(10),
	GridRelay varchar(10),
);
CREATE Table SpotData (
   Name varchar(32),
	Type varchar(32),
   Serial int(4) NOT NULL,
	TimeStamp timestamp,
   EToday int(8),
	ETotal int(8),
   OperatingTime double,
   FeedInTime double,
   Status varchar(10),
   GridRelay varchar(10),
   Temperature float,
   GridFreq float,
	Pdc1 int(4), Pdc2 int(4),
   Udc1 float, Udc2 float,
	Idc1 float, Idc2 float,
	Pac1 int(4), Pac2 int(4), Pac3 int(4),
	Uac1 float, Uac2 float, Uac3 float,
   Iac1 float, Iac2 float, Iac3 float,
);
```

1. Modify the IP addresses of your SMA® solar/battery inverters and MariaDB username & password inside the sbflite.h file

2. Type `make` to compile the file.
   You can also manually compile this by running:
```bash
gcc -o sbflite -lbluetooth -lmariadbclient sbflite.c 
```

3. Automate the data retrieval with for example: crontab
This example shows how to automaticly run this every 5 minutes
```bash
*/5 * * * * YOURUSERNAME  /path/to/sbflite
```

### **License**
[Attribution - NonCommercial - ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)](https://creativecommons.org/licenses/by-nc-sa/3.0/legalcode)

In short, you are free:
* to Share => to copy, distribute and transmit the work
* to Remix => to adapt the work
Under the following conditions:
* **Attribution:** You must attribute the work in the manner specified by the author or Licensor (but not in any way that suggests that they endorse you or your use of the work).
* **Noncommercial:** You may not use this work for commercial purposes.
* **Share Alike:** If you alter, transform, or build upon this work, you may distribute the resulting work only under the same or similar license to this one.

### **Disclaimer**
A user of SBFlite software acknowledges that he or she is receiving this software on an "as is" basis and the user is not relying on the accuracy or functionality of the software for any purpose. The user further acknowledges that any use of this software will be at his own risk and the copyright owner accepts no responsibility whatsoever arising from the use or application of the software.

SMA, Speedwire are registered trademarks of [SMA Solar Technology AG](http://www.sma.de/en/company/about-sma.html)
