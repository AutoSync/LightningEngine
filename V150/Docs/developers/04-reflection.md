# 4. Reflexão (TypeRegistry)

A Lightning Engine **não** tenta replicar a reflexão do Unreal. O custo de
metaprogramação pesada (UHT) seria desproporcional. Em vez disso, usamos
**registro manual** com macros leves.

API canônica (skeleton em
[`src/include/TypeRegistry.h`](../../src/include/TypeRegistry.h)):

```cpp
struct TypeInfo {
    std::string                name;
    std::size_t                size;
    std::vector<FieldInfo>     fields;
    std::function<void*()>     create;   // factory dinâmica
    std::function<void(void*)> destroy;
};

class TypeRegistry {
public:
    static TypeRegistry& Get();
    template<typename T> void Register(const std::string& name);
    const TypeInfo* Find(const std::string& name) const;
    std::vector<std::string> ListNames() const;
};
```

## 4.1 Para que serve

- **Serialização** (JSON/YAML de cenas, prefabs, config).
- **Editor (Next.js/Tauri)** — Inspector lê `fields` e gera UI automática.
- **Bindings Rust/JS** — geração de wrappers.
- **Instanciação dinâmica** — spawnar componente pelo nome.

Não use reflexão em hot-loop. Cacheie o `TypeInfo*` no startup.

## 4.2 Padrão de registro

```cpp
struct Transform {
    float x = 0.f, y = 0.f, z = 0.f;
};

LE_REGISTER_TYPE(Transform)
    .Field("x", &Transform::x)
    .Field("y", &Transform::y)
    .Field("z", &Transform::z);
```

`LE_REGISTER_TYPE` é uma macro fina que retorna um builder. Não há lógica
no macro além de criar `TypeInfo` e empurrar para o registry.

## 4.3 Quem registra o quê

| Tipo                    | Quem registra                                |
| ----------------------- | -------------------------------------------- |
| Tipos primitivos da engine | Core (no boot)                            |
| Componentes oficiais    | Plugin oficial em `OnRegister()`             |
| Componentes de gameplay | Plugin do projeto em `OnRegister()`          |
| Tipos de script (Nucleo)| Runtime de scripting traduz da reflexão dele |

## 4.4 Exposição para o editor

`PluginManager` agrega o `TypeRegistry` no boot. O bridge Rust serializa em:

```json
{
  "components": [
    {
      "name": "Transform",
      "fields": [
        { "name": "x", "type": "float" },
        { "name": "y", "type": "float" },
        { "name": "z", "type": "float" }
      ]
    }
  ]
}
```

Esse JSON é exatamente o que o Inspector da UI consome. Ver
[`docs/developers/07-rust-tauri-bridge.md`](07-rust-tauri-bridge.md).

## 4.5 Anti-padrões

- Refletir tipos com layout não-trivial (vtables, herança virtual) — use
  apenas POD-like ou wrappers explícitos.
- Mudar o nome refletido depois de publicado — quebra serialização.
  Adicione `oldNames` se precisar renomear.
- Usar reflexão para resolver dispatch que poderia ser virtual normal.
