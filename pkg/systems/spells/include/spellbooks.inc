
//tipo do book eh pra saber se pode inserir scrolls de um certo tipo em um certo book
function GetBookType(book)

	var type := GetObjProperty(book, "Booktype");
	if (type == error)
		return "mage";
	else
		return type;
	endif

endfunction 

function CanInsertScroll(book, scroll)

	var booktype := GetBookType(book);
	var cfg := ReadConfigFile(":spells:itemdesc");
	var scrolltype := cfg[scroll.objtype].SpellType;

	//printtextabove(book, " " + bookcfg[booktype].spelltype + " " + scrolltype);

	if ( lower(GetBookType(book)) == lower(scrolltype))
		return 1;
	else
		return 0;
	endif

endfunction
