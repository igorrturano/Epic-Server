use basic;
use uo;
use util;
use os;
use cfgfile;

function GetSoundDataArray(who)
	case(who.graphic)
				//pantera
				214:
				  return array(

						"escolha sua açao para a forma de pantera:",
						"1 - rosnar",
						"2 - rugir",
						"3 - rosnar2",
						"4 - morder",
						"5 - chorar"
					);
				//serpente gigante
				21:
				  return array(

						"escolha sua açao para a forma de serpente:",
						"1 - sibilar",
						"2 - sibilar2",
						"3 - sibilar3",
						"4 - morder",
						"5 - chorar"
					);
				//urso negro
				211:
				  return array(

						"escolha sua açao para a forma de urso:",
						"1 - rosnar",
						"2 - bramir",
						"3 - rosnar2",
						"4 - morder",
						"5 - chorar"
					);
				//lobo
				225:
				  return array(

						"escolha sua açao para a forma de lobo:",
						"1 - rosnar",
						"2 - uivar",
						"3 - rosnar2",
						"4 - morder",
						"5 - chorar"
					);
				//qualquer outra coisa
				default:
					return array(

						"escolha sua açao:",
						"1 - ah",
						"2 - ahha",
						"3 - aplaudir",
						"4 - assoar",
						"5 - comemorar",
						"6 - limpargarganta",
						"7 - tossir",
						"8 - tossir2",
						"9 - chorar",
						"10 - arfar",
						"11 - risadinha",
						"12 - gemer",
						"13 - rosnar",
						"14 - hey",
						"15 - solucar",
						"16 - huh?",
						"17 - beijar",
						"18 - rir",
						"19 - no",
						"20 - oh",
						"21 - oooh",
						"22 - oops",
						"23 - gritar",
						"24 - shhh",
						"25 - suspirar",
						"26 - espirrar",
						"27 - sniff",
						"28 - roncar",
						"29 - cuspir",
						"30 - assobiar",
						"31 - bocejar",
						"32 - yeah",
						"33 - gritar2"
					);
			endcase
endfunction

function GetSoundEffect(who, input)
	var resultado := {};

	if ( (input[1]==1) )
	case(who.graphic)
	401:
	  resultado[1] := 0x30B;
	  resultado[2] := "*Ah*";

	400:
	  resultado[1] := 0x41A;
	  resultado[2] := "*Ah*";

	1828:
	  resultado[1] := 0x41A;
	  resultado[2] := "*Ah*";

	1829:
	  resultado[1] := 0x30B;
	  resultado[2] := "*Ah*";

	1830:
	  resultado[1] := 0x41A;
	  resultado[2] := "*Ah*";

	1831:
	  resultado[1] := 0x30B;
	  resultado[2] := "*Ah*";
	1832:
	  resultado[1] := 0x41A;
	  resultado[2] := "*Ah*";

	1833:
	  resultado[1] := 0x30B;
	  resultado[2] := "*Ah*";
	1834:
	  resultado[1] := 0x41A;
	  resultado[2] := "*Ah*";

	1835:
	  resultado[1] := 0x30B;
	  resultado[2] := "*Ah*";
	225:
	  resultado[1] := 0x0E6;

	211:
	  resultado[1] := 0x060;

	21:
	  resultado[1] := 0x0DC;

	214:
	  resultado[1] := 0x463;
	  endcase
	endif
	if ( (input[2]==1) )
	case(who.graphic)
	401:
	  resultado[1] := 0x30C ;
	  resultado[2] := "*Ahha*";

	400:
	  resultado[1] := 0x41B ;
	  resultado[2] := "*Ahha*";

	1828:
	  resultado[1] := 0x41B ;
	  resultado[2] := "*Ahha*";

	1829:
	  resultado[1] := 0x30C ;
	  resultado[2] := "*Ahha*";

	1830:
	  resultado[1] := 0x41B ;
	  resultado[2] := "*Ahha*";

	1831:
	  resultado[1] := 0x30C ;
	  resultado[2] := "*Ahha*";

	1832:
	  resultado[1] := 0x41B ;
	  resultado[2] := "*Ahha*";

	1833:
	  resultado[1] := 0x30C ;
	  resultado[2] := "*Ahha*";

	1834:
	  resultado[1] := 0x41B ;
	  resultado[2] := "*Ahha*";

	1835:
	  resultado[1] := 0x30C ;
	  resultado[2] := "*Ahha*";
	225:
	  resultado[1] := 0x0E7 ;

	211:
	  resultado[1] := 0x061 ;

	21:
	  resultado[1] := 0x0DD;

	214:
	  resultado[1] := 0x464;
	  endcase
	endif
	if ( (input[3]==1) )
	case(who.graphic)
	401:
	  resultado[1] := 0x30D ;
	  resultado[2] := "*Aplausos*";

	400:
	  resultado[1] := 0x41C ;
	  resultado[2] := "*Aplausos*";

	1828:
	  resultado[1] := 0x41C ;
	  resultado[2] := "*Aplausos*";

	1829:
	  resultado[1] := 0x30D ;
	  resultado[2] := "*Aplausos*";

	1830:
	  resultado[1] := 0x41C ;
	  resultado[2] := "*Aplausos*";

	1831:
	  resultado[1] := 0x30D ;
	  resultado[2] := "*Aplausos*";

	1832:
	  resultado[1] := 0x41C ;
	  resultado[2] := "*Aplausos*";

	1833:
	  resultado[1] := 0x30D ;
	  resultado[2] := "*Aplausos*";

	1834:
	  resultado[1] := 0x41C ;
	  resultado[2] := "*Aplausos*";

	1835:
	  resultado[1] := 0x30D ;
	  resultado[2] := "*Aplausos*";


	225:
	  resultado[1] := 0x0E8 ;

	211:
	  resultado[1] := 0x062 ;

	21:
	  resultado[1] := 0x0DE;

	214:
	  resultado[1] := 0x465;
	endcase
	endif
	if ( (input[4]==1) )
	case(who.graphic)
	401:
	  resultado[1] := 0x30E ;
	  resultado[2] := "*Assoando o nariz*";

	400:
	  resultado[1] := 0x41D ;
	  resultado[2] := "*Assoando o nariz*";

	1828:
	  resultado[1] := 0x41D ;
	  resultado[2] := "*Assoando o nariz*";

	1829:
	  resultado[1] := 0x30E ;
	  resultado[2] := "*Assoando o nariz*";

	1830:
	  resultado[1] := 0x41D ;
	  resultado[2] := "*Assoando o nariz*";

	1831:
	  resultado[1] := 0x30E ;
	  resultado[2] := "*Assoando o nariz*";

	1832:
	  resultado[1] := 0x41D ;
	  resultado[2] := "*Assoando o nariz*";

	1833:
	  resultado[1] := 0x30E ;
	  resultado[2] := "*Assoando o nariz*";

	1834:
	  resultado[1] := 0x41D ;
	  resultado[2] := "*Assoando o nariz*";

	1835:
	  resultado[1] := 0x30E ;
	  resultado[2] := "*Assoando o nariz*";

	225:
	  resultado[1] := 0x0E9 ;

	211:
	  resultado[1] := 0x063 ;

	21:
	  resultado[1] := 0x0DF;

	214:
	  resultado[1] := 0x466;
	  endcase
	endif
	if ( (input[5]==1) )
	case(who.graphic)
	401:
	  resultado[1] := 0x310 ;
	  resultado[2] := "*Comemorando*";

	400:
	  resultado[1] := 0x41F ;
	  resultado[2] := "*Comemorando*";

	1828:
	  resultado[1] := 0x41F ;
	  resultado[2] := "*Comemorando*";

	1829:
	  resultado[1] := 0x310 ;
	  resultado[2] := "*Comemorando*";

	1830:
	  resultado[1] := 0x41F ;
	  resultado[2] := "*Comemorando*";

	1831:
	  resultado[1] := 0x310 ;
	  resultado[2] := "*Comemorando*";

	1832:
	  resultado[1] := 0x41F ;
	  resultado[2] := "*Comemorando*";

	1833:
	  resultado[1] := 0x310 ;
	  resultado[2] := "*Comemorando*";

	1834:
	  resultado[1] := 0x41F ;
	  resultado[2] := "*Comemorando*";

	1835:
	  resultado[1] := 0x310 ;
	  resultado[2] := "*Comemorando*";

	225:
	  resultado[1] := 0x0EA ;

	211:
	  resultado[1] := 0x064 ;

	21:
	  resultado[1] := 0x0E0;

	214:
	  resultado[1] := 0x467;
	  endcase
	endif
	if ( (input[6]==1) )
	case(who.graphic)
	401:
	  resultado[1] := 0x311 ;
	  resultado[2] := "*Limpa a garganta*";
	400:
	  resultado[1] := 0x420 ;
	  resultado[2] := "*Limpa a garganta*";

	1828:
	  resultado[1] := 0x420 ;
	  resultado[2] := "*Limpa a garganta*";
	1829:
	  resultado[1] := 0x311 ;
	  resultado[2] := "*Limpa a garganta*";
	1830:
	  resultado[1] := 0x420 ;
	  resultado[2] := "*Limpa a garganta*";
	1831:
	  resultado[1] := 0x311 ;
	  resultado[2] := "*Limpa a garganta*";
	1832:
	  resultado[1] := 0x420 ;
	  resultado[2] := "*Limpa a garganta*";
	1833:
	  resultado[1] := 0x311 ;
	  resultado[2] := "*Limpa a garganta*";
	1834:
	  resultado[1] := 0x420 ;
	  resultado[2] := "*Limpa a garganta*";
	1835:
	  resultado[1] := 0x311 ;
	  resultado[2] := "*Limpa a garganta*";

	  endcase
	endif
	if ( (input[7]==1) )
	case(who.graphic)
	401:
	  resultado[1] := 0x312 ;
	  resultado[2] := "*Tossindo*";
	400:
	  resultado[1] := 0x421 ;
	  resultado[2] := "*Tossindo*";
	1828:
	  resultado[1] := 0x421 ;
	  resultado[2] := "*Tossindo*";
	1829:
	  resultado[1] := 0x312 ;
	  resultado[2] := "*Tossindo*";
	1830:
	  resultado[1] := 0x421 ;
	  resultado[2] := "*Tossindo*";
	1831:
	  resultado[1] := 0x312 ;
	  resultado[2] := "*Tossindo*";
	1832:
	  resultado[1] := 0x421 ;
	  resultado[2] := "*Tossindo*";
	1833:
	  resultado[1] := 0x312 ;
	  resultado[2] := "*Tossindo*";
	1834:
	  resultado[1] := 0x421 ;
	  resultado[2] := "*Tossindo*";
	1835:
	  resultado[1] := 0x312 ;
	  resultado[2] := "*Tossindo*";


	  endcase
	endif
	if ( (input[8]==1) )
	case(who.graphic)
	401:
	  resultado[1] := 0x313 ;
	  resultado[2] := "*Tossindo*";
	400:
	  resultado[1] := 0x422 ;
	  resultado[2] := "*Tossindo*";

	1828:
	  resultado[1] := 0x422 ;
	  resultado[2] := "*Tossindo*";
	1829:
	  resultado[1] := 0x313 ;
	  resultado[2] := "*Tossindo*";
	1830:
	  resultado[1] := 0x422 ;
	  resultado[2] := "*Tossindo*";
	1831:
	  resultado[1] := 0x313 ;
	  resultado[2] := "*Tossindo*";
	1832:
	  resultado[1] := 0x422 ;
	  resultado[2] := "*Tossindo*";
	1833:
	  resultado[1] := 0x313 ;
	  resultado[2] := "*Tossindo*";
	1834:
	  resultado[1] := 0x422 ;
	  resultado[2] := "*Tossindo*";
	1835:
	  resultado[1] := 0x313 ;
	  resultado[2] := "*Tossindo*";



	  endcase
	endif
	if ( (input[9]==1) )
	case(who.graphic)
	401:
	  resultado[1] := 0x314 ;
	  resultado[2] := "*Chorando*";
	400:
	  resultado[1] := 0x423 ;
	  resultado[2] := "*Chorando*";
	1828:
	  resultado[1] := 0x423 ;
	  resultado[2] := "*Chorando*";
	1829:
	  resultado[1] := 0x314 ;
	  resultado[2] := "*Chorando*";
	1830:
	  resultado[1] := 0x423 ;
	  resultado[2] := "*Chorando*";
	1831:
	  resultado[1] := 0x314 ;
	  resultado[2] := "*Chorando*";
	1832:
	  resultado[1] := 0x423 ;
	  resultado[2] := "*Chorando*";
	1833:
	  resultado[1] := 0x314 ;
	  resultado[2] := "*Chorando*";
	1834:
	  resultado[1] := 0x423 ;
	  resultado[2] := "*Chorando*";
	1835:
	  resultado[1] := 0x314 ;
	  resultado[2] := "*Chorando*";

	  endcase
	endif
	if ( (input[10]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x31A ;
	  resultado[2] := "*Arfando*";
	400:
	  resultado[1] := 0x42A ;
	  resultado[2] := "*Arfando*";
	1828:
	  resultado[1] := 0x42A ;
	  resultado[2] := "*Arfando*";
	1829:
	  resultado[1] := 0x31A ;
	  resultado[2] := "*Arfando*";
	1830:
	  resultado[1] := 0x42A ;
	  resultado[2] := "*Arfando*";
	1831:
	  resultado[1] := 0x31A ;
	  resultado[2] := "*Arfando*";
	1832:
	  resultado[1] := 0x42A ;
	  resultado[2] := "*Arfando*";
	1833:
	  resultado[1] := 0x31A ;
	  resultado[2] := "*Arfando*";
	1834:
	  resultado[1] := 0x42A ;
	  resultado[2] := "*Arfando*";
	1835:
	  resultado[1] := 0x31A ;
	  resultado[2] := "*Arfando*";

	endcase
	endif
	if ( (input[11]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x31B ;
	  resultado[2] := "*Rindo*";
	400:
	  resultado[1] := 0x42B ;
	  resultado[2] := "*Rindo*";
	1828:
	  resultado[1] := 0x42B ;
	  resultado[2] := "*Rindo*";
	1829:
	  resultado[1] := 0x31B ;
	  resultado[2] := "*Rindo*";
	1830:
	  resultado[1] := 0x42B ;
	  resultado[2] := "*Rindo*";
	1831:
	  resultado[1] := 0x31B ;
	  resultado[2] := "*Rindo*";
	1832:
	  resultado[1] := 0x42B ;
	  resultado[2] := "*Rindo*";
	1833:
	  resultado[1] := 0x31B ;
	  resultado[2] := "*Rindo*";
	1834:
	  resultado[1] := 0x42B ;
	  resultado[2] := "*Rindo*";
	1835:
	  resultado[1] := 0x31B ;
	  resultado[2] := "*Rindo*";

endcase
	endif
	if ( (input[12]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x31C ;
	  resultado[2] := "*Gemendo*";
	400:
	  resultado[1] := 0x42C ;
	  resultado[2] := "*Gemendo*";
	1828:
	  resultado[1] := 0x42C ;
	  resultado[2] := "*Gemendo*";
	1829:
	  resultado[1] := 0x31C ;
	  resultado[2] := "*Gemendo*";
	1830:
	  resultado[1] := 0x42C ;
	  resultado[2] := "*Gemendo*";
	1831:
	  resultado[1] := 0x31C ;
	  resultado[2] := "*Gemendo*";
	1832:
	  resultado[1] := 0x42C ;
	  resultado[2] := "*Gemendo*";
	1833:
	  resultado[1] := 0x31C ;
	  resultado[2] := "*Gemendo*";
	1834:
	  resultado[1] := 0x42C ;
	  resultado[2] := "*Gemendo*";
	1835:
	  resultado[1] := 0x31C ;
	  resultado[2] := "*Gemendo*";

endcase
	endif
	if ( (input[13]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x31D ;
	  resultado[2] := "*Rosnando*";
	400:
	  resultado[1] := 0x42D ;
	  resultado[2] := "*Rosnando*";
	1828:
	  resultado[1] := 0x42D ;
	  resultado[2] := "*Rosnando*";
	1829:
	  resultado[1] := 0x31D ;
	  resultado[2] := "*Rosnando*";
	1830:
	  resultado[1] := 0x42D ;
	  resultado[2] := "*Rosnando*";
	1831:
	  resultado[1] := 0x31D ;
	  resultado[2] := "*Rosnando*";
	1832:
	  resultado[1] := 0x42D ;
	  resultado[2] := "*Rosnando*";
	1833:
	  resultado[1] := 0x31D ;
	  resultado[2] := "*Rosnando*";
	1834:
	  resultado[1] := 0x42D ;
	  resultado[2] := "*Rosnando*";
	1835:
	  resultado[1] := 0x31D ;
	  resultado[2] := "*Rosnando*";



endcase
	endif
	if ( (input[14]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x31E ;
	  resultado[2] := "*Hey*";
	400:
	  resultado[1] := 0x42E ;
	  resultado[2] := "*Hey*";
	1828:
	  resultado[1] := 0x42E ;
	  resultado[2] := "*Hey*";
	1829:
	  resultado[1] := 0x31E ;
	  resultado[2] := "*Hey*";
	1830:
	  resultado[1] := 0x42E ;
	  resultado[2] := "*Hey*";
	1831:
	  resultado[1] := 0x31E ;
	  resultado[2] := "*Hey*";
	1832:
	  resultado[1] := 0x42E ;
	  resultado[2] := "*Hey*";
	1833:
	  resultado[1] := 0x31E ;
	  resultado[2] := "*Hey*";
	1834:
	  resultado[1] := 0x42E ;
	  resultado[2] := "*Hey*";
	1835:
	  resultado[1] := 0x31E ;
	  resultado[2] := "*Hey*";

endcase
	endif
	if ( (input[15]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x31F ;
	  resultado[2] := "*Solucando*";
	400:
	  resultado[1] := 0x42F ;
	  resultado[2] := "*Solucando*";
	1828:
	  resultado[1] := 0x42F ;
	  resultado[2] := "*Solucando*";
	1829:
	  resultado[1] := 0x31F ;
	  resultado[2] := "*Solucando*";
	1830:
	  resultado[1] := 0x42F ;
	  resultado[2] := "*Solucando*";
	1831:
	  resultado[1] := 0x31F ;
	  resultado[2] := "*Solucando*";
	1832:
	  resultado[1] := 0x42F ;
	  resultado[2] := "*Solucando*";
	1833:
	  resultado[1] := 0x31F ;
	  resultado[2] := "*Solucando*";
	1834:
	  resultado[1] := 0x42F ;
	  resultado[2] := "*Solucando*";
	1835:
	  resultado[1] := 0x31F ;
	  resultado[2] := "*Solucando*";

endcase
	endif
	if ( (input[16]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x320 ;
	  resultado[2] := "*Huh?*";
	400:
	  resultado[1] := 0x430 ;
	  resultado[2] := "*Huh?*";
	1828:
	  resultado[1] := 0x430 ;
	  resultado[2] := "*Huh?*";
	1829:
	  resultado[1] := 0x320 ;
	  resultado[2] := "*Huh?*";
	1830:
	  resultado[1] := 0x430 ;
	  resultado[2] := "*Huh?*";
	1831:
	  resultado[1] := 0x320 ;
	  resultado[2] := "*Huh?*";
	1832:
	  resultado[1] := 0x430 ;
	  resultado[2] := "*Huh?*";
	1833:
	  resultado[1] := 0x320 ;
	  resultado[2] := "*Huh?*";
	1834:
	  resultado[1] := 0x430 ;
	  resultado[2] := "*Huh?*";
	1835:
	  resultado[1] := 0x320 ;
	  resultado[2] := "*Huh?*";


endcase
	endif
	if ( (input[17]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x321 ;
	  resultado[2] := "*Beijando*";
	400:
	  resultado[1] := 0x431 ;
	  resultado[2] := "*Beijando*";
	1828:
	  resultado[1] := 0x431 ;
	  resultado[2] := "*Beijando*";
	1829:
	  resultado[1] := 0x321 ;
	  resultado[2] := "*Beijando*";
	1830:
	  resultado[1] := 0x431 ;
	  resultado[2] := "*Beijando*";
	1831:
	  resultado[1] := 0x321 ;
	  resultado[2] := "*Beijando*";
	1832:
	  resultado[1] := 0x431 ;
	  resultado[2] := "*Beijando*";
	1833:
	  resultado[1] := 0x321 ;
	  resultado[2] := "*Beijando*";
	1834:
	  resultado[1] := 0x431 ;
	  resultado[2] := "*Beijando*";
	1835:
	  resultado[1] := 0x321 ;
	  resultado[2] := "*Beijando*";

endcase
	endif
	if ( (input[18]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x322 ;
	  resultado[2] := "*Rindo*";
	400:
	  resultado[1] := 0x432 ;
	  resultado[2] := "*Rindo*";
	1828:
	  resultado[1] := 0x432 ;
	  resultado[2] := "*Rindo*";
	1829:
	  resultado[1] := 0x322 ;
	  resultado[2] := "*Rindo*";
	1830:
	  resultado[1] := 0x432 ;
	  resultado[2] := "*Rindo*";
	1831:
	  resultado[1] := 0x322 ;
	  resultado[2] := "*Rindo*";
	1832:
	  resultado[1] := 0x432 ;
	  resultado[2] := "*Rindo*";
	1833:
	  resultado[1] := 0x322 ;
	  resultado[2] := "*Rindo*";
	1834:
	  resultado[1] := 0x432 ;
	  resultado[2] := "*Rindo*";
	1835:
	  resultado[1] := 0x322 ;
	  resultado[2] := "*Rindo*";


endcase
	endif
	if ( (input[19]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x323 ;
	  resultado[2] := "*Negando*";
	400:
	  resultado[1] := 0x433 ;
	  resultado[2] := "*Negando*";
	1828:
	  resultado[1] := 0x433 ;
	  resultado[2] := "*Negando*";
	1829:
	  resultado[1] := 0x323 ;
	  resultado[2] := "*Negando*";
	1830:
	  resultado[1] := 0x433 ;
	  resultado[2] := "*Negando*";
	1831:
	  resultado[1] := 0x323 ;
	  resultado[2] := "*Negando*";
	1832:
	  resultado[1] := 0x433 ;
	  resultado[2] := "*Negando*";
	1833:
	  resultado[1] := 0x323 ;
	  resultado[2] := "*Negando*";
	1834:
	  resultado[1] := 0x433 ;
	  resultado[2] := "*Negando*";
	1835:
	  resultado[1] := 0x323 ;
	  resultado[2] := "*Negando*";




endcase
	endif
	if ( (input[20]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x324 ;
	  resultado[2] := "*Oh*";
	400:
	  resultado[1] := 0x434 ;
	  resultado[2] := "*Oh*";
	1828:
	  resultado[1] := 0x434 ;
	  resultado[2] := "*Oh*";
	1829:
	  resultado[1] := 0x324 ;
	  resultado[2] := "*Oh*";
	1830:
	  resultado[1] := 0x434 ;
	  resultado[2] := "*Oh*";
	1831:
	  resultado[1] := 0x324 ;
	  resultado[2] := "*Oh*";
	1832:
	  resultado[1] := 0x434 ;
	  resultado[2] := "*Oh*";
	1833:
	  resultado[1] := 0x324 ;
	  resultado[2] := "*Oh*";
	1834:
	  resultado[1] := 0x434 ;
	  resultado[2] := "*Oh*";
	1835:
	  resultado[1] := 0x324 ;
	  resultado[2] := "*Oh*";
endcase
	endif
	if ( (input[21]==1) )
	case(who.graphic)
	401:
	  resultado[1] := 0x32C ;
	  resultado[2] := "*Oooh*";
	400:
	  resultado[1] := 0x43E ;
	  resultado[2] := "*Oooh*";
	1828:
	  resultado[1] := 0x43E ;
	  resultado[2] := "*Oooh*";
	1829:
	  resultado[1] := 0x32C ;
	  resultado[2] := "*Oooh*";
	1830:
	  resultado[1] := 0x43E ;
	  resultado[2] := "*Oooh*";
	1831:
	  resultado[1] := 0x32C ;
	  resultado[2] := "*Oooh*";
	1832:
	  resultado[1] := 0x43E ;
	  resultado[2] := "*Oooh*";
	1833:
	  resultado[1] := 0x32C ;
	  resultado[2] := "*Oooh*";
	1834:
	  resultado[1] := 0x43E ;
	  resultado[2] := "*Oooh*";
	1835:
	  resultado[1] := 0x32C ;
	  resultado[2] := "*Oooh*";
endcase
	endif
	if ( (input[22]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x32D ;
	  resultado[2] := "*Oops*";
	400:
	  resultado[1] := 0x43F ;
	  resultado[2] := "*Oops*";
	1828:
	  resultado[1] := 0x43F ;
	  resultado[2] := "*Oops*";
	1829:
	  resultado[1] := 0x32D ;
	  resultado[2] := "*Oops*";
	1830:
	  resultado[1] := 0x43F ;
	  resultado[2] := "*Oops*";
	1831:
	  resultado[1] := 0x32D ;
	  resultado[2] := "*Oops*";
	1832:
	  resultado[1] := 0x43F ;
	  resultado[2] := "*Oops*";
	1833:
	  resultado[1] := 0x32D ;
	  resultado[2] := "*Oops*";
	1834:
	  resultado[1] := 0x43F ;
	  resultado[2] := "*Oops*";
	1835:
	  resultado[1] := 0x32D ;
	  resultado[2] := "*Oops*";
endcase
	endif
	if ( (input[23]==1) )
	case(who.graphic)
	401:
	  resultado[1] := 0x32F ;
	  resultado[2] := "*Gritando*";
	400:
	  resultado[1] := 0x441 ;
	  resultado[2] := "*Gritando*";
	1828:
	  resultado[1] := 0x441 ;
	  resultado[2] := "*Gritando*";
	1829:
	  resultado[1] := 0x32F ;
	  resultado[2] := "*Gritando*";
	1830:
	  resultado[1] := 0x441 ;
	  resultado[2] := "*Gritando*";
	1831:
	  resultado[1] := 0x32F ;
	  resultado[2] := "*Gritando*";
	1832:
	  resultado[1] := 0x441 ;
	  resultado[2] := "*Gritando*";
	1833:
	  resultado[1] := 0x32F ;
	  resultado[2] := "*Gritando*";
	1834:
	  resultado[1] := 0x441 ;
	  resultado[2] := "*Gritando*";
	1835:
	  resultado[1] := 0x32F ;
	  resultado[2] := "*Gritando*";
endcase
	endif
	if ( (input[24]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x330 ;
	  resultado[2] := "*SHHH*";
	400:
	  resultado[1] := 0x442 ;
	  resultado[2] := "*SHHH*";
	1828:
	  resultado[1] := 0x442 ;
	  resultado[2] := "*SHHH*";
	1829:
	  resultado[1] := 0x330 ;
	  resultado[2] := "*SHHH*";
	1830:
	  resultado[1] := 0x442 ;
	  resultado[2] := "*SHHH*";
	1831:
	  resultado[1] := 0x330 ;
	  resultado[2] := "*SHHH*";
	1832:
	  resultado[1] := 0x442 ;
	  resultado[2] := "*SHHH*";
	1833:
	  resultado[1] := 0x330 ;
	  resultado[2] := "*SHHH*";
	1834:
	  resultado[1] := 0x442 ;
	  resultado[2] := "*SHHH*";
	1835:
	  resultado[1] := 0x330 ;
	  resultado[2] := "*SHHH*";
endcase
	endif
	if ( (input[25]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x331 ;
	  resultado[2] := "*Suspirando*";
	400:
	  resultado[1] := 0x443 ;
	  resultado[2] := "*Suspirando*";
	1828:
	  resultado[1] := 0x443 ;
	  resultado[2] := "*Suspirando*";
	1829:
	  resultado[1] := 0x331 ;
	  resultado[2] := "*Suspirando*";
	1830:
	  resultado[1] := 0x443 ;
	  resultado[2] := "*Suspirando*";
	1831:
	  resultado[1] := 0x331 ;
	  resultado[2] := "*Suspirando*";
	1832:
	  resultado[1] := 0x443 ;
	  resultado[2] := "*Suspirando*";
	1833:
	  resultado[1] := 0x331 ;
	  resultado[2] := "*Suspirando*";
	1834:
	  resultado[1] := 0x443 ;
	  resultado[2] := "*Suspirando*";
	1835:
	  resultado[1] := 0x331 ;
	  resultado[2] := "*Suspirando*";


endcase
	endif
	if ( (input[26]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x332 ;
	  resultado[2] := "*Espirrando*";
	400:
	  resultado[1] := 0x444 ;
	  resultado[2] := "*Espirrando*";
	1828:
	  resultado[1] := 0x444 ;
	  resultado[2] := "*Espirrando*";
	1829:
	  resultado[1] := 0x332 ;
	  resultado[2] := "*Espirrando*";
	1830:
	  resultado[1] := 0x444 ;
	  resultado[2] := "*Espirrando*";
	1831:
	  resultado[1] := 0x332 ;
	  resultado[2] := "*Espirrando*";
	1832:
	  resultado[1] := 0x444 ;
	  resultado[2] := "*Espirrando*";
	1833:
	  resultado[1] := 0x332 ;
	  resultado[2] := "*Espirrando*";
	1834:
	  resultado[1] := 0x444 ;
	  resultado[2] := "*Espirrando*";
	1835:
	  resultado[1] := 0x332 ;
	  resultado[2] := "*Espirrando*";

endcase
	endif
	if ( (input[27]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x333 ;
	  resultado[2] := "*Sniff*";
	400:
	  resultado[1] := 0x445 ;
	  resultado[2] := "*Sniff*";
	1828:
	  resultado[1] := 0x445 ;
	  resultado[2] := "*Sniff*";
	1829:
	  resultado[1] := 0x333 ;
	  resultado[2] := "*Sniff*";
	1830:
	  resultado[1] := 0x445 ;
	  resultado[2] := "*Sniff*";
	1831:
	  resultado[1] := 0x333 ;
	  resultado[2] := "*Sniff*";
	1832:
	  resultado[1] := 0x445 ;
	  resultado[2] := "*Sniff*";
	1833:
	  resultado[1] := 0x333 ;
	  resultado[2] := "*Sniff*";
	1834:
	  resultado[1] := 0x445 ;
	  resultado[2] := "*Sniff*";
	1835:
	  resultado[1] := 0x333 ;
	  resultado[2] := "*Sniff*";
endcase
	endif
	if ( (input[28]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x334 ;
	  resultado[2] := "*Roncando*";
	400:
	  resultado[1] := 0x446 ;
	  resultado[2] := "*Roncando*";
	1828:
	  resultado[1] := 0x446 ;
	  resultado[2] := "*Roncando*";
	1829:
	  resultado[1] := 0x334 ;
	  resultado[2] := "*Roncando*";
	1830:
	  resultado[1] := 0x446 ;
	  resultado[2] := "*Roncando*";
	1831:
	  resultado[1] := 0x334 ;
	  resultado[2] := "*Roncando*";
	1832:
	  resultado[1] := 0x446 ;
	  resultado[2] := "*Roncando*";
	1833:
	  resultado[1] := 0x334 ;
	  resultado[2] := "*Roncando*";
	1834:
	  resultado[1] := 0x446 ;
	  resultado[2] := "*Roncando*";
	1835:
	  resultado[1] := 0x334 ;
	  resultado[2] := "*Roncando*";
endcase
	endif
	if ( (input[29]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x335 ;
	  resultado[2] := "*Cuspindo*";
	400:
	  resultado[1] := 0x447 ;
	  resultado[2] := "*Cuspindo*";
	1828:
	  resultado[1] := 0x447 ;
	  resultado[2] := "*Cuspindo*";
	1829:
	  resultado[1] := 0x335 ;
	  resultado[2] := "*Cuspindo*";
	1830:
	  resultado[1] := 0x447 ;
	  resultado[2] := "*Cuspindo*";
	1831:
	  resultado[1] := 0x335 ;
	  resultado[2] := "*Cuspindo*";
	1832:
	  resultado[1] := 0x447 ;
	  resultado[2] := "*Cuspindo*";
	1833:
	  resultado[1] := 0x335 ;
	  resultado[2] := "*Cuspindo*";
	1834:
	  resultado[1] := 0x447 ;
	  resultado[2] := "*Cuspindo*";
	1835:
	  resultado[1] := 0x335 ;
	  resultado[2] := "*Cuspindo*";

endcase
	endif
	if ( (input[30]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x336 ;
	  resultado[2] := "*Assobiando*";
	400:
	  resultado[1] := 0x448 ;
	  resultado[2] := "*Assobiando*";
	1828:
	  resultado[1] := 0x448 ;
	  resultado[2] := "*Assobiando*";
	1829:
	  resultado[1] := 0x336 ;
	  resultado[2] := "*Assobiando*";
	1830:
	  resultado[1] := 0x448 ;
	  resultado[2] := "*Assobiando*";
	1831:
	  resultado[1] := 0x336 ;
	  resultado[2] := "*Assobiando*";
	1832:
	  resultado[1] := 0x448 ;
	  resultado[2] := "*Assobiando*";
	1833:
	  resultado[1] := 0x336 ;
	  resultado[2] := "*Assobiando*";
	1834:
	  resultado[1] := 0x448 ;
	  resultado[2] := "*Assobiando*";
	1835:
	  resultado[1] := 0x336 ;
	  resultado[2] := "*Assobiando*";

endcase
	endif
	if ( (input[31]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x337 ;
	  resultado[2] := "*Bocejando*";
	400:
	  resultado[1] := 0x449 ;
	  resultado[2] := "*Bocejando*";
	1828:
	  resultado[1] := 0x449 ;
	  resultado[2] := "*Bocejando*";
	1829:
	  resultado[1] := 0x337 ;
	  resultado[2] := "*Bocejando*";
	1830:
	  resultado[1] := 0x449 ;
	  resultado[2] := "*Bocejando*";
	1831:
	  resultado[1] := 0x337 ;
	  resultado[2] := "*Bocejando*";
	1832:
	  resultado[1] := 0x449 ;
	  resultado[2] := "*Bocejando*";
	1833:
	  resultado[1] := 0x337 ;
	  resultado[2] := "*Bocejando*";
	1834:
	  resultado[1] := 0x449 ;
	  resultado[2] := "*Bocejando*";
	1835:
	  resultado[1] := 0x337 ;
	  resultado[2] := "*Bocejando*";
endcase
	endif
	if ( (input[32]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x338 ;
	  resultado[2] := "*Yeah*";
	400:
	  resultado[1] := 0x44A ;
	  resultado[2] := "*Yeah*";
	1828:
	  resultado[1] := 0x44A ;
	  resultado[2] := "*Yeah*";
	1829:
	  resultado[1] := 0x338 ;
	  resultado[2] := "*Yeah*";
	1830:
	  resultado[1] := 0x44A ;
	  resultado[2] := "*Yeah*";
	1831:
	  resultado[1] := 0x338 ;
	  resultado[2] := "*Yeah*";
	1832:
	  resultado[1] := 0x44A ;
	  resultado[2] := "*Yeah*";
	1833:
	  resultado[1] := 0x338 ;
	  resultado[2] := "*Yeah*";
	1834:
	  resultado[1] := 0x44A ;
	  resultado[2] := "*Yeah*";
	1835:
	  resultado[1] := 0x338 ;
	  resultado[2] := "*Yeah*";

endcase
	endif
	if ( (input[33]==1) )
	case (who.graphic)
	401:
	  resultado[1] := 0x339 ;
	  resultado[2] := "*Gritando*";
	400:
	  resultado[1] := 0x44B ;
	  resultado[2] := "*Gritando*";
	1828:
	  resultado[1] := 0x44B ;
	  resultado[2] := "*Gritando*";
	1829:
	  resultado[1] := 0x339 ;
	  resultado[2] := "*Gritando*";
	1830:
	  resultado[1] := 0x44B ;
	  resultado[2] := "*Gritando*";
	1831:
	  resultado[1] := 0x339 ;
	  resultado[2] := "*Gritando*";
	1832:
	  resultado[1] := 0x44B ;
	  resultado[2] := "*Gritando*";
	1833:
	  resultado[1] := 0x339 ;
	  resultado[2] := "*Gritando*";
	1834:
	  resultado[1] := 0x44B ;
	  resultado[2] := "*Gritando*";
	1835:
	  resultado[1] := 0x339 ;
	  resultado[2] := "*Gritando*";

endcase
	endif

	return resultado;
endfunction

function GetSoundLayout(who)
	case(who.graphic)
				//pantera
				214:
				  return array(
				 		"page 0",
						"resizepic 50 50 3600 430 430",
			            "checkertrans 50 50 430 430",
						"text 160 65 500 0",
						"gumppic 70 85 96",
						"button 70 100 2103 2104 1 0 1",
						"text 90 95 500 1",
						"button 70 120 2103 2104 1 0 2",
						"text 90 115 500 2",
						"button 70 140 2103 2104 1 0 3",
						"text 90 135 500 3",
						"button 70 160 2103 2104 1 0 4",
						"text 90 155 500 4",
						"button 70 180 2103 2104 1 0 5",
						"text 90 175 500 5",
						"button 235 440 2121 2120 1 0 0"
					);
				//serpente gigante
				21:
				  return array(
				 		"page 0",
						"resizepic 50 50 3600 430 430",
			            "checkertrans 50 50 430 430",
						"text 160 65 500 0",
						"gumppic 70 85 96",
						"button 70 100 2103 2104 1 0 1",
						"text 90 95 500 1",
						"button 70 120 2103 2104 1 0 2",
						"text 90 115 500 2",
						"button 70 140 2103 2104 1 0 3",
						"text 90 135 500 3",
						"button 70 160 2103 2104 1 0 4",
						"text 90 155 500 4",
						"button 70 180 2103 2104 1 0 5",
						"text 90 175 500 5",
						"button 235 440 2121 2120 1 0 0"
					);
				//urso negro
				211:
				  return array(
				 		"page 0",
						"resizepic 50 50 3600 430 430",
			            "checkertrans 50 50 430 430",
						"text 160 65 500 0",
						"gumppic 70 85 96",
						"button 70 100 2103 2104 1 0 1",
						"text 90 95 500 1",
						"button 70 120 2103 2104 1 0 2",
						"text 90 115 500 2",
						"button 70 140 2103 2104 1 0 3",
						"text 90 135 500 3",
						"button 70 160 2103 2104 1 0 4",
						"text 90 155 500 4",
						"button 70 180 2103 2104 1 0 5",
						"text 90 175 500 5",
						"button 235 440 2121 2120 1 0 0"
					);
				//lobo
				225:
				  return array(
				 		"page 0",
						"resizepic 50 50 3600 430 430",
			            "checkertrans 50 50 430 430",
						"text 160 65 500 0",
						"gumppic 70 85 96",
						"button 70 100 2103 2104 1 0 1",
						"text 90 95 500 1",
						"button 70 120 2103 2104 1 0 2",
						"text 90 115 500 2",
						"button 70 140 2103 2104 1 0 3",
						"text 90 135 500 3",
						"button 70 160 2103 2104 1 0 4",
						"text 90 155 500 4",
						"button 70 180 2103 2104 1 0 5",
						"text 90 175 500 5",
						"button 235 440 2121 2120 1 0 0"
					);
				//qualquer outra coisa
				default:
					return array(
				 		"page 0",
						"resizepic 50 50 3600 430 430",
			            "checkertrans 50 50 430 430",
						"text 160 65 500 0",
						"gumppic 70 85 96",
						"button 70 100 2103 2104 1 0 1",
						"text 90 95 49 1",
						"button 70 120 2103 2104 1 0 2",
						"text 90 115 49 2",
						"button 70 140 2103 2104 1 0 3",
						"text 90 135 49 3",
						"button 70 160 2103 2104 1 0 4",
						"text 90 155 49 4",
						"button 70 180 2103 2104 1 0 5",
						"text 90 175 300 5",
						"button 70 200 2103 2104 1 0 6",
						"text 90 195 300 6",
						"button 70 220 2103 2104 1 0 7",
						"text 90 215 300 7",
						"button 70 240 2103 2104 1 0 8",
						"text 90 235 300 8",
						"button 70 260 2103 2104 1 0 9",
						"text 90 255 300 9",
						"button 70 280 2103 2104 1 0 10",
						"text 90 275 500 10",
						"button 70 300 2103 2104 1 0 11",
						"text 90 295 500 11",
						"button 70 320 2103 2104 1 0 12",
						"text 90 315 500 12",
						"button 70 340 2103 2104 1 0 13",
						"text 90 335 500 13",
						"button 70 360 2103 2104 1 0 14",
						"text 90 355 500 14",
						"button 70 380 2103 2104 1 0 15",
						"text 90 375 500 15",
						"button 70 400 2103 2104 1 0 16",
						"text 90 395 500 16",
						"button 70 420 2103 2104 1 0 17",
						"text 90 415 500 17",
						"gumppic 250 85 96",
						"gumppic 280 85 96",
						"button 260 100 2103 2104 1 1 18",
						"text 280 95 500 18",
						"button 260 120 2103 2104 1 1 19",
						"text 280 115 500 19",
						"button 260 140 2103 2104 1 1 20",
						"text 280 135 500 20",
						"button 260 160 2103 2104 1 1 21",
						"text 280 155 500 21",
						"button 260 180 2103 2104 1 1 22",
						"text 280 175 500 22",
						"button 260 200 2103 2104 1 1 23",
						"text 280 195 500 23",
						"button 260 220 2103 2104 1 1 24",
						"text 280 215 500 24",
						"button 260 240 2103 2104 1 1 25",
						"text 280 235 500 25",
						"button 260 260 2103 2104 1 1 26",
						"text 280 255 500 26",
						"button 260 280 2103 2104 1 1 27",
						"text 280 275 500 27",
						"button 260 300 2103 2104 1 1 28",
						"text 280 295 500 28",
						"button 260 320 2103 2104 1 1 29",
						"text 280 315 500 29",
						"button 260 340 2103 2104 1 1 30",
						"text 280 335 500 30",
						"button 260 360 2103 2104 1 1 31",
						"text 280 355 500 31",
						"button 260 380 2103 2104 1 1 32",
						"text 280 375 500 32",
						"button 260 400 2103 2104 1 1 33",
						"text 280 395 500 33",
						"button 235 440 2121 2120 1 0 0"
					);
			endcase
endfunction

function GetSound(who, text)
		var data  := GetSoundDataArray(who);

		var layout := GetSoundLayout(who);

		text := splitwords(text);

		var input  := array;

		var delay := 1;
		var repeatcount := 1;
		var repeatflag := 0;
		if (text.size() == 0)
			input := SendDialogGump( who, Layout, Data );
			if ( (input[0]==0) )
				SendSysMessage(who, "Cancelled.");
				return;
			endif
		else

			var k := 0;
			while (k < 33)
				input[k] := 0;
				k := k + 1;
			endwhile

	    if (GetSoundNumber(text[1], who) > 0)
	      text[1] := CStr(GetSoundNumber(text[1], who));
	    endif

			if (cint(text[1]) >= 1)
				input[cint(text[1])] := 1;
			endif

			//sendsysmessage(who, "input " + text[1] );

			var i := 1;

			for(i := 1; i < text.size(); i := i + 1)
				if (text[i] == "d")
					delay := cint(text[i+1]);
					if (delay == 0)
						delay := 1;
					endif
				elseif (text[i] == "l")
					repeatcount := cint(text[i+1]);
					if (repeatcount == 0)
						repeatcount := 1;
						repeatflag := 0;
					endif
				endif
			endfor
	endif


	who.hidden := 0;

	return GetSoundEffect(who, input);
endfunction

function GetSoundNumber(sound, who)
	case(who.graphic)
				//pantera
				214:
					case(Lower(sound))
						"rosnar":
						  return 1;

						"rugir":
						  return 2;

						"rosnar2":
						  return 3;

						 "morder":
						  return 4;

						"chorar":
						  return 5;
					endcase
				//serpente gigante
				21:
					case(Lower(sound))
						"sibilar":
						  return 1;

						"sibilar2":
						  return 2;

						"sibilar3":
						  return 3;

						 "morder":
						  return 4;

						"chorar":
						  return 5;
					endcase
				//urso negro
				211:
					case(Lower(sound))
						"rosnar":
						  return 1;

						"bramir":
						  return 2;

						"rosnar2":
						  return 3;

						 "morder":
						  return 4;

						"chorar":
						  return 5;
					endcase
				//lobo
				225:
					case(Lower(sound))
						"rosnar":
						  return 1;

						"uivar":
						  return 2;

						"rosnar2":
						  return 3;

						 "morder":
						  return 4;

						"chorar":
						  return 5;
					endcase
				//qualquer outra coisa
				default:
					case(Lower(sound))
						"ah":
						  return 1;

						"ahha":
						  return 2;

						"aplaudir":
						  return 3;

						 "assoar":
						  return 4;

						"comemorar":
						  return 5;

						"limpargarganta":
						  return 6;

						"tossir":
						  return 7;

						"tossir2":
						  return 8;

						"chorar":
						  return 9;

						"arfar":
						  return 10;

						"risadinha":
						  return 11;

						"gemer":
						  return 12;

						"rosnar":
						  return 13;

						"hey":
						  return 14;

						"solucar":
						  return 15;

						"huh?":
						  return 16;

						"beijar":
						  return 17;

						"rir":
						  return 18;

						 "no":
						  return 19;

						"oh":
						  return 20;

						"oooh":
						  return 21;

						"oops":
						  return 22;

						"gritar":
						  return 23;

						"shhh":
						  return 24;

						"suspirar":
						  return 25;

						"espirrar":
						  return 26;

						 "sniff":
						  return 27;

						"roncar":
						  return 28;

						"cuspir":
						  return 29;

						"assobiar":
						  return 30;

						"bocejar":
						  return 31;

						"yeah":
						  return 32;

						"gritar2":
						  return 33;
					endcase
			endcase

	return 0;

endfunction

function GetActionNumber(action)
	case(Lower(action))
		"andardesarmado":
			return 1;
		"andararmado":
			return 2;
		"andarmodocombate":
			return 3;
		"correrdesarmado":
			return 4;
		"correrarmado":
			return 5;
		"olharemvolta":
			return 6;
		"inquietar":
			return 7;
		"soltarfeitico":
			return 8;
		"soltarfeitico2":
			return 9;
		"curvar":
			return 10;
		"saudar":
			return 11;
		"comer":
			return 12;
		"esquivar":
			return 13;
		"socar":
			return 14;
		"cairparatras":
			return 15;
		"cairparafrente":
			return 16;
		"levargolpe":
			return 17;
		"atirarcomarco":
			return 18;
		"atirarcombesta":
			return 19;
		"pular":
			return 20;
		"atacarmontado":
			return 21;
		"atacarmontadocomarco":
			return 22;
		"atacarmontadocombesta":
			return 23;
		"cavalgarlentamente":
			return 24;
		"cavalgarrapidamente":
			return 25;
		"baternocavalo":
			return 26;
		"modocombate1":
			return 27;
		"modocombate2":
			return 28;
		"atacar":
			return 29;
		"apunhalar":
			return 30;
		"atacar2":
			return 31;
		"atacar3":
			return 32;
		"atacar4":
			return 33;
		"furar":
			return 34;
	endcase
	return 0;

endfunction

function GetActionLayoutArray()
	return array(

		 		"page 0",
				"resizepic 50 50 3600 430 430",
				"text 160 65 500 0",
				"gumppic 70 85 96",
				"button 70 100 2103 2104 1 0 1",
				"text 90 95 500 1",
				"button 70 120 2103 2104 1 0 2",
				"text 90 115 500 2",
				"button 70 140 2103 2104 1 0 3",
				"text 90 135 500 3",
				"button 70 160 2103 2104 1 0 4",
				"text 90 155 500 4",
				"button 70 180 2103 2104 1 0 5",
				"text 90 175 500 5",
				"button 70 200 2103 2104 1 0 6",
				"text 90 195 500 6",
				"button 70 220 2103 2104 1 0 7",
				"text 90 215 500 7",
				"button 70 240 2103 2104 1 0 8",
				"text 90 235 500 8",
				"button 70 260 2103 2104 1 0 9",
				"text 90 255 500 9",
				"button 70 280 2103 2104 1 0 10",
				"text 90 275 500 10",
				"button 70 300 2103 2104 1 0 11",
				"text 90 295 500 11",
				"button 70 320 2103 2104 1 0 12",
				"text 90 315 500 12",
				"button 70 340 2103 2104 1 0 13",
				"text 90 335 500 13",
				"button 70 360 2103 2104 1 0 14",
				"text 90 355 500 14",
				"button 70 380 2103 2104 1 0 15",
				"text 90 375 500 15",
				"button 70 400 2103 2104 1 0 16",
				"text 90 395 500 16",
				"button 70 420 2103 2104 1 0 17",
				"text 90 415 500 17",
				"gumppic 250 85 96",
				"gumppic 280 85 96",
				"button 260 100 2103 2104 1 1 18",
				"text 280 95 500 18",
				"button 260 120 2103 2104 1 1 19",
				"text 280 115 500 19",
				"button 260 140 2103 2104 1 1 20",
				"text 280 135 500 20",
				"button 260 160 2103 2104 1 1 21",
				"text 280 155 500 21",
				"button 260 180 2103 2104 1 1 22",
				"text 280 175 500 22",
				"button 260 200 2103 2104 1 1 23",
				"text 280 195 500 23",
				"button 260 220 2103 2104 1 1 24",
				"text 280 215 500 24",
				"button 260 240 2103 2104 1 1 25",
				"text 280 235 500 25",
				"button 260 260 2103 2104 1 1 26",
				"text 280 255 500 26",
				"button 260 280 2103 2104 1 1 27",
				"text 280 275 500 27",
				"button 260 300 2103 2104 1 1 28",
				"text 280 295 500 28",
				"button 260 320 2103 2104 1 1 29",
				"text 280 315 500 29",
				"button 260 340 2103 2104 1 1 30",
				"text 280 335 500 30",
				"button 260 360 2103 2104 1 1 31",
				"text 280 355 500 31",
				"button 260 380 2103 2104 1 1 32",
				"text 280 375 500 32",
				"button 260 400 2103 2104 1 1 33",
				"text 280 395 500 33",
				"button 260 420 2103 2104 1 1 34",
				"text 280 415 500 34",
				"button 235 440 2121 2120 1 0 0"
			);
endfunction

function GetActionDataArray()
	return array(

				"esclha sua ação:",
				"1 - andardesarmado",
				"2 - andararmado",
				"3 - andarmodocombate",
				"4 - correrdesarmado",
				"5 - correrarmado",
				"6 - olharemvolta",
				"7 - inquietar",
				"8 - soltarfeitico",
				"9 - soltarfeitico2",
				"10 - curvar",
				"11 - saudar",
				"12 - comer",
				"13 - esquivar",
				"14 - socar",
				"15 - cairparatras",
				"16 - cairparafrente",
				"17 - levargolpe",
				"18 - atirarcomarco",
				"19 - atirarcombesta",
				"20 - pular",
				"21 - atacarmontado",
				"22 - atacarmontadocomarco",
				"23 - atacarmontadocombesta",
				"24 - cavalgarlentamente",
				"25 - cavalgarrapidamente",
				"26 - baternocavalo",
				"27 - modocombate1",
				"28 - modocombate2",
				"29 - atacar",
				"30 - apunhalar",
				"31 - atacar2",
				"32 - atacar3",
				"33 - atacar4",
				"34 - furar"
			);
endfunction

function GetActionEffect(input, repeatcount, repeatflag, delay)
	var resultado := {};

	if ( (input[1]==1) )
		resultado[1] := 0x00;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[2]==1) )
		resultado[1] := 0x01;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[3]==1) )
		resultado[1] := 0x0f;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[4]==1) )
		resultado[1] := 0x02;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[5]==1) )
		resultado[1] := 0x03;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[6]==1) )
		resultado[1] := 0x05;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[7]==1) )
		resultado[1] := 0x06;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[8]==1) )
		resultado[1] := 0x10;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[9]==1) )
		resultado[1] := 0x11;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[10]==1) )
		resultado[1] := 0x20;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[11]==1) )
		resultado[1] := 0x21;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[12]==1) )
		resultado[1] := 0x22;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[13]==1) )
		resultado[1] := 0x1e;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[14]==1) )
		resultado[1] := 0x1f;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[15]==1) )
		resultado[1] := 0x15;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[16]==1) )
		resultado[1] := 0x16;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[17]==1) )
		resultado[1] := 0x14;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[18]==1) )
		resultado[1] := 0x12;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[19]==1) )
		resultado[1] := 0x13;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[20]==1) )
		resultado[1] := 0x19;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[21]==1) )
		resultado[1] := 0x1a;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[22]==1) )
		resultado[1] := 0x1b;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[23]==1) )
		resultado[1] := 0x1c;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[24]==1) )
		resultado[1] := 0x17;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[25]==1) )
		resultado[1] := 0x18;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[26]==1) )
		resultado[1] := 0x1d;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[27]==1) )
		resultado[1] := 0x07;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[28]==1) )
		resultado[1] := 0x08;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[29]==1) )
		resultado[1] := 0x09;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[30]==1) )
		resultado[1] := 0x0a;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif

	if ( (input[31]==1) )
		resultado[1] := 0x0b;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif

	if ( (input[32]==1) )
		resultado[1] := 0x0d;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[33]==1) )
		resultado[1] := 0x0c;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif
	if ( (input[34]==1) )
		resultado[1] := 0x0e;
		resultado[2] := 5;
		resultado[3] := repeatcount;
		resultado[4] := ACTION_DIR_FORWARD;
		resultado[5] := repeatflag;
		resultado[6] := delay;
	endif

	return resultado;
endfunction

function GetAction(who, text)
		var layout := GetActionLayoutArray();

		var data := GetActionDataArray();

		text := splitwords(text);

		var input  := array;
	//	sendsysmessage(who, " " + text.size());

		var delay := 1;
		var repeatcount := 1;
		var repeatflag := 0;
		if (text.size() == 0)
			input := SendDialogGump( who, Layout, Data );
			if ( (input[0]==0) )
				SendSysMessage(who, "Cancelled.");
				return;
			endif
		else

			var k := 0;
			while (k < 34)
				input[k] := 0;
				k := k + 1;
			endwhile

			if (GetActionNumber(text[1]) > 0)
		      text[1] := CStr(GetActionNumber(text[1]));
		    endif

			if (cint(text[1]) >= 1)
				input[cint(text[1])] := 1;
			endif
			//sendsysmessage(who, "input " + text[1] );
			var i := 1;
			for(i := 1; i < text.size(); i := i + 1)
				if (text[i] == "d")
					delay := cint(text[i+1]);
					if (delay == 0)
						delay := 1;
					endif
				elseif (text[i] == "l")
					repeatcount := cint(text[i+1]);
					if (repeatcount == 0)
						repeatcount := 1;
						repeatflag := 0;
					else
						repeatflag := 1;
					endif
				endif
			endfor
	endif

	return GetActionEffect(input, repeatcount, repeatflag, delay);

endfunction