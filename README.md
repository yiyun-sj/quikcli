# QuikCli
QuikCli is an opinionated interactive command line interface builder

## Table of Contents

- [Use Case](#use-case)
- [Design](#design)
- [example](#example)

## Use Case
The interactivity of QuikCli was inspired by form-like cli programs where users are continuously prompted for a response. QuikCli offers a suite of prompt "components" that the entire cli application is then composed of. As such, QuikCli is geared towards more interactive cli programs (see [here](#example) for an example).

If you are looking for a more un-opinionated experience or more freedom in your cli applications, there are many other wonderful C++ libraries for cli parsing and building such as [CLI11](https://github.com/CLIUtils/CLI11) and [daniele77/cli](https://github.com/daniele77/cli).

## Design
The cli design of QuikCli was influenced by the [Command Line Interface Guide](https://clig.dev/).

Parts of the guide that QuikCli strives to do:
- Usable: the cli program should be design for human usage
- Consistent: existing patterns should be respected and followed
<!-- - Supportive: provide meaningful help, examples, and error text (i.e. autocomplete) -->
<!-- - Robust: tested for edge cases; gives assurance to users -->

Thus, certain decisions were made regarding the parsing, user interface, and interactivity provided by QuikCli as a effort to simplify and improve the usability of resulting applications.

## Example
An example cli program built with QuikCli in the form of a RPG character builder is provided that leverages most of the features QuikCli offers.

### Compilation

To compile the example program:

``` sh
mkdir build && cd build
cmake .. && make
```

To run the example:

``` sh
./example/example
```
