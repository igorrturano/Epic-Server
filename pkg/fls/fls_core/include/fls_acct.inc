
use uo;

function jailAcc(byref acct, time)

	acct.setprop("releaseday",(ReadGameClock() + time));
	var characters := EnumerateOnlineCharacters();
	acct.disable();

	foreach char in characters
		if(char.acctname == acct.name)
			SendSysMessage(char, "Voce foi kickado.");
			DisconnectClient(char);
			break;
		endif
	endforeach

endfunction
