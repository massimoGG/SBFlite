# SBFlite

### **Introduction**
SBFlite is a stripped down version of [SBFspot](https://github.com/SBFspot/SBFspot/).
Most components have been removed in order to keep it simple to be kept in one C file.

### **What it does**
SBFlite connects via Ethernet to your SMA® solar/battery inverter and reads actual (spot) data. The collected data is stored in a SQLite/MariaDB SQL database.

### **Why**
I used SBFspot for a while, which successfully exported the data to my MySQL database. Unfortunately this was slow and requested extra data which I didn't need causing it to be slow. When I had some free time, I decided to "redo"/convert this to a single C file while also removing all unnecessarry code (Archival code like Day Data, Month Data, Event Data). 

### **Requirements**
* A C compiler
* libbluetooth-dev
* A MySQL/MariaDB database

### **Compilation**
1. Modify the IP addresses of your SMA® solar/battery inverters and MySQL username & password inside the sbflite.h file

2. Type `make` to compile the file.

3. Automate this with for example: crontab
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
