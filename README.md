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
