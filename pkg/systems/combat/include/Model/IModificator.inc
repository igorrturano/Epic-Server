enum class MOD_SOURCES
	CRAFTING := "fabricacao"
endenum

class IModificator()
	uninit function GetSource(this);
	uninit function GetMod(this);
	uninit function SetMod(this, mod);
endclass