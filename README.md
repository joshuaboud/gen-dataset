# gen-dataset
A command line tool to quickly generate a lot of files in a lot of directories.

## Installation
### Precompiled Static Binary
```sh
sudo curl https://github.com/joshuaboud/gen-dataset/releases/download/v1.0/gen-dataset -o /usr/local/bin/gen-dataset
sudo chmod +x /usr/local/bin/gen-dataset
```
### From Source
Install boost development libraries then
```sh
git clone https://github.com/joshuaboud/gen-dataset.git
cd gen-dataset
make -j8
sudo make install
```

### Usage
```
usage:
  gen-dataset -d -b -c [-s -w -t] [path]

flags:
  -d, --depth <int>                 - number of directory levels
  -b, --branches <int>              - number of subdirectories per directory
  -c, --count <int>                 - total number of files to create
  -s, --size <float [K..T][i]B>     - file size
  -w, --max-wait <float (seconds)>  - max random wait between file creation
  -t, --threads <int>               - number of parallel file creation threads
  -y, --yes                         - don't prompt before creating files
```
#### Example
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

