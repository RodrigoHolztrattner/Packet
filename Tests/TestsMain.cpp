#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "..\Packet\Packet.h"
#include "MyResource.h"
#include "MyFactory.h"
#include "HelperMethods.h"
#include "HelperDefines.h"

/*
    - Instance pode ter um metodo que retorna um holder do seu recurso interno (incrementando a ref count?) que tenha 
    acesso tambem a propria instance, ao fazer isso a instance ficara com lock e apenas sera unlocked quando esse objeto
    for out of scope ou explicitamente chamarmos unlock nele.
    - O resource tambem deve respeitar o lock e unlock?
    - Esse objeto permite que funcoes do resource sejam chamadas usando o operador ->.
    - Ao finalizar as edicoes, o resource deve ser colocado para edicao no manager!?

    - Adicionar get resource e flags no resource test pra ver se as funcoes foram chamadas

    - Nao esquecer do external construct object!
*/