use uo;
use os;

const DUMMY_DPS_WINDOW_SECONDS := 20;

const DUMMY_PROP_TOTAL := "#totalDamage";
const DUMMY_PROP_FIRST := "#firstHitClock";
const DUMMY_PROP_LAST  := "#lastHitClock";

function DummyIsIdle(dummy)
	var lastHit := CInt(GetObjProperty(dummy, DUMMY_PROP_LAST));
	var total := CInt(GetObjProperty(dummy, DUMMY_PROP_TOTAL));
	if (total <= 0)
		return 0;
	endif
	return (ReadGameClock() - lastHit) > DUMMY_DPS_WINDOW_SECONDS;
endfunction

function DummyResetWindow(dummy)
	EraseObjProperty(dummy, DUMMY_PROP_TOTAL);
	EraseObjProperty(dummy, DUMMY_PROP_FIRST);
	EraseObjProperty(dummy, DUMMY_PROP_LAST);
	return 1;
endfunction

function DummyRefreshOverhead(dummy)
	var total := CInt(GetObjProperty(dummy, DUMMY_PROP_TOTAL));
	if (total <= 0)
		return 0;
	endif

	var firstHit := CInt(GetObjProperty(dummy, DUMMY_PROP_FIRST));
	var lastHit := CInt(GetObjProperty(dummy, DUMMY_PROP_LAST));
	var windowSec := lastHit - firstHit;
	if (windowSec < 1)
		windowSec := 1;
	endif

	var dps := CInt(total / windowSec);

	PrintTextAbove(dummy, $"DPS: {dps} — {total} em {windowSec}s");
	return 1;
endfunction

function DummyRegisterHit(dummy, amount)
	if (amount <= 0)
		amount := 0;
	endif

	var now := ReadGameClock();
	var lastHit := CInt(GetObjProperty(dummy, DUMMY_PROP_LAST));
	var total := CInt(GetObjProperty(dummy, DUMMY_PROP_TOTAL));

	if (total <= 0 || (now - lastHit) > DUMMY_DPS_WINDOW_SECONDS)
		SetObjProperty(dummy, DUMMY_PROP_FIRST, now);
		SetObjProperty(dummy, DUMMY_PROP_TOTAL, amount);
	else
		SetObjProperty(dummy, DUMMY_PROP_TOTAL, total + amount);
	endif

	SetObjProperty(dummy, DUMMY_PROP_LAST, now);

	DummyRefreshOverhead(dummy);
	return 1;
endfunction
