# uC-OS2

## Setup
- install dosbox 0.74
- `vi ~/.dosbox/dosbox-0.74.conf`
- 
```
-	cycles=auto
+	cycles=max

...

+	mount c [base]/uC-OS2
+	c:
+	cd c:\software\ucos-ii
```
## Structure
```
 --SOFTWARE (...)
  -bc45 (Borland C++ compiler)
```
## Issue
```
0	complete        0	1
1	complete        1	2
3	complete        2	3
4	preempt         3	1
5	complete        1	2
7	complete        2	3
8	complete        3	1
9	complete        1	12
10	preempt         12	2
12	complete        2	1
13	complete        1	3
15	preempt         3	2	<- This is OK cuz Tick ISR will reschedule and make task2(deadline=20) preempt task3(deadline=20)
16	preempt         2	1
17	complete        1	2
18	complete        2	3
18	complete        3	12
20	preempt         12	1
```
