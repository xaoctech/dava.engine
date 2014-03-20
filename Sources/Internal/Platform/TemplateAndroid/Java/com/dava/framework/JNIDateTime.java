package com.dava.framework;

import java.util.Iterator;
import java.util.TimeZone;
import java.util.Date;
import android.text.format.Time;

public class JNIDateTime {
	final static String TAG = "JNIDateTime";

	public static String GetTimeAsString( final String format, long timeStamp, int timeZoneOffset)
	{
        TimeZone tz = TimeZone.getTimeZone("");
        int offset = timeZoneOffset*1000;
        String tzArr[] = TimeZone.getAvailableIDs(offset);
        for(int i = 0; i < tzArr.length; ++i)
		{
			TimeZone innerTz =  TimeZone.getTimeZone(tzArr[i]); 
			if(innerTz.getRawOffset() == offset)
			{
				tz = innerTz;
				break;
			}
		}
        
        Time time = new Time();
        time.set(timeStamp * 1000);
        time.switchTimezone(tz.getID());

        return time.format(format);
	}
	
	public static int GetLocalTimeZoneOffset()
	{
		TimeZone tz = TimeZone.getDefault();
		Date now = new Date();
		int offsetFromUtc = tz.getOffset(now.getTime()) / 1000;
		return offsetFromUtc;
	}
}
