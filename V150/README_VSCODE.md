# Configuração do VS Code para Lightning Engine (C++)

## Arquivos de Configuração Criados

### 1. `tasks.json` - Tarefas de Build
- **Build (Debug x64)**: Compila a solução com MSBuild (Debug, x64)
- **Build (Release x64)**: Compila a solução com MSBuild (Release, x64)
- **Clean**: Limpa os arquivos de build
- **Run (Debug)**: Executa o executável após compilar

**Como usar:**
1. Abra a paleta de comandos (Ctrl+Shift+P)
2. Digite "Tasks: Run Task"
3. Selecione a tarefa desejada

### 2. `launch.json` - Configuração de Debug
- **Launch LightningEngine**: Debug do executável Debug
- **Launch LightningEngine (Release)**: Debug do executável Release

**Como usar:**
1. Vá para a aba "Run and Debug" (Ctrl+Shift+D)
2. Selecione a configuração desejada
3. Clique no botão verde "Start Debugging" (F5)

### 3. `c_cpp_properties.json` - Configuração do IntelliSense
- Define caminhos de include para SDL3 e Lightning Engine
- Configura padrões C++20 e C17
- Configurações para compilador GCC (MinGW)

## Instruções de Uso

### Pré-requisitos
1. **Visual Studio Build Tools** instalado (MSBuild)
2. **SDL3** instalado no sistema
3. **Extensão C/C++** do VS Code instalada

### Passos para Configurar
1. **Coloque os arquivos na pasta `.vscode`** (crie se não existir):
   ```
   LightningEngine/V150/
   ├── .vscode/
   │   ├── tasks.json      (copie de tasks.json)
   │   ├── launch.json     (copie de launch.json)
   │   └── c_cpp_properties.json (copie de c_cpp_properties.json)
   └── ...
   ```

2. **Ajuste os caminhos** se necessário:
   - Em `c_cpp_properties.json`: ajuste `compilerPath` e `includePath`
   - Em `launch.json`: verifique o caminho do executável

3. **Compile pela primeira vez:**
   - Abra a paleta de comandos (Ctrl+Shift+P)
   - Execute "Tasks: Run Task" → "Build (Debug x64)"

4. **Debug:**
   - Pressione F5 para iniciar debug
   - Ou use a aba "Run and Debug"

## Atalhos Úteis
- **Ctrl+Shift+B**: Executa a tarefa de build padrão
- **F5**: Inicia debug
- **Ctrl+F5**: Executa sem debug
- **Ctrl+Shift+P**: Abre paleta de comandos

## Solução de Problemas

### Erro "MSBuild não encontrado"
- Instale Visual Studio Build Tools
- Ou adicione MSBuild ao PATH

### Erro "SDL3 não encontrado"
- Instale SDL3 via vcpkg ou manualmente
- Atualize os caminhos em `c_cpp_properties.json`

### Erro de compilação
- Verifique se a solução `.sln` existe
- Execute `msbuild LightningEngine.sln` manualmente no terminal

## Personalização

### Para usar MinGW/GCC em vez de MSBuild:
1. Modifique `tasks.json`:
   - Troque `"command": "msbuild"` por `"command": "g++"`
   - Ajuste os argumentos para compilação GCC

### Para outras configurações:
- Consulte a documentação da extensão C/C++
- Ajuste os padrões C++ conforme necessário

---

**Nota:** O diretório `.vscode` está bloqueado neste workspace. Copie os arquivos manualmente para a pasta `.vscode` do seu projeto.