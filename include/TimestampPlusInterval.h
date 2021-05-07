#include <Vertica.h>

#ifndef TIMESTAMPPLUSINTERVAL_H
#define TIMESTAMPPLUSINTERVAL_H

namespace Vertica
{


typedef int64 pg_time_t;

struct pg_tm
{
    int64       tm_sec;
    int64       tm_min;
    int64       tm_hour;
    int64       tm_mday;
    int64       tm_mon;         /* origin 0, not 1 */
    int64       tm_year;        /* relative to 1900 */
    int         tm_wday;
    int         tm_yday;
    int         tm_isdst;       /* -1=>no tz, 0=>not DST, 1=>DST */
    long int    tm_gmtoff;
    const char *tm_zone;
};

typedef int32 fsec_t;

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

const int   day_tab[2][13] =
{
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0}
};

void
dt2time(Timestamp jd, int64 *hour, int64 *min, int64 *sec, fsec_t *fsec)
{
    int64       time;

    time = jd;

    *hour = (time / usPerHour);
    time -= ((*hour) * usPerHour);
    *min = (time / usPerMinute);
    time -= ((*min) * usPerMinute);
    *sec = (time / usPerSecond);
    *fsec = (time - (*sec * usPerSecond));

    return;
}   /* dt2time() */


 // julian epoch to previous gregorian 400-year cycle
const int32 je2gc = 32044;              // 1/1/4713 BC - 3/1/4801 BC
const int32 days4yrs = 365*4 + 1;       // 1461 (if there is a leap day)
const int32 days100yrs = days4yrs*25 - 1; // 36524 (for the first 3)
const int32 daysgc = days100yrs*4 + 1;  // 146097 (days in gregorian cycle)

void
j2date(int64 jdn, int64 *year, int64 *month, int64 *day)
{
    int64 g;                            // Gregorian quadirecentennial cycles
    int64 j = jdn + je2gc;              // shift epoch to 3/1/4801 BC
    if (j < 0)
        g = (j + 1) / daysgc - 1;       // make divide work right
    else
        g = j / daysgc;                 // days per 400-year cycle
    int32 dg = j - g * daysgc;          // days within the cycle (positive)
    int32 c = dg / days100yrs;          // century within cycle
    if (c == 4) c = 3;                  // last century can be a day longer
    int32 dc = dg - c * days100yrs;     // days within century
    int32 b = dc / days4yrs;            // quadrennial cycles within century
    int32 db = dc - b * days4yrs;       // days within quadrennial cycle
    int32 a = db / 365;                 // Roman annual cycles (years)
    if (a == 4) a = 3;                  // if the last year was a day longer
    int32 da = db - a * 365;            // days in the last year (from 3/1)
    int64 y = ((g*4 + c)*25 + b)*4 + a; // years since epoch
    int32 m = (da*5 + 308)/153 - 2 + 2; // month from January by interpolation
    int32 d = da - (m + 2)*153/5 + 122; // day of month by interpolation

    *year = y - 4800 + m/12;            // adjust for epoch and 1-based m,d
    *month = m % 12 + 1;
    *day = d + 1;

    return;
}   /* j2date() */

int64
date2j(int64 y, int m, int d)
{
    int64       julian;

    if (++m < 4)
        --y, m += 12;

    julian = y * 365 + 1720997;         // - 32167;
    if (y < 0)
        julian += ++y / 4 - 1;          // make divide work right
    else
        julian += y / 4;                // leap year every four years,
    julian -= y / 100;                  // except centuries,
    julian += y / 400;                  // except quad-centuries
    julian += 7834LL * m / 256;         // march thru feb interpolation ~30.6
    julian += d;

    return julian;
}   /* date2j() */


static int64
time2t(const int hour, const int min, const int sec, const fsec_t fsec)
{
    return hour * usPerHour + min * usPerMinute + sec * usPerSecond + fsec;
}   /* time2t() */


Timestamp
dt2local(Timestamp dt, int tz)
{
    dt -= tz * usPerSecond;
    return dt;
}   /* dt2local() */

/*
 * timestamp2tm() - Convert timestamp/tz to a Postgres time structure.
 * 
 * If attimezone is NULL, the current session timezone will be used.
 */
void
timestamp2tm(Timestamp dt, int *tzp, struct pg_tm *tm, fsec_t *fsec,
             const struct pg_tz *attimezone)
{
    Timestamp   date;
    Timestamp   time;
    pg_time_t   utime;

    memset(tm, 0, sizeof *tm);          // all paths set tm_mon non-zero

    /* If tzp is null, no TZ conversion is wanted (TStz will use GMT) */
    if (tzp == NULL)
    {
        tm->tm_isdst = -1;              // no timezone flag

        time = kMod(dt, usPerDay);
        dt2time(time, &tm->tm_hour, &tm->tm_min, &tm->tm_sec, fsec);
        date = kDiv(dt, usPerDay) + POSTGRES_EPOCH_JDATE;
        j2date(date, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
        return;
    }

    /*
     * If the time falls within the range of pg_time_t, use pg_localtime_r() to
     * rotate to the local timezone.
     *
     * First, convert to an integral timestamp, avoiding possibly
     * platform-specific roundoff-in-wrong-direction errors, and adjust to
     * Unix epoch.
     */
    *fsec = kMod(dt, usPerSecond);
    utime = kDiv(dt, usPerSecond) +
        (POSTGRES_EPOCH_JDATE - UNIX_EPOCH_JDATE) * secPerDay;

    tm->tm_year += 1900;
    tm->tm_mon += 1;
    *tzp = tm->tm_gmtoff = -tm->tm_gmtoff;     // convert UNIX timezone to SQL
}


/* tm2timestamp()
 * Convert a tm structure to a timestamp data type.
 *
 * Returns -1 on failure (value out of range).
 */
int
tm2timestamp(struct pg_tm *tm, fsec_t fsec, int *tzp, Timestamp *result)
{
    int64       date;
    int64       time;
    int64       r;

    *result = 0;	  // avoid maybe-uninitialized compiler warnings
    date = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) - POSTGRES_EPOCH_JDATE;
    time = time2t(tm->tm_hour, tm->tm_min, tm->tm_sec, fsec);
    if (tzp != NULL)
        time += *tzp * usPerSecond;     // can't overflow

    // be very careful about overflow!!
    uint64 udate = llabs(date) + 3;     // allow time to be +/-3 days
    if (udate > -1ull / usPerDay - 3)
        return -1;                      // multiply will overflow uint64
    udate *= usPerDay;
    udate += (date >= 0 ? time : -time); // will not overflow or go negative

    if (udate >= uint64(DT_NOEND) + 3 * usPerDay)
        return -1;                      // abs(result) >= infinity

    r = udate - 3 * usPerDay;
    *result = (date >= 0 ? r : -r);
    return 0;
}

/* timestamp_pl_intervalym()
 * Add an intervalYM to a timestamp data type.
 */
Timestamp timestamp_pl_intervalym(Timestamp ts, IntervalYM inv)
{
    Timestamp result;
    if (ts == DT_NULL || inv == DT_NULL)
        result = DT_NULL;
    else if (TIMESTAMP_NOT_FINITE(ts))
        result = ts;
    else {
        struct pg_tm tt;
        fsec_t fsec;

        timestamp2tm(ts, NULL, &tt, &fsec, NULL);
        tt.tm_mon += inv - 1;
        tt.tm_year += kDiv(tt.tm_mon, MONTHS_PER_YEAR);
        tt.tm_mon = kMod(tt.tm_mon, MONTHS_PER_YEAR) + 1;

        /* adjust for end of month boundary problems... */
        if (tt.tm_mday > day_tab[isleap(tt.tm_year)][tt.tm_mon - 1])
            tt.tm_mday = (day_tab[isleap(tt.tm_year)][tt.tm_mon - 1]);

        if (tm2timestamp(&tt, fsec, NULL, &result) != 0)
            throw std::invalid_argument("timestamp out of range");
    }
    
	return result;
}

}

#endif
