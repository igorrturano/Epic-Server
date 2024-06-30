// $Id$

use uo;

// function list
//
// GetDayOfMonth(year, dayofyear)
// GetFormattedSysClock(ticks:= -1)
// GetMonth(year, dayofyear)
// GetSysClock(ticks:= -1)
// GetSysClockStr(ticks:= -1)
// GetSysDate(ticks:= -1)
// GetSysTime(ticks:= -1)
// IsLeapyear(year)


function GetSysClockStr(ticks:= -1)

  return GetSysClock(ticks).toString;

endfunction



function GetSysDate(ticks:= -1)

  var allclock:= GetSysClock(ticks);
  var myclock:= struct;
  myclock.+year:= allclock.year;
  myclock.+month:= allclock.month;
  myclock.+day:= allclock.day;
  myclock.+toString:= CStr (myclock.year) + "-" + CStr (myclock.month) + "-" + CStr (myclock.day);
  return myclock;

endfunction



function GetSysTime(ticks:= -1)

  var allclock:= GetSysClock(ticks);
  var myclock:= struct;
  myclock.+h:= allclock.h;
  myclock.+m:= allclock.m;
  myclock.+s:= allclock.s;
  myclock.+toString:= CStr (myclock.h) + ":" + CStr (myclock.m) + ":" + CStr (myclock.s);
  return myclock;

endfunction



function GetSysClock(ticks:= -1)

  const TIMEZONE:= 2;

  if (ticks == -1)
    ticks:= PolCore ().systime;
  endif
  
  ticks:= ticks + TIMEZONE * 60 * 60;
  var year:= 1970;

  while (ticks >= (60 * 60 * 24 * (365 + isLeapyear(year))))
    year:= year + 1;
    ticks:= ticks - (60 * 60 * 24 * (365 + isLeapyear(year)));
  endwhile

  var dayofyear:= 1;

  while (ticks >= (60 * 60 * 24))
    dayofyear:= dayofyear + 1;
    ticks:= ticks - (60 * 60 * 24);
  endwhile

  var month:= GetMonth(year, dayofyear);
  var day:= GetDayOfMonth(year, dayofyear);
  var h:= ticks / (60 * 60);
  ticks:= ticks - (h * 60 * 60);
  var m:= ticks / 60;
  ticks:= ticks - (m * 60);
  var s:= ticks;

  var myclock:= struct;
  myclock.+year:= year;
  myclock.+month:= month;
  myclock.+day:= day;
  myclock.+h:= h;
  myclock.+m:= m;
  myclock.+s:= s;
  myclock.+toString:= CStr (year) + "-" + CStr (month) + "-" + CStr (day) + " @ " + CStr (h) + ":" + CStr (m) + ":" + CStr (s);
  return myclock;

endfunction



function GetFormattedSysClock (ticks:= -1)

  var day;
  var hour;
  var minute;
  var month;
  var now:= GetSysClock(ticks);
  var second;
  var text;
  var year:= now.year;

  month:= CStr (now.month);

  if (len (month) == 1)
    month:= "0" + month;
  endif

  day:= CStr (now.day);

  if (len (day) == 1)
     day:= "0" + day;
  endif

  hour:= CStr (now.h);

  if (len (hour) == 1)
    hour:= "0" + hour;
  endif

  minute:= CStr (now.m);

  if (len (minute) == 1)
    minute:= "0" + minute;
  endif

  second:= CStr (now.s);

  if (len (second) == 1)
    second := "0" + second;
  endif
  
  text:= year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;
  return text;

endfunction



function IsLeapyear(year)

  if ((year % 4 == 0) &&
      ((!((year % 100) == 0)) || ((year % 400) == 0)))
    return 1;
  else
    return 0;
  endif

endfunction



// Algorithm by R. A. Stone
function GetMonth(year, dayofyear)

  var a:= isLeapyear(year);
  var n:= dayofyear;

  if (n > (59 + a))
    n:= n + 2 - a;
  endif

  n:= n + 91;
  return (((20 * n) / 611) - 2);

endfunction



function GetDayOfMonth(year, dayofyear)

  var day:= dayofyear;
  var i:= 1;
  var monthlength:= array {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  monthlength[2]:= monthlength[2] + IsLeapyear(year);

  while (day > monthlength[i])
    day:= day - monthlength[i];
    i:= i + 1;
  endwhile

  return day;

endfunction
