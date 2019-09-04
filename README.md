# StoreData
========

StoreData is a project to record images and encode data in video sequence
or binary format.
It contains classes for volatile data buffering.

#![StoreData Video Sample](assets/sample.png)

Features
--------

- Record asynchronously video data in avi format
- Record asynchronously video data and encode information in avi format
- Record asynchronously video and data in binary format (dat)
- Record asynchronously binary data (sample_record_container)
- Bufferize in primary memory chunk of data which can persist for a limited amount of time (sample_volatile_timed_buffer)

Installation
------------

Build by running:

    configure/win/0. configure.bat
    configure/win/1. create_slns.bat
    configure/win/2. build_slns.bat

	or by executing cmake from the root folder to compile with favourite compiler.
	The cmake script requires the path config created by the batch file:
	    configure/win/0. configure.bat

Contribute
----------

- Issue Tracker: 
- Source Code: 

License
-------

Please check the LICENSE file.
