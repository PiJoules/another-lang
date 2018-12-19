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
$ clang++ -std=c++14 -g -Werror -Wall -fno-rtti -ledit Prompt.cpp Lexer.cpp Parser.cpp Nodes.cpp
```

or

```
$ build.sh
```

### Tests

```
$ clang++ -std=c++14 -Werror -Wall -fno-rtti TestLexer.cpp Lexer.cpp -pthread -lgtest
$ clang++ -std=c++14 -Werror -Wall -fno-rtti TestParser.cpp Parser.cpp Lexer.cpp -pthread -lgtest
$ clang++ -std=c++14 -Werror -Wall -fno-rtti TestEval.cpp Parser.cpp Lexer.cpp Nodes.cpp -pthread -lgtest
```

or

```
$ run_tests.sh
```

## Formatting

```
$ clang-format -style=Google -i *.cpp *.h
```

or

```
$ format.sh
```
