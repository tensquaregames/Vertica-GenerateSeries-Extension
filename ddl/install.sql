SELECT version();

\set libfile '\''`pwd`'/lib/GenerateSeriesLib.so\'';
CREATE LIBRARY GenerateSeriesLib as :libfile;

CREATE TRANSFORM FUNCTION generate_series AS LANGUAGE 'C++' Name 'IntSeriesFactory' LIBRARY GenerateSeriesLib NOT FENCED;
CREATE TRANSFORM FUNCTION generate_series AS LANGUAGE 'C++' Name 'IntSeriesWithStepFactory' LIBRARY GenerateSeriesLib NOT FENCED;
CREATE TRANSFORM FUNCTION generate_series AS LANGUAGE 'C++' Name 'FloatSeriesFactory' LIBRARY GenerateSeriesLib NOT FENCED;
CREATE TRANSFORM FUNCTION generate_series AS LANGUAGE 'C++' Name 'FloatSeriesWithStepFactory' LIBRARY GenerateSeriesLib NOT FENCED;
CREATE TRANSFORM FUNCTION generate_series AS LANGUAGE 'C++' Name 'TimestampSeriesFactory' LIBRARY GenerateSeriesLib NOT FENCED;
CREATE TRANSFORM FUNCTION generate_series AS LANGUAGE 'C++' Name 'TimestampSeriesWithStepFactory' LIBRARY GenerateSeriesLib NOT FENCED;
CREATE TRANSFORM FUNCTION generate_series AS LANGUAGE 'C++' Name 'TimestampTzSeriesFactory' LIBRARY GenerateSeriesLib NOT FENCED;
CREATE TRANSFORM FUNCTION generate_series AS LANGUAGE 'C++' Name 'TimestampTzSeriesWithStepFactory' LIBRARY GenerateSeriesLib NOT FENCED;
