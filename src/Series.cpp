#include <Vertica.h>
using Vertica::TransformFunction;
using Vertica::TransformFunctionFactory;


// This should match the function name specified in `ddl/install.sql`.
#define RESULT_COLUMN_NAME "generate_series"


template<class D, typename T, typename S>
class AbstractSeries : public TransformFunction {
public:

	virtual void processPartition(Vertica::ServerInterface &,
	                              Vertica::PartitionReader &argReader,
	                              Vertica::PartitionWriter &resWriter)
	{
		try {
			const size_t numCols = argReader.getNumCols();
			switch (numCols) {
			case 2:
			case 3:
				break;
			default:
				vt_report_error(0, "Function received %zu arguments, but accepts only two or three.",
				                numCols);
				return;
			}

			do {
				T start;
				if (argReader.isNull(0)) {
					continue;
				} else {
					start = D::getBound(argReader, 0);
				}

				T end;
				if (argReader.isNull(1)) {
					continue;
				} else {
					end = D::getBound(argReader, 1);
				}

				T step;
				if (numCols == 3) {
					if (argReader.isNull(2)) {
						continue;
					} else {
						step = D::getStep(argReader, 2);
					}
				} else {
					step = D::defaultStep();
				}

				for (T i = start; i <= end; i += step) {
					D::setRes(resWriter, i);
					resWriter.next();
				}
			} while (argReader.next());
		} catch (std::exception &exception) {
			vt_report_error(0, "Exception while processing partition: [%s]",
			                exception.what());
		}
	}
};

class IntSeries : public AbstractSeries<IntSeries, Vertica::vint, Vertica::vint> {
public:

	static Vertica::vint getBound(const Vertica::PartitionReader &argReader, size_t col)
	{
		return argReader.getIntRef(col);
	}

	static Vertica::vint getStep(const Vertica::PartitionReader &argReader, size_t col)
	{
		return argReader.getIntRef(col);
	}

	static Vertica::vint defaultStep()
	{
		return 1;
	}

	static void setRes(Vertica::PartitionWriter &resWriter,
	                   const Vertica::vint &value)
	{
		return resWriter.setInt(0, value);
	}
};

class FloatSeries: public AbstractSeries<FloatSeries, Vertica::vfloat, Vertica::vfloat> {
public:

	static Vertica::vfloat getBound(const Vertica::PartitionReader &argReader, size_t col)
	{
		return argReader.getFloatRef(col);
	}

	static Vertica::vfloat getStep(const Vertica::PartitionReader &argReader, size_t col)
	{
		return argReader.getFloatRef(col);
	}

	static Vertica::vfloat defaultStep()
	{
		return 1.0;
	}

	static void setRes(Vertica::PartitionWriter &resWriter,
	                   const Vertica::vfloat &value)
	{
		return resWriter.setFloat(0, value);
	}
};

class TimestampSeries : public AbstractSeries<TimestampSeries, Vertica::Timestamp, Vertica::Interval> {
public:

	static Vertica::Timestamp getBound(const Vertica::PartitionReader &argReader, size_t col)
	{
		return argReader.getTimestampRef(col);
	}

	static Vertica::Interval getStep(const Vertica::PartitionReader &argReader, size_t col)
	{
		return argReader.getIntervalRef(col);
	}

	static Vertica::Interval defaultStep()
	{
		// Days in microseconds.
		return 24LL * 60LL * 60LL * 1000LL * 1000LL;
	}

	static void setRes(Vertica::PartitionWriter &resWriter,
	                   const Vertica::Timestamp &value)
	{
		return resWriter.setTimestamp(0, value);
	}
};

class TimestampTzSeries : public AbstractSeries<TimestampTzSeries, Vertica::TimestampTz, Vertica::Interval> {
public:

	static Vertica::TimestampTz getBound(const Vertica::PartitionReader &argReader, size_t col)
	{
		return argReader.getTimestampRef(col);
	}

	static Vertica::Interval getStep(const Vertica::PartitionReader &argReader, size_t col)
	{
		return argReader.getIntervalRef(col);
	}

	static Vertica::Interval defaultStep()
	{
		// Days in microseconds.
		return 24LL * 60LL * 60LL * 1000LL * 1000LL;
	}

	static void setRes(Vertica::PartitionWriter &resWriter,
	                   const Vertica::TimestampTz &value)
	{
		return resWriter.setTimestampTz(0, value);
	}
};


template<class D>
class AbstractSeriesFactory : public TransformFunctionFactory {
public:

	virtual TransformFunction *createTransformFunction(Vertica::ServerInterface &iface)
	{
		return vt_createFuncObj(iface.allocator, D);
	}
};

class IntSeriesFactory : public AbstractSeriesFactory<IntSeries> {
public:

	virtual void getPrototype(Vertica::ServerInterface &,
	                          Vertica::ColumnTypes &argTypes,
	                          Vertica::ColumnTypes &resTypes)
	{
		argTypes.addInt();
		argTypes.addInt();
		resTypes.addInt();
	}

	virtual void getReturnType(Vertica::ServerInterface &,
	                           const Vertica::SizedColumnTypes &,
	                           Vertica::SizedColumnTypes &resTypes)
	{
		resTypes.addInt(RESULT_COLUMN_NAME);
	}
};

class IntSeriesWithStepFactory : public IntSeriesFactory {
public:

	virtual void getPrototype(Vertica::ServerInterface &iface,
	                          Vertica::ColumnTypes &argTypes,
	                          Vertica::ColumnTypes &resTypes)
	{
		IntSeriesFactory::getPrototype(iface, argTypes, resTypes);
		argTypes.addInt();
	}
};

class FloatSeriesFactory : public AbstractSeriesFactory<FloatSeries> {
public:

	virtual void getPrototype(Vertica::ServerInterface &,
	                          Vertica::ColumnTypes &argTypes,
	                          Vertica::ColumnTypes &resTypes)
	{
		argTypes.addFloat();
		argTypes.addFloat();
		resTypes.addFloat();
	}

	virtual void getReturnType(Vertica::ServerInterface &,
	                           const Vertica::SizedColumnTypes &,
	                           Vertica::SizedColumnTypes &resTypes)
	{
		resTypes.addFloat(RESULT_COLUMN_NAME);
	}
};

class FloatSeriesWithStepFactory : public FloatSeriesFactory {
public:

	virtual void getPrototype(Vertica::ServerInterface &iface,
	                          Vertica::ColumnTypes &argTypes,
	                          Vertica::ColumnTypes &resTypes)
	{
		FloatSeriesFactory::getPrototype(iface, argTypes, resTypes);
		argTypes.addFloat();
	}
};

class TimestampSeriesFactory : public AbstractSeriesFactory<TimestampSeries> {
public:

	virtual void getPrototype(Vertica::ServerInterface &,
	                          Vertica::ColumnTypes &argTypes,
	                          Vertica::ColumnTypes &resTypes)
	{
		argTypes.addTimestamp();
		argTypes.addTimestamp();
		resTypes.addTimestamp();
	}

	virtual void getReturnType(Vertica::ServerInterface &,
	                           const Vertica::SizedColumnTypes &argTypes,
	                           Vertica::SizedColumnTypes &resTypes)
	{
		const Vertica::VerticaType &opType = argTypes.getColumnType(0);
		resTypes.addTimestamp(opType.getTimestampPrecision(), RESULT_COLUMN_NAME);
	}
};

class TimestampSeriesWithStepFactory : public TimestampSeriesFactory {
public:

	virtual void getPrototype(Vertica::ServerInterface &iface,
	                          Vertica::ColumnTypes &argTypes,
	                          Vertica::ColumnTypes &resTypes)
	{
		TimestampSeriesFactory::getPrototype(iface, argTypes, resTypes);
		argTypes.addInterval();
	}
};

class TimestampTzSeriesFactory : public AbstractSeriesFactory<TimestampTzSeries> {
public:

	virtual void getPrototype(Vertica::ServerInterface &,
	                          Vertica::ColumnTypes &argTypes,
	                          Vertica::ColumnTypes &resTypes)
	{
		argTypes.addTimestampTz();
		argTypes.addTimestampTz();
		resTypes.addTimestampTz();
	}

	virtual void getReturnType(Vertica::ServerInterface &,
	                           const Vertica::SizedColumnTypes &argTypes,
	                           Vertica::SizedColumnTypes &resTypes)
	{
		const Vertica::VerticaType &opType = argTypes.getColumnType(0);
		resTypes.addTimestampTz(opType.getTimestampPrecision(), RESULT_COLUMN_NAME);
	}
};

class TimestampTzSeriesWithStepFactory : public TimestampTzSeriesFactory {

	virtual void getPrototype(Vertica::ServerInterface &iface,
	                          Vertica::ColumnTypes &argTypes,
	                          Vertica::ColumnTypes &resTypes)
	{
		TimestampTzSeriesFactory::getPrototype(iface, argTypes, resTypes);
		argTypes.addInterval();
	}
};


RegisterFactory(IntSeriesFactory);
RegisterFactory(IntSeriesWithStepFactory);
RegisterFactory(FloatSeriesFactory);
RegisterFactory(FloatSeriesWithStepFactory);
RegisterFactory(TimestampSeriesFactory);
RegisterFactory(TimestampSeriesWithStepFactory);
RegisterFactory(TimestampTzSeriesFactory);
RegisterFactory(TimestampTzSeriesWithStepFactory);
