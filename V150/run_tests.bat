@echo off
setlocal

set EXE=x64\Debug\LightningEngine.exe

if not exist "%EXE%" (
  echo [INFO] Executavel nao encontrado, compilando Debug x64...
  "D:\Arquivos\HDDR\Microsoft\VisualStudio\VS26\MSBuild\Current\Bin\MSBuild.exe" LightningEngine.sln /p:Configuration=Debug /p:Platform=x64 /m /nologo
  if errorlevel 1 (
    echo [ERROR] Falha na compilacao.
    exit /b 1
  )
)

echo [INFO] Rodando testes automatizados...
"%EXE%" --run-tests
exit /b %ERRORLEVEL%
