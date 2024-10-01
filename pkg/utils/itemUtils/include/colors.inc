use uo;
use os;
use util;

function RandomPinkHue()

	return RandomIntMinMax( 1201, 1255 );
endfunction

function RandomBlueHue()

	return RandomIntMinMax( 1301, 1355 );
endfunction

function RandomGreenHue()

	return RandomIntMinMax( 1401, 1455 );
endfunction

function RandomOrangeHue()

	return RandomIntMinMax( 1501, 1555 );
endfunction

function RandomRedHue()

	return RandomIntMinMax( 1601, 1655 );
endfunction

function RandomYellowHue()

	return RandomIntMinMax( 1701, 1755 );
endfunction

function RandomNeutralHue()

	return RandomIntMinMax( 1801, 1909 );
endfunction

function RandomDyedHue()

	return RandomIntMinMax( 2, 1001 );
endfunction

// defs_hues.scp do POL by GERGEBOY

//Rosa
var cores_rosa := RandomIntMinMax(1201, 1254);

//Azul
var cores_azul := RandomIntMinMax(1301, 1354);

//Verde
var cores_verde := RandomIntMinMax(1401, 1454);

//Laranja
var cores_laranja := RandomIntMinMax(1501,  1554);

//Vermelho
var cores_vermelho := RandomIntMinMax(1601, 1654);

//Amarelo
var cores_amarelo := RandomIntMinMax(1701, 1754);

//Neutro (marrons, etc.)
var cores_neutro := RandomIntMinMax(1801, 1908);

//Branco/Cinza
var cores_cinza := array{0, 946, 947};

//Cores Gerais
var cores_geral := array{cores_rosa, cores_azul, cores_verde, cores_laranja, cores_vermelho, cores_amarelo, cores_neutro, cores_cinza};


// Cores dos Povos -- Roupas

//Aigla
var cores_aigla := array{cores_vermelho, cores_amarelo, cores_neutro};

//Björske
var cores_bjorske := array{cores_verde, cores_amarelo, cores_neutro};

//Aluriel
var cores_aluriel := array{cores_rosa, cores_azul, cores_verde, cores_neutro};

//Har'oloth
var cores_haroloth := array{cores_rosa, cores_cinza, cores_azul, cores_neutro};

//Badûran
var cores_baduran := array{cores_laranja, cores_amarelo, cores_neutro, cores_cinza};

//Polski
var cores_polski := array{cores_laranja, cores_amarelo, cores_verde, cores_azul, cores_neutro};

//Povos do Leste
var cores_leste := array{cores_amarelo, cores_laranja, cores_rosa, cores_cinza};

//Gulthrak
var cores_gulthrak := array{cores_rosa, cores_vermelho, cores_neutro, cores_cinza};