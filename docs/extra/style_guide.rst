Style Guide
===========


Line Length
-----------

By convention the maximum length of a line of code is 80 characters long
in C, with that being historically grounded in standard-sized monitors
on older computer terminals, which could display 24 lines vertically and
80 characters horizontally. The 80 character limit should be considered a
"soft stop", while 100 characters is a "hard stop".

.. code:: c

    // These next lines of code first prompt the user to give two integer values and then multiplies those two integer values together so they can be used later in the program
    int first_collected_integer_value_from_user = get_int("Integer please: ");
    int second_collected_integer_value_from_user = get_int("Another integer please: ");
    int product_of_the_two_integer_values_from_user = first_collected_integer_value_from_user * second_collected_integer_value_from_user;

In other languages, particularly JavaScript, it is significantly more
difficult to constrain lines to a maximum length; there, your goal
should instead be to break up lines (as via ``\n``) in locations that
maximize readability and clarity.

Comments
--------

Within functions, use "inline comments".
Place the comment above the line(s) to which it applies. No need to
write in full sentences, but do capitalize the comment's first word
(unless it's the name of a function, variable, or the like), and do
leave one space between the ``//`` and your comment's first character,
as in:

.. code:: c

    // Convert Fahrenheit to Celsius
    float c = 5.0 / 9.0 * (f - 32.0);

In other words, don't do this:

.. code:: c

    //Convert Fahrenheit to Celsius
    float c = 5.0 / 9.0 * (f - 32.0);

Or this:

.. code:: c

    // convert Fahrenheit to Celsius
    float c = 5.0 / 9.0 * (f - 32.0);

Or this:

.. code:: c

    float c = 5.0 / 9.0 * (f - 32.0); // Convert Fahrenheit to Celsius

For documentation, use `Doxygen <https://www.doxygen.nl>`__'s format.

Atop your .c and .h files should be a comment that summarize what your
program (or that particular file) does, as in:

.. code:: c

    /**
      * @file hitchkiker.c
      * @author Douglas Adams
      * @brief Finds the answer to everything. 
      * @version 0.1
      * @date 2021-02-11
      * 
      * @copyright Copyright (c) 2021
      * 
      */

Atop each of your functions (except, perhaps, ``main``), meanwhile,
should be a comment that summarizes what your function is doing, as in:

.. code:: c

    /**
      * @brief Computes the square of n.
      * 
      * @param n Number to be squared.
      * @return n squared.
      */
    int square(int n) {
        return n * n;
    }

A couple notes:

- All descriptions begin with an uppercase letter (except if the first word
  is a function name or other terms with specific casing);
- Function descriptions use the third person: use "Computes"
  instead of "Compute";
- There is no article at the beginning of parameter descriptions: use "Number to be squared"
  instead of "The number to be squared"
- Acronyms are all caps: use "PCB" instead of "pcb"

Conditions
----------

Conditions should be styled as follows:

.. code:: c

    if (x > 0) {
        printf("x is positive\n");
    }
    else if (x < 0) {
        printf("x is negative\n");
    }
    else {
        printf("x is zero\n");
    }

Don't do this:

.. code:: c

    if (x < 0)
    {
        printf("x is negative\n");
    }
    else if (x < 0)
    {
        printf("x is negative\n");
    }

And definitely don't do this:

.. code:: c

    if (x < 0)
        {
        printf("x is negative\n");
        }
    else
        {
        printf("x is negative\n");
        }

Switches
--------

Declare a ``switch`` as follows:

.. code:: c

    switch (n) {
        case -1:
            printf("n is -1\n");
            break;

        case 1:
            printf("n is 1\n");
            break;

        default:
            printf("n is neither -1 nor 1\n");
            break;
    }

Functions
---------

In accordance with `C99 <http://en.wikipedia.org/wiki/C99>`__, be sure
to declare ``main`` with:

.. code:: c

    int main(void) {

    }

or, if using the CS50 Library, with:

.. code:: c

    #include <cs50.h>

    int main(int argc, string argv[]) {

    }

or with:

.. code:: c

    int main(int argc, char *argv[]) {

    }

or even with:

.. code:: c

    int main(int argc, char **argv) {

    }

Do not declare ``main`` with:

.. code:: c

    int main() {

    }

or with:

.. code:: c

    void main() {

    }

or with:

.. code:: c

    main() {

    }

As for your own functions, be sure to define them similiarly, with each
curly brace on its own line and with the return type on the same line as
the function's name, just as we've done with ``main``.

Indentation
-----------

Indent your code four spaces at a time to make clear which blocks of
code are inside of others. If you use your keyboard's Tab key to do so,
be sure that your text editor's configured to convert tabs (``\t``) to
four spaces, else your code may not print or display properly on someone
else's computer, since ``\t`` renders differently in different editors.

Here's some nicely indented code:

.. code:: c

    // Print command-line arguments one per line
    printf("\n");
    for (int i = 0; i < argc; i++) {
        for (int j = 0, n = strlen(argv[i]); j < n; j++) {
            printf("%c\n", argv[i][j]);
        }
        printf("\n");
    }

Loops
-----

for
~~~

Whenever you need temporary variables for iteration, use ``i``, then
``j``, then ``k``, unless more specific names would make your code more
readable:

.. code:: c

    for (int i = 0; i < LIMIT; i++) {
        for (int j = 0; j < LIMIT; j++) {
            for (int k = 0; k < LIMIT; k++) {
                // Do something
            }
        }
    }

If you need more than three variables for iteration, it might be time to
rethink your design!

while
~~~~~

Declare ``while`` loops as follows:

.. code:: c

    while (condition) {
        // Do something
    }

Notice how:

-  each curly brace is on its own line;
-  there's a single space after ``while``;
-  there isn't any space immediately after the ``(`` or immediately
   before the ``)``; and
-  the loop's body (a comment in this case) is indented with 4 spaces.

do ... while
~~~~~~~~~~~~

Declare ``do ... while`` loops as follows:

.. code:: c

    do {
        // Do something
    }
    while (condition);

Notice how:

-  each curly brace is on its own line;
-  there's a single space after ``while``;
-  there isn't any space immediately after the ``(`` or immediately
   before the ``)``; and
-  the loop's body (a comment in this case) is indented with 4 spaces.

Pointers
--------

When declaring a pointer, write the ``*`` next to the variable, as in:

.. code:: c

    int *p;

Don't write it next to the type, as in:

.. code:: c

    int* p;

Variables
---------

Because CS50 uses `C99 <http://en.wikipedia.org/wiki/C99>`__, do not
define all of your variables at the very top of your functions but,
rather, when and where you actually need them. Moreover, scope your
variables as tightly as possible. For instance, if ``i`` is only needed
for the sake of a loop, declare ``i`` within the loop itself:

.. code:: c

    for (int i = 0; i < LIMIT; i++) {
        printf("%i\n", i);
    }

Though it's fine to use variables like ``i``, ``j``, and ``k`` for
iteration, most of your variables should be more specifically named. If
you're summing some values, for instance, call your variable ``sum``. If
your variable's name warrants two words (e.g., ``is_ready``), put an
underscore between them, a convention popular in C though less so in
other languages.

If declaring multiple variables of the same type at once, it's fine to
declare them together, as in:

.. code:: c

    int quarters, dimes, nickels, pennies;

Just don't initialize some but not others, as in:

.. code:: c

    int quarters, dimes = 0, nickels = 0 , pennies;

Also take care to declare pointers separately from non-pointers, as in:

.. code:: c

    int *p;
    int n;

Don't declare pointers on the same line as non-pointers, as in:

.. code:: c

    int *p, n;

Structures
----------

Declare a ``struct`` as a type as follows:

.. code:: c

    typedef struct {
        string name;
        string dorm;
    } student;

If the ``struct`` contains as a member a pointer to another such
``struct``, declare the ``struct`` as having a name identical to the
type, without using underscores:

.. code:: c

    typedef struct node {
        int n;
        struct node *next;
    } node;
