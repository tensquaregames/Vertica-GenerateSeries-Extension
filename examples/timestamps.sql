SELECT generate_series('2016-01-25'::timestamp, '2016-02-04'::timestamp) OVER ();
SELECT generate_series('2016-01-25'::timestamp, '2016-02-04'::timestamp, '2 days'::interval) OVER ();
SELECT generate_series('2016-01-03 06:30:40'::timestamp, '2016-01-05 12:30:40', '7 hours 5 minutes') OVER ();

SELECT generate_series('2016-01-04 GMT+2'::timestamp with timezone, '2016-01-06 GMT+2') OVER ();
SELECT generate_series('2016-01-05 GMT+2'::timestamp with timezone, '2016-01-06 GMT+8', '3 hours') OVER ();

SELECT generate_series('2016-01-01'::timestamp, NULL) OVER ();
SELECT generate_series('2016-01-01 GMT+1'::timestamp with timezone, '2016-02-04 GMT+1', NULL) OVER ();
