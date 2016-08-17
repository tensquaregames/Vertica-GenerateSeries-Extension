Vertica `generate_series` extension
===================================


Overview
--------

This Vertica extension adds support for a PostgreSQL-like `generate_series`
function. It simply takes two bounds and returns set of values between these
bounds:

    $ SELECT generate_series(1, 5) OVER ();
    >  generate_series
    > -----------------
    >                1
    >                2
    >                3
    >                4
    >                5
    > (5 rows)

You can also specify a step value using a third optional argument:

    $ SELECT generate_series(1.0, 3.0, 0.5) OVER ();
    >  generate_series
    > -----------------
    >               1
    >             1.5
    >               2
    >             2.5
    >               3
    > (5 rows)

The function works with integers, floats and timestamps (with and without
time zones). Refer to the `examples` directory to see more usage samples.


Building
--------

To build the library, navigate to the folder where this README is located and
run the following command:

    make build


Installation
------------

Assuming that you have `vsql` command on your `PATH` you can use the `install`
task:

    make install

You can also specify custom `vsql` command used for installation using the
`VSQL` argument:

    make VSQL="vsql -U foo -h bar baz" install

Alternatively, you can load the install script located in `ddl/install.sql`
manually.


Testing
-------

To verify that everything works as expected, you can use execute sample
scripts:

    make examples
