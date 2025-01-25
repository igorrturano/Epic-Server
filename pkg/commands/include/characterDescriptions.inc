use uo;
use os;
use file;

function GetSkinColorName(skincolor)
    skincolor := CInt(skincolor);
    case (skincolor)
        // Lista geral
        1003: return "bege clara";
        1006: return "marrom escura";
        1007: return "marrom escura";
        1008: return "marrom escura";
        1010: return "bege clara";
        1014: return "marrom escura";
        1015: return "marrom escura";
        1017: return "bege";
        1020: return "marrom";
        1021: return "marrom";
        1022: return "marrom";
        1024: return "pêssego clara";
        1027: return "marrom";
        1028: return "marrom";
        1029: return "marrom escura";
        1031: return "bege";
        1034: return "marrom clara";
        1035: return "marrom";
        1036: return "marrom escura";
        1038: return "pêssego";
        1041: return "marrom clara";
        1042: return "marrom clara";
        1043: return "marrom clara"; //tambem é cor de cabelo
        1049: return "marrom clara";
        1050: return "marrom escura"; //tambem é cor de cabelo

             // Elfos
        1002: return "clara rosada";
        1009: return "clara rosada";
        1016: return "clara rosada";
        1023: return "clara rosada";
        1030: return "clara";
        1037: return "bronzeada clara";
        1045: return "bronzeada clara";
        1004: return "bronzeada clara";
        1005: return "bronzeada avermelhada";
        1011: return "bronzeada avermelhada";
        1012: return "bronzeada";
        1018: return "bronzeada";
        1019: return "bronzeada";
        1025: return "bronzeada";
        1026: return "bronzeada";
        1032: return "bronzeada";
        1033: return "bronzeada amarronzada";
        1039: return "bronzeada amarronzada";
        1040: return "bronzeada amarronzada";
        1047: return "bronzeada amarronzada";
        1048: return "bronzeada amarronzada";

        // Amarelos
        1052: return "amarela"; //tambem é cor de cabelo
        1053: return "amarela";

        // Drows
        2406: return "cinza clara"; //tambem é cor de cabelo
        2306: return "cinza clara"; //tambem é cor de cabelo
        1898: return "cinza clara";
        1899: return "cinza Escura";
        1907: return "cinza Escura";
        1908: return "cinza Escura";
        1308: return "roxo claro";
        1309: return "roxa";
        1318: return "roxa";

        // Orcs
        1428: return "verde"; //tambem é cor de cabelo
        1430: return "verde"; //tambem é cor de cabelo
        1432: return "verde"; //tambem é cor de cabelo
        1434: return "verde"; //tambem é cor de cabelo
        1435: return "verde"; //tambem é cor de cabelo
        1437: return "verde"; //tambem é cor de cabelo
        1439: return "verde"; //tambem é cor de cabelo
        1441: return "verde"; //tambem é cor de cabelo
        1443: return "verde"; //tambem é cor de cabelo
        1444: return "verde"; //tambem é cor de cabelo
        1528: return "laranja"; //tambem é cor de cabelo
        1530: return "laranja"; //tambem é cor de cabelo
        1532: return "laranja"; //tambem é cor de cabelo
        1534: return "laranja"; //tambem é cor de cabelo
        1535: return "laranja"; //tambem é cor de cabelo
        1537: return "laranja"; //tambem é cor de cabelo
        1539: return "laranja"; //tambem é cor de cabelo
        1541: return "laranja"; //tambem é cor de cabelo
        1543: return "laranja"; //tambem é cor de cabelo
        1544: return "laranja"; //tambem é cor de cabelo
        1546: return "laranja"; //tambem é cor de cabelo
        1548: return "laranja"; //tambem é cor de cabelo
        1550: return "laranja"; //tambem é cor de cabelo
        1552: return "laranja"; //tambem é cor de cabelo
        1553: return "laranja"; //tambem é cor de cabelo
        1629: return "vermelha"; //tambem é cor de cabelo
        1631: return "vermelha"; //tambem é cor de cabelo
        1633: return "vermelha"; //tambem é cor de cabelo
        1634: return "vermelha"; //tambem é cor de cabelo
        1635: return "vermelha"; //tambem é cor de cabelo
        1638: return "vermelha"; //tambem é cor de cabelo
        1640: return "vermelha"; //tambem é cor de cabelo
        1642: return "vermelha"; //tambem é cor de cabelo
        1643: return "vermelha"; //tambem é cor de cabelo
        1644: return "vermelha"; //tambem é cor de cabelo

       default: return "Cor de Pele Desconhecida (" + skincolor + ")";
    endcase
endfunction


function GetHairColorName(haircolor)
    haircolor := CInt(haircolor);
    case (haircolor)
        992: return "grisalho/preto";
        2305: return "grisalho/preto";
        2412: return "grisalho/preto";
        2411: return "grisalho/preto";
        2410: return "grisalho/preto";
        2409: return "grisalho/preto";
        2402: return "cinza claro";
        2406: return "grisalho/preto";
        2404: return "grisalho/preto";
        2403: return "grisalho/preto";
        2306: return "grisalho/preto";
        2106: return "grisalho/preto";
        2105: return "grisalho/preto";
        1908: return "grisalho/preto";
        1754: return "castanho";
        1753: return "castanho";
        1752: return "castanho";
        1745: return "castanho";
        1743: return "castanho";
        1736: return "castanho";
        1727: return "castanho";
        1717: return "castanho";
        1708: return "castanho";
        1707: return "castanho";
        1517: return "castanho";
        1516: return "castanho";
        1509: return "castanho";
        1131: return "castanho";
        2009: return "dourado";
        2215: return "dourado";
        2010: return "dourado";
        1513: return "dourado";
        1713: return "dourado";
        1722: return "dourado";
        1740: return "dourado";
        1504: return "dourado";
        1514: return "dourado";
        1705: return "dourado";
        1714: return "dourado";
        1723: return "dourado";
        1741: return "dourado";
        1750: return "dourado";
        1121: return "dourado";
        1506: return "dourado";
        1515: return "dourado";
        1724: return "dourado";
        1733: return "dourado";
        1742: return "dourado";
        1751: return "dourado";
        1122: return "dourado";
        2007: return "loiro";
        2313: return "loiro";
        2213: return "loiro";
        1728: return "loiro";
        2214: return "loiro";
        1701: return "loiro";
        1501: return "loiro";
        1711: return "loiro";
        1719: return "loiro";
        1502: return "loiro";
        2216: return "loiro";
        1712: return "loiro";
        1512: return "loiro";
        1721: return "loiro";
        1748: return "loiro";
        1739: return "loiro";
        1860: return "ruivo";
        1869: return "ruivo";
        1524: return "ruivo";
        1533: return "ruivo";
        1542: return "ruivo";
        1861: return "ruivo";
        1870: return "ruivo";
        1525: return "ruivo";
        1526: return "ruivo";
        1554: return "ruivo";
        1863: return "ruivo";
        1871: return "ruivo";
        1527: return "ruivo";
        1144: return "ruivo";
        1872: return "ruivo";
        2107: return "castanho claro";
        1819: return "castanho claro";
        1846: return "castanho claro";
        1052: return "castanho claro";
        2308: return "castanho claro";
        2108: return "castanho claro";
        1820: return "castanho claro";
        1829: return "castanho claro";
        1838: return "castanho claro";
        2109: return "castanho claro";
        1811: return "castanho claro";
        1847: return "castanho claro";
        1812: return "castanho claro";
        1821: return "castanho claro";
        1839: return "castanho claro";
        1054: return "castanho claro";
        1831: return "castanho claro";
        1841: return "castanho claro";
        1814: return "castanho claro";
        1832: return "castanho claro";
        1805: return "castanho claro";
        1850: return "castanho claro";
        1056: return "castanho escuro";
        1114: return "castanho escuro";
        2017: return "castanho escuro";
        1815: return "castanho escuro";
        1851: return "castanho escuro";
        1842: return "castanho escuro";
        1833: return "castanho escuro";
        1057: return "castanho escuro";
        1043: return "castanho escuro";
        1050: return "castanho escuro";
        1834: return "castanho escuro";
        1816: return "castanho escuro";
        1843: return "castanho escuro";
        1852: return "castanho escuro";
        1853: return "castanho escuro";
        1807: return "castanho escuro";
        2112: return "castanho escuro";
        1844: return "castanho escuro";
        1818: return "castanho escuro";
        1836: return "castanho escuro";
        1854: return "castanho escuro";
        1845: return "castanho escuro";
        1051: return "castanho escuro";
        477: return "colorido";
        482: return "colorido";
        491: return "colorido";
        497: return "colorido";
        502: return "colorido";
        507: return "colorido";
        512: return "colorido";
        517: return "colorido";
        532: return "colorido";
        533: return "colorido";
        552: return "colorido";
        557: return "colorido";
        562: return "colorido";
        567: return "colorido";
        587: return "colorido";
        592: return "colorido";
        612: return "colorido";
        627: return "colorido";
        657: return "colorido";
        678: return "colorido";
        692: return "colorido";
        1337: return "colorido";
        96: return "azulado";
        106: return "azulado";
        500: return "azulado";
        501: return "azulado";
        2219: return "azulado";
        2220: return "azulado";
        2221: return "azulado";
        296: return "roxo";
        301: return "roxo";
        306: return "roxo";
        926: return "cinza";
        930: return "cinza";
        2391: return "cinza";
        2599: return "cinza";
        2766: return "cinza";
        2790: return "cinza";
        2886: return "cinza";
        1246: return "lilás";
        1247: return "lilás";
        1248: return "lilás";
        1361: return "violeta";
        1362: return "violeta";
        2101: return "branco";
        2442: return "branco";
        2522: return "branco";
        2888: return "branco";
        2102: return "branco acinzentado";
        2103: return "branco acinzentado";
        2720: return "branco roxo";
        2834: return "branco roxo";
        1139: return "marrom";
        1146: return "marrom";
        1147: return "marrom";
        1428: return "verde";
        1430: return "verde";
        1432: return "verde";
        1434: return "verde";
        1435: return "verde";
        1437: return "verde";
        1439: return "verde";
        1441: return "verde";
        1443: return "verde";
        1444: return "verde";
        1528: return "ruivo";
        1530: return "ruivo";
        1532: return "ruivo";
        1534: return "ruivo";
        1535: return "ruivo";
        1537: return "ruivo";
        1539: return "ruivo";
        1541: return "ruivo";
        1543: return "ruivo";
        1544: return "ruivo";
        1546: return "ruivo";
        1548: return "ruivo";
        1550: return "ruivo";
        1552: return "ruivo";
        1553: return "laranja";
        1629: return "vermelho";
        1631: return "vermelho";
        1633: return "vermelho";
        1634: return "vermelho";
        1635: return "vermelho";
        1638: return "vermelho";
        1640: return "vermelho";
        1642: return "vermelho";
        1643: return "vermelho";
        1644: return "vermelho";
        1044: return "castanho";
        1907: return "cinza";
        1899: return "cinza";
       2301: return "branco";
        2302: return "branco";
        2303: return "branco";
        2407: return "branco";
        2408: return "branco";
        1110: return "loiro";
        1112: return "loiro";
        1113: return "loiro";
        1120: return "loiro";
        2008: return "dourado";
        1102: return "cinza claro";
        1103: return "cinza claro";
        1104: return "cinza claro";
        1105: return "cinza claro";
        1106: return "cinza claro";
        1107: return "cinza claro";
        1108: return "cinza escuro";
        1109: return "cinza escuro";
        1111: return "castanho claro";
        1142: return "castanho claro";
        1143: return "castanho claro";
        1145: return "castanho claro";
        1115: return "castanho";
        1116: return "castanho escuro";
        1117: return "castanho escuro";
        1148: return "castanho escuro";
        1149: return "castanho escuro";
        1118: return "loiro claro";
        1119: return "loiro claro";
        1123: return "loiro";
        1124: return "loiro escuro";
        1125: return "loiro escuro";
        1126: return "dourado";
        1127: return "dourado";
        1128: return "dourado";
        1129: return "dourado";
        1130: return "dourado escuro";
        1132: return "dourado escuro";
        1133: return "dourado escuro";
        1134: return "ruivo claro";
        1135: return "ruivo claro";
        1136: return "ruivo claro";
        1137: return "ruivo claro";
        1138: return "ruivo";
        1140: return "ruivo escuro";
        1141: return "ruivo escuro";
    default: return "Cor de cabelo Desconhecida (" + haircolor + ")";
    endcase
endfunction


function GetMountType(mount)
    if (!mount)
        return "animal de montaria desconhecido";
    endif

    var mount_graphic := mount.graphic;
    var npctemplate := GetObjProperty(mount, "NPCTemplate");
    if (!npctemplate)
        npctemplate := mount.npctemplate;
    endif



    if (npctemplate)
        var template := NPC_ParseTemplateName(npctemplate);

        var cfg := ReadConfigFile(":brainai:npcdesc");
        if (cfg.errortext)

        else
            var elem := cfg[template.template];
            if (elem)
                var taxonomy := elem.Taxonomy;
                var category := elem.Category;

                
                if (category == "Mount")
                    case (taxonomy)
                        "equus": return "cavalo";
                        "camelus": return "lhama";
                        "struthio": return "avestruz";
                        "ursus": return "urso";
                        "felis": return "felino";
                        "canis": return "canino";
                        "draco": return "dragão";
                        "camelidae": return "lhama"; // Adicionado para corresponder ao exemplo da Lhama
                        default: return "animal de montaria (" + taxonomy + ")";
                    endcase
                else

                endif
            else

            endif
        endif
    else

    endif

    // Se não conseguir identificar pela categoria, usa o gráfico
    case (mount_graphic)
        0x3EA2: 0x3E9F: 0x3EA0: 0x3EA1: 0x3EA8: 0x3EA7: 0x3EB6: 0x3EB7: 0x3EA9: 0x3e9e: 0x3EAA: 0x3EAF: 0x3EB0: 0x3EB1: 0x3EB3: 0x3EB2: 0x3ECD:
            return "cavalo";
        0x3EA6: 0x3EAB:
            return "lhama";
        0x3EA3: 0x3EA4: 0x3EA5: 0x3EAC:
            return "avestruz";
        0x3ec5:
            return "urso polar";
        0x3EC7: 0x3EC8:
            return "tigre";
        0x3e95: 0x3EBC: 0x3e97:
            return "besouro gigante";
        0x3EBD: 0x3EBE: 0x3e98: 0x3ECE:
            return "dragão";
        0x3EB8: 0x3EBA: 0x3E9A:
            return "ridgeback";
        0x3EAD: 0x3E9C:
            return "kirin";
        0x3EB4: 0x3E9D:
            return "unicórnio";
        0x3e94:
            return "hai-riyo";
        0x3e90:
            return "raptalon";
        0x3e91:
            return "cu sidhe";
        0x3EC9:
            return "cão infernal";
        0x3e92:
            return "montaria de mondain";
        0x3EBB:
            return "montaria esquelética";
        0x3EC6:
            return "boura";
        0x3ECA:
            return "tarântula";
        0x3ECB:
            return "lasher";
        0x3ECC:
            return "windrunner";
        0x3ECF:
            return "pavão";
        0x3ED0:
            return "cão esquelético";
        0x3ED1:
            return "caranguejo-do-coco";
        0x3ED2:
            return "javali de gGuerra";
        0x3ED3:
            return "capivara";
        0x3ED4:
            return "terra-nova";
        0x3ED5:
            return "malamute";
        0x3ED6:
            return "dogue alemão";
        0x3ED7:
            return "são bernardo";
        0x3ED8:
            return "terrier russo";
        0x3ED9:
            return "rottweiler";
        default:
            return "animal de montaria desconhecido (Gráfico: " + Hex(mount_graphic) + ")";
    endcase
endfunction
function GetBeardStyleName(beard_objtype)
    case (beard_objtype)
        0x203E: return "barba longa";
        0x2042: return "barba longa";
        0xC671: return "barba longa";
        0x203F: return "barba curta";
        0x2043: return "barba curta";
        0xC672: return "barba curta";
        0x2040: return "cavanhaque";
        0xC670: return "cavanhaque";
        0x2041: return "bigode";
        0xC673: return "bigode";
        0x204B: return "barba curta com bigode";
        0xC6D8: return "barba curta com bigode";
        0x204C: return "barba longa com bigode";
        0xC675: return "barba longa com bigode";
        0x204D: return "barba vandyke";
        0xC677: return "barba vandyke";
        0xA1A8: return "barba lenhador";
        0xA1A9: return "bigode costeleta";
        0xC8FC: return "bigode costeleta";
        0xA1AA: return "barba trançada";
        0xC8FD: return "barba trançada";
        0xA1AB: return "costeletas";
        0xC8FE: return "costeletas";
        0xC97F: return "barba longa mal feita";
        0xC980: return "barba muito longa emaranhada";
        0xC981: return "barba longa encaracolada";
        0xC982: return "barba muito longa";
        0xC983: return "barba longa pontuda";
        0xC984: return "barba rabo de cavalo";
        0xC985: return "barba longa grande trança";
        0xC986: return "barba multi tranças";
        0xC987: return "barba trança dupla";
        0xC988: return "barba longa costeleta";
        0xC989: return "barba três rabos";
        0xC98A: return "barba longa cheia";
        0xC98B: return "barba longa desgrenhada";
        0xC98C: return "barba longa um rabo";
        0xC98D: return "barba longa duas tranças";
        0xC98E: return "barba longa dois rabos";
        0xC98F: return "barba longa bem cuidada";
        0xC990: return "barba longa bagunçada";
        0xC991: return "barba longa fina";
        0xC992: return "barba longa trabalhada";
        0xC993: return "barba longa no meio e tranças";
        0xC994: return "barba longa tranças laterais";
        0xC995: return "barba longa bigode costeleta";
        0xC996: return "barba longa aparada bigode costeleta";
        0xC997: return "barba curta ornamentada";
        0xC998: return "barba curta trança única";
        0xC999: return "barba curta dupla";
        0xC99A: return "bigode clássico";
        0xC99B: return "barba curta costeleta bigode";
        0xC99F: return "barba curta costeleta bigode";
        0xC99C: return "barba curta costeleta";
        0xC99D: return "barba curta grande costeleta";
        0xC99E: return "barba curta dupla trança";
        0xC9A0: return "barba curta trança única";
        0xC9A1: return "barba curta costeleta trança dupla";
        0xC9A2: return "barba curta costeleta bigode trança dupla";
        0xC9A3: return "barba longa trança dupla esvoaçada";
        0xC9A4: return "barba longa trança tripla esvoaçada";
        0xC9A5: return "barba longa dupla uma ponta";
        0xC9A6: return "barba longa trança quatro esvoaçante";
        0xC9A7: return "barba longa dupla uma ponta esvoaçante";
        0xC9A8: return "barba curta cheia";
        0xC9A9: return "barba longa dupla trança";
        0xC9AA: return "barba curta mal feita";
        0xC9AB: return "bigode longo";
        0xC9AC: return "bigode curto";
        0xCBA1: return "barba longa espetada";
        0xCBA2: return "barba longa emaranhada";
        0xCBA3: return "barba trançada enfeitada";
        0xCBA4: return "barba comprida";
        0xCBA5: return "barba penteada amarrada";
        0xCBA6: return "barba bigodões enfeitada";
        0xCBA7: return "barba real enfeitada";
        0xCBA8: return "barba múltiplas tranças enfeitadas";
        0xCBA9: return "barba trançada bipartida";
        0xCBAA: return "barba muito comprida";
        0xCBAB: return "barba trançada tripartida";
        0xCBAC: return "barba volumosa";
        0xCBAD: return "barba comprida bagunçada";
        0xCBAE: return "barba comprida atada";
        0xCBAF: return "barba múltiplas tranças";
        0xCBB0: return "barba volumosa bipartida";
        0xCBB1: return "barba longa cheia";
        0xCBB2: return "barba longa frizada";
        0xCBB3: return "barba longa pontuda";
        0xCBB4: return "barba enfeitada revoltosa";
        0xCBB5: return "barba volumosa com tranças";
        0xCBB6: return "barba com Pontas enfeitadas";
        0xCBB7: return "barba comprida aparada";
        0xCBB8: return "barba média aparada";
        0xCBBD: return "barba média aparada";
        0xCBB9: return "barba articulada";
        0xCBBA: return "barbicha trançada";
        0xCBBB: return "barbicha dupla";
        0xCBBC: return "bigode curvado";
        0xCBBE: return "barba média sem bigode";
        0xCBBF: return "barba bigode revoltoso";
        0xCBC0: return "barba trançada bipartida sobrancelhas";
        0xCBC1: return "barba média sobrancelhas";
        0xCBC2: return "barbicha trança longa enfeitada";
        0xCBC3: return "barbicha bipartida enfeitada";
        0xCBC4: return "barba média bipartida";
        0xCBC5: return "barba longa bipartida";
        0xCBC6: return "barba longa tripartida";
        0xCBC7: return "barba longa intrincada";
        0xCBC8: return "barba longa quatro pontas";
        0xCBC9: return "barba tripartida ondulada";
        0xCBCA: return "barba curta volumosa";
        0xCBCB: return "barba média tranças";
        0xCBCC: return "barba média revoltosa";
        default: return "estilo de barba desconhecido (0x" + Hex(beard_objtype) + ")";
    endcase
endfunction

function GetHairStyleName(hair_objtype)
    hair_objtype := CInt(hair_objtype);
    var style := "";

    if (hair_objtype == 0x203B)
        style := "cabelo curto";
    elseif (hair_objtype == 0x203C)
        style := "cabelos soltos";
    elseif (hair_objtype == 0x203D)
        style := "cabelos Presos";
    elseif (hair_objtype == 0x2034)
        style := "moicano";
    elseif (hair_objtype == 0x2037)
        style := "afro";
    elseif (hair_objtype == 0x2044)
        style := "rabo de cavalo";
    elseif (hair_objtype == 0x2045)
        style := "corte pageboy";
    elseif (hair_objtype == 0x2046)
        style := "coques";
    elseif (hair_objtype == 0x2047)
        style := "afro";
    elseif (hair_objtype == 0x2048)
        style := "cabelo ralo";
    elseif (hair_objtype == 0x2049)
        style := "cabelos trançados";
    elseif (hair_objtype == 0x204A)
        style := "topete";
    elseif (hair_objtype == 0x2FBF)
        style := "cabelo médio longo";
    elseif (hair_objtype == 0x2FC0)
        style := "cabelo com uma Pena";
    elseif (hair_objtype == 0x2FC1)
        style := "cabelo curto";
    elseif (hair_objtype == 0x2FC2)
        style := "mullet";
    elseif (hair_objtype == 0x2FCC)
        style := "cabelo com uma flor";
    elseif (hair_objtype == 0x2FCE)
        style := "nó grande";
    elseif (hair_objtype == 0x2FCF)
        style := "trança grande";
    elseif (hair_objtype == 0x2FD0)
        style := "coque grande";
    elseif (hair_objtype == 0x2FD1)
        style := "cabelo espetado";
    elseif (hair_objtype == 0xA1A4)
        style := "cabelo nagunçado";
    elseif (hair_objtype == 0xA1A6)
        style := "monge";
    elseif (hair_objtype == 0xA1A7)
        style := "topete";
    elseif (hair_objtype == 0xA1AC)
        style := "trança longa";
    elseif (hair_objtype == 0xA1AD)
        style := "dreadlocks";
    elseif (hair_objtype == 0xA1AE)
        style := "maria-chiquinha";
    elseif (hair_objtype == 0xA1AF)
        style := "coques laterais";
    elseif (hair_objtype == 0xA1B0)
        style := "franja";
    elseif (hair_objtype == 0xA1B1)
        style := "espetado";
    elseif (hair_objtype == 0xA1B2)
        style := "cabelo longo";
    elseif (hair_objtype == 0xA1B3)
        style := "cabelo longo";
    elseif (hair_objtype == 0xA1B4)
        style := "cabelo lateral";
    elseif (hair_objtype == 0xA1B5)
        style := "cabelo ondulado";
    elseif (hair_objtype == 0xA1B6)
        style := "tranças";
    elseif (hair_objtype == 0xA1B7)
        style := "cabelo enrolado no pescoço";
    elseif (hair_objtype == 0x2776)
        style := "cabelo longo de anão";
    elseif (hair_objtype == 0x266D)
        style := "cabelo longo de anão";
    elseif (hair_objtype == 0x383A)
        style := "moicano de anão";
    elseif (hair_objtype == 0x383B)
        style := "dois rabos de cavalo de anão";
    elseif (hair_objtype == 0x383C)
        style := "rabo de cavalo de anão";
    elseif (hair_objtype == 0x383D)
        style := "corte pageboy de anão";
    elseif (hair_objtype == 0x383E)
        style := "cabelo curto de anão";
    elseif (hair_objtype == 0xC60C)
        style := "cabelo curto (masculino)";
    elseif (hair_objtype == 0xC60D)
        style := "cabelo longo (masculino)";
    elseif (hair_objtype == 0xC60E)
        style := "rabo de cavalo (masculino)";
    elseif (hair_objtype == 0xC60F)
        style := "moicano (masculino)";
    elseif (hair_objtype == 0xC612)
        style := "tranças (masculino)";
    elseif (hair_objtype == 0xC616)
        style := "corte pageboy (masculino)";
    elseif (hair_objtype == 0xC618)
        style := "afro (masculino)";
    elseif (hair_objtype == 0xC61B)
        style := "topete (masculino)";
    elseif (hair_objtype == 0xED1C)
        style := "cabelo curto (feminino)";
    elseif (hair_objtype == 0xED1D)
        style := "cabelo longo (feminino)";
    elseif (hair_objtype == 0xED1E)
        style := "rabo de cavalo (feminino)";
    elseif (hair_objtype == 0xED1F)
        style := "moicano (feminino)";
    elseif (hair_objtype == 0xED23)
        style := "tranças (feminino)";
    elseif (hair_objtype == 0xED25)
        style := "afro (feminino)";
    elseif (hair_objtype == 0xED26)
        style := "corte pageboy (feminino)";
    elseif (hair_objtype == 0xED28)
        style := "coques (feminino)";
    elseif (hair_objtype == 0xED2B)
        style := "topete (feminino)";
    elseif (hair_objtype == 0xEDE5)
        style := "cabelo ralo";
    elseif (hair_objtype == 0xC6D4)
        style := "cabelo médio longo (masculino)";
    elseif (hair_objtype == 0xC6D5)
        style := "cabelo com pena longa (masculino)";
    elseif (hair_objtype == 0xC6D6)
        style := "cabelo curto (masculino)";
    elseif (hair_objtype == 0xC6D7)
        style := "mullet (masculino)";
    elseif (hair_objtype == 0xC6AA)
        style := "cabelo com flor (masculino)";
    elseif (hair_objtype == 0xC6AC)
        style := "nó grande (masculino)";
    elseif (hair_objtype == 0xC6AD)
        style := "trança grande (masculino)";
    elseif (hair_objtype == 0xC6AE)
        style := "coque grande (masculino)";
    elseif (hair_objtype == 0xC6AF)
        style := "cabelo espetado (masculino)";
    elseif (hair_objtype == 0xEE14)
        style := "cabelo médio longo (feminino)";
    elseif (hair_objtype == 0xEE15)
        style := "cabelo com Pena longa (feminino)";
    elseif (hair_objtype == 0xEE16)
        style := "cabelo curto (feminino)";
    elseif (hair_objtype == 0xEE17)
        style := "mullet (feminino)";
    elseif (hair_objtype == 0xEDEA)
        style := "cabelo com flor (feminino)";
    elseif (hair_objtype == 0xEDEC)
        style := "nó grande (feminino)";
    elseif (hair_objtype == 0xEDED)
        style := "trança grande (feminino)";
    elseif (hair_objtype == 0xEDEE)
        style := "coque grande (feminino)";
    elseif (hair_objtype == 0xEDEF)
        style := "cabelo espetado (feminino)";
    elseif (hair_objtype == 0xC907)
        style := "cabelo bagunçado";
    elseif (hair_objtype == 0xC908)
        style := "afro ";
    elseif (hair_objtype == 0xC909)
        style := "monge";
    elseif (hair_objtype == 0xC90A)
        style := "topete ";
    elseif (hair_objtype == 0xC90F)
        style := "trança longa (masculino)";
    elseif (hair_objtype == 0xC910)
        style := "dreadlocks (masculino)";
    elseif (hair_objtype == 0xC911)
        style := "maria-chiquinha (masculino)";
    elseif (hair_objtype == 0xC912)
        style := "coques laterais (masculino)";
    elseif (hair_objtype == 0xC913)
        style := "franja";
    elseif (hair_objtype == 0xC914)
        style := "espetado ";
    elseif (hair_objtype == 0xC915)
        style := "cabelo longo";
    elseif (hair_objtype == 0xC916)
        style := "cabelo longo";
    elseif (hair_objtype == 0xC917)
        style := "cabelo lateral (masculino)";
    elseif (hair_objtype == 0xC918)
        style := "cabelo ondulado (masculino)";
    elseif (hair_objtype == 0xC919)
        style := "tranças (masculino)";
    elseif (hair_objtype == 0xC91A)
        style := "cabelo enrolado no pescoço (masculino)";
    elseif (hair_objtype == 0xF01F)
        style := "trança longa (feminino)";
    elseif (hair_objtype == 0xF020)
        style := "dreadlocks (feminino)";
    elseif (hair_objtype == 0xF021)
        style := "maria-chiquinha (feminino)";
    elseif (hair_objtype == 0xF022)
        style := "coques laterais (feminino)";
    elseif (hair_objtype == 0xF027)
        style := "cabelo lateral (feminino)";
    elseif (hair_objtype == 0xF028)
        style := "cabelo ondulado (feminino)";
    elseif (hair_objtype == 0xF029)
        style := "tranças (feminino)";
    elseif (hair_objtype == 0xF02A)
        style := "cabelo enrolado no pescoço (feminino)";
    else
        style := "estilo de cabelo desconhecido (0x" + Hex(hair_objtype) + ")";
    endif

    return style;
endfunction

