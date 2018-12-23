## Dependencies

- editline
  - https://github.com/troglobit/editline
  - `sudo apt-get install libedit-dev` (linux)
  - `brew install libedit` (mac OSX)
- gtest
  - https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/
  - `sudo apt-get install libgtest-dev` (linux)
    - This only gets the source code. Will still need to make and copy into
      /usr/lib
  - Mac OSX (https://stackoverflow.com/a/46611467)
    ```
    $ git clone https://github.com/google/googletest
    $ cd googletest
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ make install
    ```
- valgrind
- clang
- clang-format

## Building

```
$ build.sh
```

### Tests

```
$ run_tests.sh
```

## Formatting

```
$ format.sh
```
