Joshua Marchi
Yonatan Feleke
Lab 3

---fdisk---
This is a user program that formats the file system as specified for the lab

	To clean files related to the os and fdisk:
	-- while in the 'flat' directory, type 'cleanfdisk'

	To compile all files related to the os and fdisk:
	-- while in the 'flat' directory, type 'makefdisk'

	To clean, make, AND run fdisk in one command:
	-- while in the 'flat' directory, type 'runfdisk'


--ostest--
This program traps to the OS, and runs several commands to test the file system drivers

	To clean files related to the os and ostest:
	-- while in the 'flat' directory, type 'cleanostest'

	To compile all files related to the os and ostest:
	-- while in the 'flat' directory, type 'makeostest'

	To clean, make, AND run ostest in one command:
	-- while in the 'flat' directory, type 'runostest'
	To clean, make, AND run ostest in one command WITH debugging statements for the buffer cache:
	-- while in the 'flat' directory, type 'runostestwithcache'

	**NOTE** runostest actually runs the simulator TWICE.  The first run, a file is open, and
	several chunks of data are written to it.  The simulator then exists, and is restarted.
	The second run, the file is re-opened, data is read from it and checked against what was
	written, and the file is then deleted.

	**NOTE** since many debug statements are used to prove functionality, it may help to re-direct
	the output text into another file for further review by typing 'runostest > output.txt' or
	'runostestwithcache > output.txt' instead.


--filetest--
These programs test several commands related to the file API

	To clean files related to the os and filetest:
	-- while in the 'flat' directory, type 'cleanfiletest'

	To compile all files related to the os and filetest:
	-- while in the 'flat' directory, type 'makefiletest'

	To clean, make, AND run filetest in one command:
	-- while in the 'flat' directory, type 'runfiletest'
	To clean, make, AND run filetest in one command WITH debugging statements for the buffer cache:
	-- while in the 'flat' directory, type 'runfiletestwithcache'

	*NOTE* runfiletest actually runs the simulator TWICE.  The first run, a file is open, and
	several chunks of data are written to it.  The simulator then exists, and is restarted.
	The second run, the file is re-opened, data is read from it and checked against what was
	written, and the file is then deleted.

	**NOTE** since many debug statements are used to prove functionality, it may help to re-direct
	the output text into another file for further review by typing 'runfiletest > output.txt' or
	'runfiletestwithcache > output.txt' instead.


External sources referenced: Wikipedia for basic information on C syntax and variables
