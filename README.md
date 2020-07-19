# StoreData
========

StoreData is a project to record images and encode data in video sequence
or binary format.
It contains classes for volatile data buffering.

#![StoreData Video Sample](assets/sample.png)

## Getting Started

### Prerequisites

OpenCV 3.x+<br/>
Boost 1.69+<br/>

### Installing

Windows
Run batch file in the configuration folder: configure\win\configure.bat<br/>
The batch file is used to configures the location of the third party libraries.<br/>

Run batch files from folder configure\win in the order (1~2).<br/>
If new third party libraries are installed (i.e. new version of boost), run "configure\win\900. clear cache.bat". Please, note that all the files in build folders are deleted.<br/>

```

0. configure.bat<br/>
Batch file to set the third party libraries location<br/>
1. create_slns.bat<br/>
It creates the projects solution.<br/>
2. build_slns.bat<br/>
It builds the solutions and creates the binary files.<br/>

or by executing cmake from the root folder to compile with favourite compiler.<br/>

```

## Features

- Record asynchronously video data in avi format<br/>
- Record asynchronously video data and encode information in avi format<br/>
- Record asynchronously video and data in binary format (dat)<br/>
- Record asynchronously binary data (sample_record_container)<br/>
- Bufferize in primary memory chunk of data which can persist for a limited amount of time (sample_volatile_timed_buffer)<br/>

## Authors

* **Khoronus** - *Initial work* - [Khoronus](https://github.com/Khoronus)<br/>

## License

This project is licensed under the (see the [LICENSE.md](LICENSE.md) file for details).<br/>
