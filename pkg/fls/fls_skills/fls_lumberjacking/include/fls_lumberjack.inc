
function GetWoodType(tree)
    case (tree.objtype)
       0xcd6: 0xcd8:
              return UOBJ_PINE_LOG; //pinheiro
       0xcda: 0xcdd:
              return UOBJ_OAK_LOG; //carvalho
       0xce0: 0xce3:
              return UOBJ_WALNUT_LOG; //nogueira
       0xce6:
              return UOBJ_WILLOW_LOG; //salgueiro
       0xcf8: 0xcfb: 0xcfe: 0xd01:
              return UOBJ_CYPRESS_LOG; //cipreste 
   	0x247d:
              return 0xee3d; //polar
       0x12b9:
              return 0xee3e; //teixo
       0xa5ce:
              return 0xee3f; //Imbuia
       0x309c:
              return 0xee40; //mogno
       0x222E: 0x222F: 0x2230: 0x2231:
              return 0xee3c; //cogumelo
       0x30bd:
              return 0xee3b; //ebano
       default: 
              return UOBJ_DEFAULT_LOG;
    endcase
endfunction
