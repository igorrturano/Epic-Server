// Estes objtypes precisam bater com as chaves Resource em
// pkg/systems/crafting/config/resourceList.cfg, que e' o que o crafting
// grava na CProp "material" do item. Comparar com valores fora de sync
// faz qualquer if (material == MATERIAL_OBJTYPE::X) virar no-op silencioso
// (ja queimou a magia Encantar Objeto pra quase todos os metais).
enum class MATERIAL_OBJTYPE
	AREIA := 0x6075,
	COBRE := 0x6465,
	BRONZE := 0x6994,
	ARGILA := 0x6074,
	FERRO := 0x6309,
	FULMIUM := 0x630b,
	VENETITA := 0x6303,
	VENETIUM := 0x630c,
	IGNIUM := 0x630d,
	OURO := 0x630e,
	ATRIUM := 0x6310,
	MITHRIL := 0x6311,
	FRIGUTIUM := 0x6460,
	ILLYRA := 0x6315,
	PRATA := 0x6993,
	ESTANHO := 0x6990,
	PEDRAS := 0xBD11,
	MARMORE := 0x8850,
	ARENITO := 0x8852,
	FULMIUM := 0x630b,
	ESCAMOSO := 0xef20,
	DRACONIANO := 0xed29,
	GIGANTE := 0xef27,
	POLAR := 0xef26
endenum

enum class MATERIAL_NAME
	AREIA := "Areia",
	COBRE := "Cobre",
	BRONZE := "Bronze",
	ARGILA := "Argila",
	FERRO := "Ferro",
	FULMIUM := "Fulmium",
	VENETITA := "Venetita",
	IGNIUM := "Ignium",
	OURO := "Ouro",
	ATRIUM := "Atrium",
	MITHRIL := "Mithril",
	FRIGUTIUM := "Frigutium",
	ILLYRA := "Illyra",
	PRATA := "Prata",
	ESTANHO := "Estanho",
	PEDRAS := "Pedras",
	MARMORE := "Marmore",
	ARENITO := "Arenito",
	FULMIUM := "Fulmium",
	ESCAMOSO := "Escamoso",
	DRACONIANO := "Draconiano",
	GIGANTE := "Gigante",
	POLAR := "Polar"
endenum