GRANT EXECUTE ON TRANSFORM FUNCTION public.generate_series (int, int) TO public;
GRANT EXECUTE ON TRANSFORM FUNCTION public.generate_series (int, int, int) TO public;
GRANT EXECUTE ON TRANSFORM FUNCTION public.generate_series (float, float) TO public;
GRANT EXECUTE ON TRANSFORM FUNCTION public.generate_series (float, float, float) TO public;
GRANT EXECUTE ON TRANSFORM FUNCTION public.generate_series (timestamp, timestamp) TO public;
GRANT EXECUTE ON TRANSFORM FUNCTION public.generate_series (timestamp, timestamp, interval day to second) TO public;
GRANT EXECUTE ON TRANSFORM FUNCTION public.generate_series (timestamp with timezone, timestamp with timezone) TO public;
GRANT EXECUTE ON TRANSFORM FUNCTION public.generate_series (timestamp with timezone, timestamp with timezone, interval day to second) TO public;
