include "IMastery";
include ":mastery:exceptions/InvalidMasteryException";
include "system/exception/FormatException/InvalidFormatException";

/**
 * Representa uma maestria de um jogador ou objeto.
 * 
 * A classe Mastery encapsula todas as informações relacionadas a uma maestria,
 * incluindo seu nível, experiência acumulada e atributos específicos.
 * 
 * @author EvilLink
 * @version 2.0
 * 
 * @example
 *   var mastery := Mastery(weaponRef);
 *   mastery.SetLevel(10)
 *          .AddXP(500)
 *          .SetAttribute(MASTERY_ATTRIBUTE.HITS, 100)
 *          .SetAttribute(MASTERY_ATTRIBUTE.KILLS, 50);
 */
 class Mastery(IMastery)
	var Ref; // Referência ao que pertence a maestria (ex: objType da arma, skillId, etc.) [Não utilizar strings, use alguma referência]
	var Description; // Descrição da maestria (opcional)
	var Type; // Tipo da maestria (ex: COMBAT, WORKING, etc.)
	var Level;
	var XP;
	var Attributes := dictionary{}; // attributes[MASTERY_ATTRIBUTE] := 0;

	function Mastery(this, ref)
		this.Ref := ref;
		this.Description := "";
		this.Level := CInt(0);
		this.XP := CInt(0);
		this.Attributes := dictionary{};
	endfunction

	function GetType(this)
		return this.Type;
	endfunction

	function SetType(this, value)
		this.Type := value;

		return this;
	endfunction

	function GetDescription(this)
		return this.Description;
	endfunction

	function SetDescription(this, value)
		this.Description := CStr(value);

		return this;
	endfunction

	function GetLevel(this)
		return this.Level;
	endfunction

	function SetLevel(this, value)
		this.Level := CInt(value);

		return this;
	endfunction

	function AddLevel(this, value)
		this.Level += CInt(value);

		return this;
	endfunction

	function GetXP(this)
		return this.XP;
	endfunction

	function SetXP(this, value)
		this.XP := CInt(value);

		return this;
	endfunction

	function AddXP(this, value)
		this.XP += CInt(value);

		return this;
	endfunction

	function GetAttribute(this, attribute)
		if (this.Attributes.ContainsKey(attribute))
			return this.Attributes[attribute];
		else
			return InvalidMasteryException("Atributo de maestria inválido: " + CStr(attribute));
		endif
	endfunction

	/**
	 * Define o valor de um atributo de maestria.
	 * 
	 * Armazena um valor inteiro associado ao atributo especificado no dicionário
	 * de atributos da maestria. O valor é convertido para inteiro antes de ser
	 * armazenado.
	 * 
	 * @param attribute Um valor do enum MASTERY_ATTRIBUTE que identifica o atributo a ser definido
	 * @param value     O valor a ser atribuído ao atributo (será convertido para inteiro)
	 * 
	 * @return Esta instância de Mastery para encadeamento de métodos
	 * 
	 * @example
	 *   mastery.SetAttribute(MASTERY_ATTRIBUTE::HITS, 100)
	 *          .SetAttribute(MASTERY_ATTRIBUTE::KILLS, 50);
	 */
	function SetAttribute(this, attribute, value)
		this.Attributes[attribute] := CInt(value);

		return this;
	endfunction

	function IncreaseAttribute(this, attribute, value)
		if (this.Attributes.ContainsKey(attribute))
			this.Attributes[attribute] += CInt(value);
		else
			this.Attributes[attribute] := CInt(value);
		endif

		return this;
	endfunction

endclass

enum class MASTERY_TYPE
	COMBAT,
	WORKING
endenum

enum class MASTERY_ATTRIBUTE
	HITS,
	KILLS,
	DAMAGE
endenum