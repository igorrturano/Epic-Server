include ":merchants:cambio";

print("INSTALLING: Methods for Character -> Merchants System... OK!");

exported function spendgold( who , quantidade)
	return EfetuaDebito(who, quantidade);
endfunction