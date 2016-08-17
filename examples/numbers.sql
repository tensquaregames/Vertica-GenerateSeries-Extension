SELECT generate_series(1, 10) OVER ();
SELECT generate_series(1, 100, 10) OVER ();

SELECT generate_series(-2.5, 2.5) OVER ();
SELECT generate_series(-2.5, 2.5, 0.125) OVER ();

SELECT generate_series(NULL, 10) OVER ();
SELECT generate_series(1, NULL) OVER ();
SELECT generate_series(1, 10, NULL) OVER ();
