SELECT * FROM (SELECT generate_series(1, 100) OVER () AS n) AS nats
WHERE NOT EXISTS
    (SELECT * FROM (SELECT generate_series(1, 100) OVER () AS n) AS divs
     WHERE divs.n > 1 AND divs.n < nats.n AND nats.n % divs.n = 0);
