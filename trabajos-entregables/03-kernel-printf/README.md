# Writing a small kernel library

In this step we include a small set of useful functions for memory and string
operations and `printf(fmr, ...)` in `klib.c` module.

We modify source code structure for better modularity moving UART input/output
to `console.c`.

Also, we define a `panic` and `stop` macros in `klib.h`.

# The printf function

The C programming language enable us to define *variadic functions* (functions
with a variable number of arguments).

Some platforms pass arguments on stack, others in registers or both, depending
on number of arguments. Types and macros defined in `stdarg.h` hide (abstract)
the platform details to write portable code.

The data type `va_list` represents the arguments list passed to function. The
main macros are:

- `va_start(vargs, fmt)`: Enable the use of `vargs` arguments list.
- `va_arg(vargs, <type>)` Return the *current* argument value and advance the
  vargs internal pointer to next argument value.
- `va_end(vargs)`: Reset the `vargs` list.

For mor information about `va_list` macros, do `man stdarg`.

The `printf(fmt, ...)` function parse the `fmt` string to get types and values
needed for `va_arg()` macro.

## Testing printf function

In function `kernel_main()` calls the `printf` function with some arguments
of different types.