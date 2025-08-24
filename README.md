Simple reverse Polish notation calculator written in C.

## Usage

```bash
$ ccalc [FILE]...
```

Reads instructions from `FILE`(s).
If no `FILE` specified or is `-`, reads standard input.

Supported instructions are:
- `number` - push a number to stack
- `addition` - pop two numbers from the stack and push their sum to it
- `subtraction` - pop two numbers from the stack and push their diff to it (left - right)
- `multiplication` - pop two numbers from the stack and push their prod to it
- `division` - pop two numbers from the stack and push their quotient to it (left / right)
- `print` - pop a number and print it to stdout

Here's the instruction set syntax definition in Wirth notation:
```wsn
digit = "0" | "1" | ... | "9" .
fraction = "." digit { digit } .
e = "e" | "E" .
exponent = e [ "-" ] digit { digit } .
number = [ "-" ] ( ( digit { digit } [ fraction ] ) | ( { digit } fraction ) ) [ exponent ] .
addition = "+" .
subtraction = "-" .
multiplication = "*" .
division = "/" .
print = "=" .
instruction
    = number
    | addition
    | subtraction
    | multiplication
    | division
    | print
    .
```

Here are some examples:
```bash
$ ccalc
> 1 2 + =
3
$ echo 123 45 = | ccalc
45
Unused value on stack: 123
$ ccalc
> -.2394872498326423984732987423 =
-0.239487
$ ccalc
> 12 0 /
Attempt to divide by zero
$ echo 12 12 \* 24 24 \* - = | ccalc # \* instead of * to avoid mixing up with some regex
-432
```

## Building

The project is simple enough to be compiled in a single command, e.g.:
```bash
$ clang -o ccalc main.c lexer.c stack.c interpreter.c
```

However, `CMakeLists.txt` is provided, so you can use CMake. Here's a
copy-paste friendly command list to build the app:
```bash
mkdir build && cd build
cmake ..
cmake --build .
```
It will produce `./ccalc` binary executable.

## Credits

Made by **nz** aka **nunzayin** aka **Nick Zaber**
