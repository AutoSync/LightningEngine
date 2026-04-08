# Expansao da Engine Via Plugins e Codigos de Expansao

Blueprint tecnico detalhado:

- Plan/Features/Blueprint-Bibliotecas-Nativas.md

Com a base solida o proximo passo e criar um sistema via codigo da engine para expandir naturamente elementos de UI e funcionabilidades dos sitema.
o metodo pode ser de dois tipos

- Expansao por Codigo
- Expansao por Plugin

## Expansao por Codigo

A expansao por codigo e a forma mais simples de expandir a engine, onde o usuario pode criar novas classes que herdam de classes base da engine e implementam novas funcionalidades. Por exemplo, o usuario pode criar uma nova classe de UI que herda de uma classe base de UI da engine e implementa novos metodos e propriedades. para ficar de forma mais efetiva o codigo é escrito em C++ e compilado junto com a engine, ou seja, o usuario precisa recompilar a engine para usar as novas funcionalidades. Essa forma de expansao e mais indicada para usuarios avancados que tem conhecimento em C++ e querem ter controle total sobre as novas funcionalidades.

Outra forma de expansao por codigo e criar scripts que interagem com a engine, onde o usuario pode criar scripts em linguagens de script como Lua ou Python que interagem com a engine e adicionam novas funcionalidades. Essa forma de expansao e mais indicada para usuarios que querem criar novas funcionalidades sem precisar recompilar a engine, mas tem conhecimento em linguagens de script.

O sistema de bibliotecas da engine devem oferecer as APIs de codigo necessarios para a implementação bem como as documentacoes necessarias

- API de UI para a Expansao da Engine e dos elementos visuais
  - Persistir em todo ecossistema sensivel ao Contexto
  - Adicionar elementos ao registro de Menu de Contexto, suportar submenu por categoria e subcategoria
  - Adicionar novos tipos de Arquivos comuns e proprietarios bem como thumbails, icones e ativos proprios bem como expandir a engine para outras ferramentas
- Suporte a HTTP
- Suporte a devinces por meios de portas USB ou Proprietarias

## Expansao por Plugin

A expansao por plugins podem ser tambem de dois tipos usando codigo C++ e Codigo Nativo da engine seja C# ou Ignite. O sistema de plugins deve ser projetado para ser flexivel e permitir que os usuarios criem plugins que adicionam novas funcionalidades a engine sem precisar recompilar a engine. Os plugins podem ser carregados dinamicamente em tempo de execucao, permitindo que os usuarios adicionem ou removam funcionalidades da engine sem precisar reiniciar a engine. Essa forma de expansao e mais indicada para usuarios que querem criar novas funcionalidades sem precisar recompilar a engine, mas tem conhecimento em C++ ou linguagens de script.

- Criar painel de plugins para os usuarios habilitarem ou desabilitarem os plugins divididos por categorias e subcategorias
- Criar lista de plugins habilitados por padrao em arquivo .yaml nas configurações da engine por padrao
- O Sistema deve poder exportar esses plugins para outros Projetos em forma de pacotes
  - leplugin package e unpackage instalar automaticamente na lista de plugins e iniciar quando o projeto for iniciado e expor as funcionalidades do plugin para o projeto
- Expor funcões para o Ignite e C# mesmo que eles tenham sido desenvolvidos por C++ para que possam ser usados em ambos os sistemas de plugins
- O Escopo de Plugins pode ser global ou de projeto
  - Plugins globais são carregados para todos os projetos e ficam disponiveis para serem usados em qualquer projeto
  - Plugins de projeto são carregados apenas para o projeto em que foram instalados e ficam disponiveis apenas para esse projeto
- O sistema de plugins deve ser projetado para ser seguro e evitar que plugins maliciosos possam afetar a estabilidade da engine ou comprometer a segurança do sistema. Os plugins devem ser isolados da engine, ou seja, um plugin nao deve afetar o funcionamento da engine ou de outros plugins. O sistema de plugins deve ter mecanismos de controle de acesso e permissões para garantir que os plugins nao possam acessar recursos sensiveis da engine ou do sistema sem autorizacao.
- O sistema de plugins deve ser projetado para ser compatível com diferentes plataformas e sistemas operacionais, permitindo que os usuarios criem plugins que funcionam em diferentes ambientes. Os plugins devem ser compilados para as plataformas de destino e a engine deve ser capaz de carregar os plugins corretamente em cada plataforma.
- O sistema de plugins deve ser projetado para ser fácil de usar e documentado, fornecendo guias e exemplos para os usuarios criarem seus próprios plugins. A engine deve fornecer uma API clara e consistente para os plugins interagirem com a engine e adicionarem novas funcionalidades. A documentação deve incluir exemplos de código, tutoriais e referências para ajudar os usuarios a entenderem como criar plugins eficazes.

### C++

O sistema de plugins em C++ deve permitir que os usuarios criem plugins que herdam de classes base da engine e implementam novas funcionalidades. Os plugins devem ser compilados como bibliotecas dinamicas (DLLs) e carregados pela engine em tempo de execucao. A engine deve fornecer uma API para os plugins interagirem com a engine e adicionarem novas funcionalidades. Os plugins devem ser isolados da engine, ou seja, um plugin nao deve afetar o funcionamento da engine ou de outros plugins.

### C# ou Ignite

O Sistema deve ser capas de executar em tempo real plugins criados em C# ao iniciar o Editor. Esses plugins devem ser compilados como assemblies e carregados pela engine em tempo de execucao. A engine deve fornecer uma API para os plugins interagirem com a engine e adicionarem novas funcionalidades. Os plugins devem ser isolados da engine, ou seja, um plugin nao deve afetar o funcionamento da engine ou de outros plugins. O sistema de plugins em C# ou Ignite deve ser projetado para ser fácil de usar e documentado, fornecendo guias e exemplos para os usuarios criarem seus próprios plugins. A engine deve fornecer uma API clara e consistente para os plugins interagirem com a engine e adicionarem novas funcionalidades. A documentação deve incluir exemplos de código, tutoriais e referências para ajudar os usuarios a entenderem como criar plugins eficazes.
