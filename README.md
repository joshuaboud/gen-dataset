# gen-dataset
A command line tool to quickly generate a lot of files in a lot of directories. This tool creates an [M-ary tree](https://en.wikipedia.org/wiki/M-ary_tree)
shaped directory tree and randomly places any number of files of any size within this tree. The distribution of files per directory is roughly equal. If a size is provided, the files will be filled with zeros up to that size.

## Installation
### Precompiled Static Binary
* Download Binary
  - curl
  ```sh
  sudo curl https://github.com/joshuaboud/gen-dataset/releases/download/v1.2/gen-dataset -o /usr/local/bin/gen-dataset
  ```
  - wget
  ```sh
  sudo wget https://github.com/joshuaboud/gen-dataset/releases/download/v1.2/gen-dataset -P /usr/local/bin
  ```
* Mark Executable
  ```sh
  sudo chmod +x /usr/local/bin/gen-dataset
  ```
### From Source
* Install Boost Development Libraries
* Get Source and Install
  ```sh
  git clone https://github.com/joshuaboud/gen-dataset.git
  cd gen-dataset
  make -j8
  sudo make install
  ```

### Usage
```
usage:
  gen-dataset  -c [-b -d -s -t -w -y] [path]

flags:
  -b, --branches <int>              - number of subdirectories per directory
  -c, --count <int>                 - total number of files to create
  -d, --depth <int>                 - number of directory levels
  -s, --size <float [K..T][i]B>     - file size
  -t, --threads <int>               - number of parallel file creation threads
  -w, --max-wait <float (seconds)>  - max random wait between file creation
  -y, --yes                         - don't prompt before creating files
```
#### Example
Generate 10 1GiB files in a single subdirectory named 'subdir':
```sh
gen-dataset -c 10 -s 1GiB subdir
```
Generate 10,000 1M files in 3905 directories:
```sh
gen-dataset -d 5 -b 5 -c 10000 -s 1MiB
```
Simulate real usage by randomly waiting up to 2.5 seconds between file creations:
```sh
gen-dataset -d 4 -b 6 -c 1000 -s 1MiB -w 2.5
```
Generate 1,000,000 empty files in 55986 directories with 16 threads writing the files:
```sh
gen-dataset -d 6 -b 6 -c 1000000 -t 16
```

