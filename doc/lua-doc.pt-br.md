# Tópicos

1. [Funções Lua](#funções-lua)
2. [Constantes Lua](#constantes-lua)
3. [Comportamentos de LED](#comportamentos-de-led)
4. [Interface Bluetooth](#interface-bluetooth)

# Funções Lua

- [Energia](#energia)
- [Sistema](#sistema)
- [Sensores](#sensores)
- [Desenho no Painel de LED](#desenho-no-painel-de-led)
- [Funções de Dicionário](#funções-de-dicionário)
- [Controle de Servo](#controle-de-servo)
- [Decodificação de Imagem](#decodificação-de-imagem)
- [Tela Interna](#tela-interna)
- [Fitas de LED](#fitas-de-led-1)
- [Núcleo Arduino](#núcleo-arduino)
- [Controle Remoto IR](#controle-remoto-ir)
- [Modelos 2D](#modelos-2d)
- [Animações por Keyframe](#animações-por-keyframe)

## Energia

[↑ Voltar ao topo](#tópicos)

#### `panelPowerOn()`
Liga o regulador, alimentando o painel e a saída de 5V.
> **⚠️ Requer `PIN_ENABLE_REGULATOR`**: Se este pino não estiver definido na configuração de hardware, esta função não faz nada.
- **Retorna**: `nil`

#### `panelPowerOff()`
Desliga o regulador.
> **⚠️ Requer `PIN_ENABLE_REGULATOR`**: Se este pino não estiver definido na configuração de hardware, esta função não faz nada.
- **Retorna**: `nil`

#### `getBatteryVoltage()`
Retorna a tensão da bateria/USB em volts.
- **Retorna**: `float`

#### `getAvgBatteryVoltage()`
Retorna a tensão média da bateria/USB em volts.
- **Retorna**: `float`

#### `setAutoCheckPowerLevel(bool)`
Define a verificação automática de energia. Se a energia ficar abaixo do `setVoltageStopThreshold` e o hardware estiver configurado usando um conversor buck, a tensão é cortada.
- **Retorna**: `nil`

#### `setVoltageStopThreshold(voltage)`
Define o limite de tensão no qual o sistema desligará automaticamente para evitar danos por baixa tensão da bateria.
- **Parâmetros**:
  - `voltage` (int): O limite de tensão em volts.
- **Retorna**: `nil`

#### `setVoltageStartThreshold(voltage)`
Define o limite de tensão no qual o sistema ligará automaticamente após ser desligado por baixa tensão da bateria.
- **Parâmetros**:
  - `voltage` (int): O limite de tensão em volts.
- **Retorna**: `nil`

#### `setPoweringMode(mode)`
Define o modo de alimentação do sistema. O modo pode ser `POWER_MODE_NONE`, `POWER_MODE_5V_PD`, `POWER_MODE_USB_9V` ou `POWER_MODE_BATTERY`.
- **Parâmetros**:
  - `mode` (int): O modo de alimentação a ser definido.
- **Retorna**: `nil`

## Sistema

[↑ Voltar ao topo](#tópicos)

#### `log(msg)`
Registra uma mensagem no sistema de log (aparece na saída serial e no arquivo de log).
- **Parâmetros**:
  - `msg` (string): A mensagem a ser registrada.
- **Retorna**: `nil`

#### `setHaltOnError(halt)`
Controla se o sistema trava (congela com exibição de erro) quando ocorre um erro Lua. Útil para depuração; desative em produção para recuperação sem travar.
- **Parâmetros**:
  - `halt` (bool): `true` para travar no erro (padrão), `false` para continuar.
- **Retorna**: `nil`

### `getFreePsram()`
Retorna a quantidade de PSRAM (Pseudo Static RAM) livre em bytes.
- **Retorna**: `int` (A quantidade de PSRAM livre em bytes).

### `getFreeHeap()`
Retorna a quantidade de memória heap livre em bytes.
- **Retorna**: `int` (A quantidade de memória heap livre em bytes).

#### `getLuaFps()`
Retorna os quadros por segundo atuais medidos no loop Lua (Core 1).
- **Retorna**: `float`

#### `getFps()`
Retorna os FPS medidos automaticamente pelo sistema.
- **Retorna**: `float`

#### `restart()`
Reinicia o microcontrolador ESP32.
- **Retorna**: `nil`

#### `getResetReason()`
Retorna o motivo do último reinício.
- **Retorna**: `int` (Uma das constantes ESP_RST_*).

#### `setBrownoutDetection(bool enable)`
Habilita ou desabilita a detecção de brownout.
- **Parâmetros**:
  - `enable` (bool): `true` para habilitar, `false` para desabilitar.
- **Retorna**: `nil`

#### `listFiles(path, recursive)`
Lista arquivos em um diretório no cartão SD.
- **Parâmetros**:
  - `path` (string): Caminho do diretório.
  - `recursive` (bool): Se deve listar recursivamente.
- **Retorna**: `array de string`

#### `listFilesInFolder(path)`
Lista arquivos em uma pasta usando o auxiliar de armazenamento interno.
- **Parâmetros**:
  - `path` (string): Caminho do diretório.
- **Retorna**: `array de string`

#### `moveFile(src, dest)`
Move/renomeia um arquivo no cartão SD.
- **Parâmetros**:
  - `src` (string): Caminho de origem.
  - `dest` (string): Caminho de destino.
- **Retorna**: `bool` (Sucesso).

#### `removeFile(path)`
Exclui um arquivo do cartão SD.
- **Parâmetros**:
  - `path` (string): Caminho do arquivo.
- **Retorna**: `bool` (Sucesso).

#### `createDir(path)`
Cria um diretório no cartão SD.
- **Parâmetros**:
  - `path` (string): Caminho do diretório a ser criado.
- **Retorna**: `bool` (Sucesso).

#### `fileExists(path)`
Verifica se um arquivo ou diretório existe no cartão SD.
- **Parâmetros**:
  - `path` (string): Caminho do arquivo.
- **Retorna**: `bool`

#### `formatFFAT(full)`
Formata o sistema de arquivos FFat interno (flash).
- **Parâmetros**:
  - `full` (bool): Se `true`, realiza uma formatação completa.
- **Retorna**: `bool` (Sucesso).

#### `deleteBulkFile()`
Exclui o arquivo `frames.bulk`, forçando uma reconstrução na próxima inicialização.
- **Retorna**: `nil`

#### `composeBulkFile()`
Dispara manualmente a reconstrução do arquivo `frames.bulk` a partir dos frames PNG no cartão SD.
- **Retorna**: `nil`

#### `dumpStackToSerial()`
Utilitário de depuração. Imprime o conteúdo atual da pilha Lua na porta serial.
- **Retorna**: `nil`

## Sensores

[↑ Voltar ao topo](#tópicos)

#### `readButtonStatus(int)`
Lê o status do botão do controle remoto. Há um total de 5 botões. Os estados possíveis são:
```
BUTTON_RELEASED = 0
BUTTON_JUST_PRESSED = 1
BUTTON_PRESSED = 2
BUTTON_JUST_RELEASED = 3
```
- **Parâmetros**:
  - `button` (int): O ID do botão a ser verificado.
- **Retorna**: `int` (O estado do botão).

#### `readAccelerometerX([device])`
Retorna o valor bruto do sensor acelerômetro do controle remoto, apenas no eixo X.
- **Parâmetros**:
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.
- **Retorna**: `float` (O valor bruto do acelerômetro no eixo X).

#### `readAccelerometerY([device])`
Retorna o valor bruto do sensor acelerômetro do controle remoto, apenas no eixo Y.
- **Parâmetros**:
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.
- **Retorna**: `float` (O valor bruto do acelerômetro no eixo Y).

#### `readAccelerometerZ([device])`
Retorna o valor bruto do sensor acelerômetro do controle remoto, apenas no eixo Z.
- **Parâmetros**:
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.
- **Retorna**: `float` (O valor bruto do acelerômetro no eixo Z).

#### `readGyroX([device])`
Retorna o valor bruto do sensor giroscópio do controle remoto, apenas no eixo X.
- **Parâmetros**:
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.
- **Retorna**: `int` (O valor bruto do giroscópio no eixo X).

#### `readGyroY([device])`
Retorna o valor bruto do sensor giroscópio do controle remoto, apenas no eixo Y.
- **Parâmetros**:
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.
- **Retorna**: `int` (O valor bruto do giroscópio no eixo Y).

#### `readGyroZ([device])`
Retorna o valor bruto do sensor giroscópio do controle remoto, apenas no eixo Z.
- **Parâmetros**:
  - `device` (int, opcional): O ID do dispositivo de controle remoto. Padrão é `0`.
- **Retorna**: `int` (O valor bruto do giroscópio no eixo Z).

#### `hasLidar()`
Verifica a presença de um lidar.
- **Retorna**: `bool` (`true` se um lidar estiver presente, caso contrário `false`).

#### `readLidar()`
Retorna a distância em mm do lidar.
- **Retorna**: `int` (A distância em milímetros).

#### `getInternalButtonStatus()`
Retorna o status do botão interno.
- **Retorna**: `int` (1 para pressionado e 0 para solto).

#### `i2cScan()`
Escaneia o barramento I2C e retorna os endereços dos dispositivos detectados.
- **Retorna**: `array de int` (Lista de endereços I2C detectados).

## Desenho no Painel de LED

[↑ Voltar ao topo](#tópicos)

#### `flipPanelBuffer()`
Os painéis têm dois buffers. Um para desenho e outro para renderização. Quando o desenho é concluído em um buffer, esta chamada os alterna para que você possa desenhar no outro.
- **Retorna**: `nil`

#### `drawPanelPixel(x, y, color)`
Desenha um pixel nas coordenadas especificadas com a cor fornecida.
- **Parâmetros**:
  - `x` (int): A coordenada X.
  - `y` (int): A coordenada Y.
  - `color` (int): A cor para desenhar (use `color565` ou `color444` para gerar a cor).
- **Retorna**: `nil`

#### `drawPanelPixels(x, y, pixels)`
Desenha múltiplos pixels de uma vez a partir das coordenadas fornecidas.
- **Parâmetros**:
  - `x` (int): Coordenada X inicial.
  - `y` (int): Coordenada Y inicial.
  - `pixels` (array de int): Array de valores de cor no formato `color565`.
- **Retorna**: `nil`

#### `drawPanelCircle(x, y, radius, color)`
Desenha o contorno de um círculo nas coordenadas especificadas.
- **Parâmetros**:
  - `x` (int): A coordenada X do centro.
  - `y` (int): A coordenada Y do centro.
  - `radius` (int): O raio do círculo.
  - `color` (int): A cor para desenhar.
- **Retorna**: `nil`

#### `drawPanelChar(x, y, c, color, [bg, [size]])`
Desenha um caractere no painel nas coordenadas especificadas.
- **Parâmetros**:
  - `x` (int): A coordenada X.
  - `y` (int): A coordenada Y.
  - `c` (char): O caractere a ser desenhado.
  - `color` (int): A cor do caractere.
  - `bg` (int, opcional): A cor de fundo. Padrão é `0`.
  - `size` (int, opcional): Multiplicador de tamanho. Padrão é `1`.
- **Retorna**: `nil`

#### `drawPanelFillCircle(x, y, radius, color)`
Desenha um círculo preenchido nas coordenadas especificadas.
- **Parâmetros**:
  - `x` (int): A coordenada X do centro.
  - `y` (int): A coordenada Y do centro.
  - `radius` (int): O raio do círculo.
  - `color` (int): A cor de preenchimento.
- **Retorna**: `nil`

#### `drawPanelLine(x0, y0, x1, y1, color)`
Desenha uma linha entre dois pontos.
- **Parâmetros**:
  - `x0` (int): Coordenada X do ponto inicial.
  - `y0` (int): Coordenada Y do ponto inicial.
  - `x1` (int): Coordenada X do ponto final.
  - `y1` (int): Coordenada Y do ponto final.
  - `color` (int): A cor para desenhar.
- **Retorna**: `nil`

#### `drawPanelRect(x, y, width, height, color)`
Desenha o contorno de um retângulo.
- **Parâmetros**:
  - `x` (int): A coordenada X do canto superior esquerdo.
  - `y` (int): A coordenada Y do canto superior esquerdo.
  - `width` (int): A largura do retângulo.
  - `height` (int): A altura do retângulo.
  - `color` (int): A cor para desenhar.
- **Retorna**: `nil`

#### `drawPanelFillRect(x, y, width, height, color)`
Desenha um retângulo preenchido.
- **Parâmetros**:
  - `x` (int): A coordenada X do canto superior esquerdo.
  - `y` (int): A coordenada Y do canto superior esquerdo.
  - `width` (int): A largura do retângulo.
  - `height` (int): A altura do retângulo.
  - `color` (int): A cor de preenchimento.
- **Retorna**: `nil`

#### `drawPanelFillTriangle(x0, y0, x1, y1, x2, y2, color)`
Desenha um triângulo preenchido definido por três vértices.
- **Parâmetros**:
  - `x0, y0` (int): Primeiro vértice.
  - `x1, y1` (int): Segundo vértice.
  - `x2, y2` (int): Terceiro vértice.
  - `color` (int): A cor de preenchimento.
- **Retorna**: `nil`

#### `color444(r, g, b)`
Converte valores RGB888 para o formato RGB444.
- **Parâmetros**:
  - `r` (int): Vermelho (0-255).
  - `g` (int): Verde (0-255).
  - `b` (int): Azul (0-255).
- **Retorna**: `int` (Valor de cor RGB444).

#### `color565(r, g, b)`
Converte valores RGB888 para o formato RGB565.
- **Parâmetros**:
  - `r` (int): Vermelho (0-255).
  - `g` (int): Verde (0-255).
  - `b` (int): Azul (0-255).
- **Retorna**: `int` (Valor de cor RGB565).

#### `clearPanelBuffer()`
Limpa o buffer da tela.
- **Retorna**: `nil`

#### `drawPanelFace(faceId)`
Desenha um rosto (frame) específico. Este rosto já deve ter sido carregado anteriormente.
- **Parâmetros**:
  - `faceId` (int): O ID do rosto a ser desenhado.
- **Retorna**: `nil`

#### `setPanelAnimation(frames, duration, [repeat, [drop, [storage]]])`
Se o painel estiver no modo gerenciado, define uma animação para rodar nele.
- **Parâmetros**:
  - `frames` (array de int): Os IDs de cada frame na animação.
  - `duration` (int): A duração de cada frame em milissegundos.
  - `repeat` (int, opcional): Número de repetições. Padrão é `-1` (infinito).
  - `drop` (bool, opcional): Se `true`, todas as animações empilhadas são apagadas e esta passa a ser a única.
  - `storage` (int, opcional): Quando definido, pode ser consultado via `getCurrentAnimationStorage()`. Útil quando animações compartilham os mesmos frames.
- **Retorna**: `nil`

#### `setPanelModelAnimation(modelAnimId, [repeat, [drop]])`
Se o painel estiver no modo gerenciado, define uma animação de modelo 2D para rodar nele.
- **Parâmetros**:
  - `modelAnimId` (int): O ID da animação de modelo a ser reproduzida.
  - `repeat` (int, opcional): Número de repetições. Padrão é `-1` (infinito).
  - `drop` (bool, opcional): Se `true`, limpa a pilha de animações primeiro.
- **Retorna**: `nil`

#### `setInterruptFrames(frames, frameDuration)`
Define frames que são reproduzidos como animação de interrupção (ex: acionado por um sensor).
- **Parâmetros**:
  - `frames` (array de int): IDs dos frames a exibir.
  - `frameDuration` (int): Duração de cada frame em milissegundos.
- **Retorna**: `nil`

#### `setInterruptAnimationPin(pin)`
Configura o pino GPIO que aciona a animação de interrupção.
- **Parâmetros**:
  - `pin` (int): O número do pino GPIO.
- **Retorna**: `nil`

#### `setAnimationShader(shader, [intensity])`
Define um shader a ser aplicado sobre a animação do painel.
- **Parâmetros**:
  - `shader` (int): Tipo de shader (ex: `SHADER_RAINBOW`, `SHADER_FIRE`).
  - `intensity` (float, opcional): Intensidade do shader. Padrão é `1.0`.
- **Retorna**: `nil`

#### `setPanelColorMode(mode)`
Define a ordem dos canais de cor do painel (útil se seu painel tem fiação não padrão).
- **Parâmetros**:
  - `mode` (int): Um dos valores `COLOR_MODE_RGB`, `COLOR_MODE_RBG`, `COLOR_MODE_GRB`, `COLOR_MODE_GBR`, `COLOR_MODE_BRG`, `COLOR_MODE_BGR`.
- **Retorna**: `nil`

#### `loadFrameAsTexture(frameId)`
Carrega um frame do painel no slot de textura para uso pelo renderizador de modelos 2D.
- **Parâmetros**:
  - `frameId` (int): O ID do frame a carregar como textura.
- **Retorna**: `nil`

### `getCurrentAnimationStorage()`
Retorna o ID do armazenamento de animação atual sendo usado.
- **Retorna**: `int`

#### `setPanelManaged(managed)`
Habilita ou desabilita o modo gerenciado. No modo gerenciado, a renderização é tratada assincronamente pelo Core 0.
- **Parâmetros**:
  - `managed` (bool): `true` para habilitar, `false` para desabilitar.
- **Retorna**: `nil`

#### `isPanelManaged()`
Retorna se o painel está no modo gerenciado.
- **Retorna**: `bool`

#### `getPanelCurrentFace()`
Retorna o ID do frame atual sendo exibido.
- **Retorna**: `int`

#### `drawPanelCurrentFrame()`
Desenha o frame atual imediatamente.
- **Retorna**: `nil`

#### `getAnimationStackSize()`
Retorna o número de animações atualmente empilhadas.
- **Retorna**: `int`

#### `popPanelAnimation()`
Remove a animação atual da pilha.
- **Retorna**: `nil`

#### `setPanelBrightness(brightness)`
Define o brilho do painel imediatamente.
- **Parâmetros**:
  - `brightness` (int): Nível de brilho (0-255, onde 255 é 100%).
- **Retorna**: `nil`

#### `getPanelBrightness()`
Retorna o nível de brilho atual do painel.
- **Retorna**: `int` (0-255)

#### `gentlySetPanelBrightness(brightness, [rate])`
Ajusta gradualmente o brilho do painel para o nível especificado.
- **Parâmetros**:
  - `brightness` (int): Nível de brilho alvo (0-255).
  - `rate` (int, opcional): Velocidade da transição. Padrão é `4`.
- **Retorna**: `nil`

#### `setRainbowShader(enabled)`
Habilita ou desabilita o shader de arco-íris, que converte pixels para um padrão de arco-íris.
- **Parâmetros**:
  - `enabled` (bool): `true` para habilitar, `false` para desabilitar.
- **Retorna**: `nil`

#### `getFrameOffsetByName(name)`
Retorna o offset do frame associado ao nome fornecido.
- **Parâmetros**:
  - `name` (string): O nome do frame.
- **Retorna**: `int`

#### `getFrameCountByName(name)`
Retorna a quantidade de frames que um grupo de frames possui.
- **Parâmetros**:
  - `name` (string): O nome do frame.
- **Retorna**: `int`

## Decodificação de Imagem

[↑ Voltar ao topo](#tópicos)

### `decodePng(filename)`
Decodifica um arquivo PNG do cartão SD e retorna os dados brutos dos pixels no formato RGB565.
- **Parâmetros**:
  - `filename` (string): O caminho para o arquivo PNG no cartão SD.
- **Retorna**: `tabela uint16`

## Funções de Dicionário

[↑ Voltar ao topo](#tópicos)

#### `dictGet(key)`
Obtém um valor do armazenamento persistente de dicionário.
- **Parâmetros**:
  - `key` (string): A chave para procurar.
- **Retorna**: `string` (O valor armazenado).

#### `dictSet(key, value)`
Define um valor no armazenamento persistente de dicionário.
- **Parâmetros**:
  - `key` (string): A chave para armazenar.
  - `value` (string): O valor para armazenar.
- **Retorna**: `nil`

#### `dictDet(key)`
Exclui uma chave do armazenamento persistente de dicionário.
> **Nota**: O nome registrado da função é `dictDet` (não `dictDel`).
- **Parâmetros**:
  - `key` (string): A chave para excluir.
- **Retorna**: `nil`

#### `dictSave()`
Salva o dicionário no armazenamento persistente.
- **Retorna**: `nil`

#### `dictLoad()`
Carrega o dicionário do armazenamento persistente.
- **Retorna**: `nil`

#### `dictFormat()`
Formata/limpa o armazenamento do dicionário.
- **Retorna**: `nil`

## Controle de Servo

[↑ Voltar ao topo](#tópicos)

> **⚠️ Requer `USE_SERVO`**: `servoPause()`, `servoResume()` e `servoMove()` são registradas apenas quando o firmware é compilado com suporte a servo. `hasServo()` está sempre disponível.

#### `servoPause(servoId)`
Pausa o servo especificado.
- **Parâmetros**:
  - `servoId` (int): O ID do servo a ser pausado.
- **Retorna**: `bool` (`true` se pausado com sucesso, caso contrário `false`).

#### `servoResume(servoId)`
Retoma o servo especificado.
- **Parâmetros**:
  - `servoId` (int): O ID do servo a ser retomado.
- **Retorna**: `bool` (`true` se retomado com sucesso, caso contrário `false`).

#### `servoMove(servoId, angle)`
Move o servo especificado para o ângulo fornecido.
- **Parâmetros**:
  - `servoId` (int): O ID do servo a ser movido.
  - `angle` (float): O ângulo alvo para o servo.
- **Retorna**: `bool` (`true` se movido com sucesso, caso contrário `false`).

#### `hasServo()`
Verifica se o sistema tem um servo conectado.
- **Retorna**: `bool` (`true` se um servo estiver presente, caso contrário `false`).

## Tela Interna

[↑ Voltar ao topo](#tópicos)

### `oledDrawPixel(x, y, color)`
Desenha um único pixel na tela OLED nas coordenadas especificadas.
- **Parâmetros**:
  - `x` (int): A coordenada X.
  - `y` (int): A coordenada Y.
  - `color` (int): 1 para branco, 0 para preto.
- **Retorna**: `nil`

### `oledDrawBottomBar()`
Desenha a barra inferior.
- **Retorna**: `nil`

### `oledSetTextColor(fg[, bg])`
Define a cor do texto para desenhar na tela OLED.
- **Parâmetros**:
  - `fg` (int): Cor do primeiro plano (1 para branco, 0 para preto).
  - `bg` (int, opcional): Cor do fundo. Padrão é `1`.
- **Retorna**: `nil`

### `oledSetFontSize(size)`
Define o tamanho do texto.
- **Parâmetros**:
  - `size` (int): Multiplicador de tamanho. Padrão é `1`.
- **Retorna**: `nil`

#### `oledSetCursor(x, y)`
Define a posição do cursor na tela OLED.
- **Parâmetros**:
  - `x` (int): A coordenada X.
  - `y` (int): A coordenada Y.
- **Retorna**: `nil`

#### `oledFaceToScreen(x, y)`
Desenha a imagem atual exibida nos painéis externos na tela interna na posição especificada.
- **Parâmetros**:
  - `x` (int): A coordenada X.
  - `y` (int): A coordenada Y.
- **Retorna**: `nil`

#### `oledDrawTopBar()`
Desenha a barra superior com algumas utilidades.
- **Retorna**: `nil`

#### `oledClearScreen()`
Limpa o buffer da tela OLED.
- **Retorna**: `nil`

#### `oledDisplay()`
Envia o buffer para a tela OLED.
- **Retorna**: `nil`

#### `oledDrawText(msg)`
Desenha texto na posição atual do cursor.
- **Parâmetros**:
  - `msg` (string): O texto a ser desenhado.
- **Retorna**: `nil`

#### `oledDrawRect(x, y, width, height, color)`
Desenha o contorno de um retângulo na tela OLED.
- **Parâmetros**:
  - `x` (int): Coordenada X.
  - `y` (int): Coordenada Y.
  - `width` (int): Largura do retângulo.
  - `height` (int): Altura do retângulo.
  - `color` (int): 1 para branco, 0 para preto.
- **Retorna**: `nil`

#### `oledDrawFilledRect(x, y, width, height, color)`
Desenha um retângulo preenchido na tela OLED.
- **Parâmetros**:
  - `x` (int): Coordenada X.
  - `y` (int): Coordenada Y.
  - `width` (int): Largura do retângulo.
  - `height` (int): Altura do retângulo.
  - `color` (int): 1 para branco, 0 para preto.
- **Retorna**: `nil`

#### `oledDrawLine(x, y, x2, y2, color)`
Desenha uma linha na tela OLED.
- **Parâmetros**:
  - `x, y` (int): Ponto inicial.
  - `x2, y2` (int): Ponto final.
  - `color` (int): 1 para branco, 0 para preto.
- **Retorna**: `nil`

#### `oledDrawFastHLine(x, y, w, color)`
Desenha uma linha horizontal rápida na tela OLED.
- **Parâmetros**:
  - `x` (int): Coordenada X inicial.
  - `y` (int): Coordenada Y.
  - `w` (int): Largura.
  - `color` (int): 1 para branco, 0 para preto.
- **Retorna**: `nil`

#### `oledDrawFastVLine(x, y, w, color)`
Desenha uma linha vertical rápida na tela OLED.
- **Parâmetros**:
  - `x` (int): Coordenada X.
  - `y` (int): Coordenada Y inicial.
  - `w` (int): Altura.
  - `color` (int): 1 para branco, 0 para preto.
- **Retorna**: `nil`

#### `oledDrawCircle(x, y, r, color)`
Desenha o contorno de um círculo na tela OLED.
- **Parâmetros**:
  - `x` (int): Coordenada X do centro.
  - `y` (int): Coordenada Y do centro.
  - `r` (int): Raio.
  - `color` (int): 1 para branco, 0 para preto.
- **Retorna**: `nil`

#### `oledDrawFilledCircle(x, y, r, color)`
Desenha um círculo preenchido na tela OLED.
- **Parâmetros**:
  - `x` (int): Coordenada X do centro.
  - `y` (int): Coordenada Y do centro.
  - `r` (int): Raio.
  - `color` (int): 1 para branco, 0 para preto.
- **Retorna**: `nil`

#### `oledCreateIcon(width, height, data)`
Cria um ícone a partir de dados binários e retorna seu ID.
- **Parâmetros**:
  - `width` (int): A largura do ícone.
  - `height` (int): A altura do ícone.
  - `data` (array de int): Dados binários de pixels do ícone.
- **Retorna**: `int` (O ID do ícone).

#### `oledDrawIcon(x, y, iconId)`
Desenha um ícone criado anteriormente na posição especificada.
- **Parâmetros**:
  - `x` (int): Coordenada X.
  - `y` (int): Coordenada Y.
  - `iconId` (int): O ID do ícone retornado por `oledCreateIcon`.
- **Retorna**: `nil`

## Fitas de LED

[↑ Voltar ao topo](#tópicos)

#### `ledsBegin(led_count, [max_brightness])`
Inicializa a fita de LED.
- **Parâmetros**:
  - `led_count` (int): O número de LEDs na fita.
  - `max_brightness` (int, opcional): Brilho máximo (0-255). Padrão é `128`.
- **Retorna**: `bool`

#### `ledsBeginDual(led_count, led_count2, [max_brightness])`
Inicializa duas fitas de LED individuais.
- **Parâmetros**:
  - `led_count` (int): Número de LEDs na primeira fita.
  - `led_count2` (int): Número de LEDs na segunda fita.
  - `max_brightness` (int, opcional): Brilho máximo (0-255). Padrão é `128`.
- **Retorna**: `bool`

#### `ledsIsManaged()`
Verifica se os LEDs estão no modo gerenciado.
- **Retorna**: `bool`

#### `ledsGentlySeBrightness(brightness, [rate, [startAmount]])`
Aumenta lentamente o brilho até um valor alvo.
- **Parâmetros**:
  - `brightness` (int): Brilho alvo.
  - `rate` (int, opcional): Unidades de brilho aumentadas por frame. Padrão é `1`.
  - `startAmount` (int, opcional): Brilho inicial. Padrão é `0`.
- **Retorna**: `nil`

#### `ledsStackCurrentBehavior()`
Salva o estado atual do comportamento dos LEDs em uma pilha. Útil para comportamentos temporários.
- **Retorna**: `int`

#### `ledsPopBehavior()`
Restaura o comportamento dos LEDs do topo da pilha.
- **Retorna**: `int`

#### `ledsSegmentRange(id, from, to)`
Define um segmento da fita de LED para controle independente.
- **Parâmetros**:
  - `id` (int): ID do segmento (0-15).
  - `from` (int): Índice do LED inicial.
  - `to` (int): Índice do LED final.
- **Retorna**: `nil`

#### `ledsSetManaged(managed)`
Habilita ou desabilita o modo gerenciado.
- **Parâmetros**:
  - `managed` (bool): `true` para habilitar, `false` para desabilitar.
- **Retorna**: `nil`

#### `ledsSetBrightness(brightness)`
Define o brilho da fita de LED.
- **Parâmetros**:
  - `brightness` (int): Brilho (0-255).
- **Retorna**: `nil`

#### `ledsGetBrightness()`
Retorna o brilho atual da fita de LED.
- **Retorna**: `int`

#### `ledsDisplay()`
Aplica as alterações nos LEDs na fita. Usar quando o modo gerenciado estiver desabilitado.
- **Retorna**: `nil`

#### `ledsSegmentBehavior(id, behavior, [p1, p2, p3, p4])`
Define o comportamento para um segmento específico de LED.
- **Parâmetros**:
  - `id` (int): ID do segmento (0-15).
  - `behavior` (int): Constante de comportamento (ex: `BEHAVIOR_PRIDE`, `BEHAVIOR_ROTATE`).
  - `p1..p4` (int, opcional): Parâmetros específicos do comportamento.
- **Retorna**: `nil`

#### `ledsSegmentTweenBehavior(id, behavior, [p1, p2, p3, p4])`
Transiciona suavemente para um novo comportamento para um segmento específico de LED.
- **Parâmetros**:
  - `id` (int): ID do segmento (0-15).
  - `behavior` (int): Constante de comportamento alvo.
  - `p1..p4` (int, opcional): Parâmetros específicos do comportamento.
- **Retorna**: `nil`

#### `ledsSegmentTweenSpeed(id, speed)`
Define a velocidade de transição para um segmento específico de LED.
- **Parâmetros**:
  - `id` (int): ID do segmento (0-15).
  - `speed` (int): Velocidade de transição (valores maiores = transições mais lentas).
- **Retorna**: `nil`

#### `ledsSetColor(id, r, g, b)`
Define a cor de um LED individual. Idealmente não usar no modo gerenciado.
- **Parâmetros**:
  - `id` (int): Índice do LED individual.
  - `r, g, b` (int): Componentes de cor RGB (0-255).
- **Retorna**: `nil`

#### `ledsSegmentColor(id, r, g, b)`
Define a cor de um segmento de LED. Idealmente não usar no modo gerenciado.
- **Parâmetros**:
  - `id` (int): ID do segmento.
  - `r, g, b` (int): Componentes de cor RGB (0-255).
- **Retorna**: `nil`

## Núcleo Arduino

[↑ Voltar ao topo](#tópicos)

#### `tone(frequency)`
Gera um tom no buzzer.
- **Parâmetros**:
  - `frequency` (int): Frequência do tom em Hz.
- **Retorna**: `nil`

#### `toneDuration(frequency, duration)`
Gera um tom com duração definida.
- **Parâmetros**:
  - `frequency` (int): Frequência do tom em Hz.
  - `duration` (int): Duração em milissegundos.
- **Retorna**: `nil`

#### `noTone()`
Para a geração de tom.
- **Retorna**: `nil`

#### `millis()`
Retorna o número de milissegundos desde que o sistema começou a rodar.
- **Retorna**: `int`

#### `delay(ms)`
Atrasa a execução pelo número especificado de milissegundos.
- **Parâmetros**:
  - `ms` (int): Milissegundos para atrasar.
- **Retorna**: `nil`

#### `delayMicroseconds(us)`
Atrasa a execução pelo número especificado de microssegundos.
- **Parâmetros**:
  - `us` (int): Microssegundos para atrasar.
- **Retorna**: `nil`

#### `vTaskDelay(ticks)`
Atrasa usando o delay de tarefa do FreeRTOS (cede para outras tarefas).
- **Parâmetros**:
  - `ticks` (int): Número de ticks do FreeRTOS para atrasar.
- **Retorna**: `nil`

#### `digitalWrite(pin, value)`
Escreve um valor digital em um pino.
- **Parâmetros**:
  - `pin` (int): Número do pino.
  - `value` (int): `HIGH` ou `LOW`.
- **Retorna**: `nil`

#### `digitalRead(pin)`
Lê um valor digital de um pino.
- **Parâmetros**:
  - `pin` (int): Número do pino.
- **Retorna**: `int` (`HIGH` ou `LOW`)

#### `analogRead(pin)`
Lê um valor analógico de um pino.
- **Parâmetros**:
  - `pin` (int): Número do pino.
- **Retorna**: `int`

#### `pinMode(pin, mode)`
Define o modo de um pino.
- **Parâmetros**:
  - `pin` (int): Número do pino.
  - `mode` (int): `INPUT`, `OUTPUT`, `INPUT_PULLUP`, etc.
- **Retorna**: `nil`

### Comunicação Serial

SerialIo usa os pinos IO1 e IO2 como TX e RX.

#### `beginSerialIo([baud])`
Inicializa a porta serial secundária (pinos IO1/IO2).
- **Parâmetros**:
  - `baud` (int, opcional): Taxa de transmissão. Padrão é `115200`.
- **Retorna**: `nil`

#### `setTimeoutSerialIo(timeout)`
Define o timeout de leitura para a porta serial secundária.
- **Parâmetros**:
  - `timeout` (int): Timeout em milissegundos.
- **Retorna**: `nil`

#### `serialIoAvaliable()`
Retorna o número de bytes disponíveis para leitura na porta serial secundária.
- **Retorna**: `int`

#### `serialAvaliable()`
Retorna o número de bytes disponíveis para leitura na porta serial primária.
- **Retorna**: `int`

#### `serialIoReadStringUntil([terminator])`
Lê caracteres da porta serial secundária até encontrar o terminador.
- **Parâmetros**:
  - `terminator` (char, opcional): Caractere terminador. Padrão é `'\n'`.
- **Retorna**: `string`

#### `serialReadStringUntil([terminator])`
Lê caracteres da porta serial primária até encontrar o terminador.
- **Parâmetros**:
  - `terminator` (char, opcional): Caractere terminador. Padrão é `'\n'`.
- **Retorna**: `string`

#### `serialIoRead()`
Lê um byte da porta serial secundária.
- **Retorna**: `int` (Byte lido, ou -1 se nenhum disponível).

#### `serialRead()`
Lê um byte da porta serial primária.
- **Retorna**: `int` (Byte lido, ou -1 se nenhum disponível).

#### `serialIoWrite(data)`
Escreve um único byte na porta serial secundária.
- **Parâmetros**:
  - `data` (int): Byte a escrever (0-255).
- **Retorna**: `int` (Bytes escritos).

#### `serialWrite(data)`
Escreve um único byte na porta serial primária.
- **Parâmetros**:
  - `data` (int): Byte a escrever (0-255).
- **Retorna**: `int` (Bytes escritos).

#### `serialIoWriteString(data)`
Escreve uma string na porta serial secundária.
- **Parâmetros**:
  - `data` (string): A string a escrever.
- **Retorna**: `int` (Bytes escritos).

#### `serialWriteString(data)`
Escreve uma string na porta serial primária.
- **Parâmetros**:
  - `data` (string): A string a escrever.
- **Retorna**: `int` (Bytes escritos).

#### `serialIoAvailableForWrite()`
Retorna o número de bytes que podem ser escritos sem bloqueio na serial secundária.
- **Retorna**: `int`

#### `serialAvailableForWrite()`
Retorna o número de bytes que podem ser escritos sem bloqueio na serial primária.
- **Retorna**: `int`

### Comunicação I2C

#### `wireBegin(addr)`
Inicializa o barramento I2C. Passe `0` para modo mestre.
- **Parâmetros**:
  - `addr` (uint8_t): Endereço escravo de 7 bits, ou `0` para modo mestre.
- **Retorna**: `bool`

#### `wireAvailable()`
Retorna o número de bytes disponíveis para leitura no barramento I2C.
- **Retorna**: `int`

#### `wireBeginTransmission(addr)`
Inicia uma transmissão para o endereço I2C especificado.
- **Parâmetros**:
  - `addr` (uint8_t): Endereço do dispositivo de 7 bits.
- **Retorna**: `nil`

#### `wireEndTransmission([sendStop])`
Finaliza a transmissão I2C.
- **Parâmetros**:
  - `sendStop` (bool, opcional): Enviar condição de parada. Padrão é `true`.
- **Retorna**: `uint8_t` (Status da transmissão).

#### `wireRequestFrom(address, size, sendStop)`
Solicita bytes de um dispositivo I2C específico.
- **Parâmetros**:
  - `address` (uint16_t): Endereço do dispositivo de 7 bits.
  - `size` (int): Número de bytes a solicitar.
  - `sendStop` (bool): Se deve enviar condição de parada.
- **Retorna**: `uint8_t` (Número de bytes recebidos).

#### `wireRead()`
Lê um byte do buffer I2C.
- **Retorna**: `int`

#### `wireReadBytes(length)`
Lê múltiplos bytes do buffer I2C.
- **Parâmetros**:
  - `length` (int): Número de bytes a ler.
- **Retorna**: `tabela` (Array de bytes).

#### `wirePeek()`
Espia o próximo byte no buffer I2C sem consumi-lo.
- **Retorna**: `int`

#### `wireFlush()`
Limpa o buffer I2C.
- **Retorna**: `nil`

#### `wireParseFloat()`
Analisa um float do buffer I2C.
- **Retorna**: `float`

#### `wireParseInt()`
Analisa um inteiro do buffer I2C.
- **Retorna**: `int`

#### `wireSetTimeout(timeout)`
Define o timeout para operações I2C.
- **Parâmetros**:
  - `timeout` (uint32_t): Timeout em milissegundos.
- **Retorna**: `nil`

#### `wireGetTimeout()`
Obtém o timeout atual para operações I2C.
- **Retorna**: `uint32_t`

## Controle Remoto IR

[↑ Voltar ao topo](#tópicos)

Estas funções permitem receber comandos de controles remotos infravermelhos via receptor VS1838B ou compatível.

#### `startIR()`
Inicializa o receptor IR.
- **Retorna**: `bool` (`true` se bem-sucedido).

#### `hasIRStarted()`
Retorna se o receptor IR foi inicializado.
- **Retorna**: `bool`

#### `setIRInterruptPin(pin)`
Define o pino GPIO conectado ao receptor IR.
- **Parâmetros**:
  - `pin` (uint16_t): O número do pino GPIO.
- **Retorna**: `nil`

#### `enableIRInterrupt([mode])`
Habilita a interrupção no pino do receptor IR.
- **Parâmetros**:
  - `mode` (int, opcional): Modo de interrupção (ex: `CHANGE`, `RISING`, `FALLING`). Padrão é `CHANGE`.
- **Retorna**: `nil`

#### `disableIRInterrupt()`
Desabilita a interrupção no pino do receptor IR.
- **Retorna**: `nil`

#### `hasIRCommand()`
Retorna se um novo comando IR foi recebido.
- **Retorna**: `bool`

#### `getLastIRCommand()`
Retorna o último comando IR recebido como um objeto `IrCommand`.
- **Retorna**: `IrCommand`

## Modelos 2D

[↑ Voltar ao topo](#tópicos)

Estas funções expõem o sistema de renderização de modelos 2D adicionado na v3.0.0. Os modelos são renderizados no painel usando o sistema de animação por keyframe. Os IDs de frame de modelo começam em `MODEL_FRAME_ID_OFFSET`.

#### `loadModel(modelData, name)`
Carrega um modelo 2D a partir de uma string JSON ou arquivo e o registra com o nome fornecido.
- **Parâmetros**:
  - `modelData` (string): Dados do modelo ou caminho do arquivo.
  - `name` (string, opcional): Nome identificador para o modelo. Padrão é `""`.
- **Retorna**: Objeto `Model`

### Classe `Model`

Objetos Model são retornados por `loadModel()`. Não podem ser criados diretamente.

#### `model:Recalculate()`
Recalcula a geometria do modelo (ex: após modificar pontos).
- **Retorna**: `nil`

#### `model:Reset()`
Redefine o modelo ao seu estado original.
- **Retorna**: `nil`

#### `model:GetId()`
Retorna o ID interno do modelo.
- **Retorna**: `int`

#### `model:CopyToRaster()`
Copia o estado atual do modelo para o rasterizador para renderização.
- **Retorna**: `nil`

#### `model:AddPointGroup()`
Adiciona um novo grupo de pontos ao modelo.
- **Retorna**: `nil`

#### `model:SetTriangle(index, p1, p2, p3)`
Define um triângulo referenciando três índices de pontos.
- **Parâmetros**:
  - `index` (int): Índice do triângulo.
  - `p1, p2, p3` (int): Índices de pontos.
- **Retorna**: `nil`

#### `model:GetTriangle(index)`
Obtém os dados do triângulo no índice fornecido.
- **Parâmetros**:
  - `index` (int): Índice do triângulo.
- **Retorna**: `tabela`

#### `model:SetBatchOperations(enabled)`
Habilita ou desabilita o modo de operações em lote para transformações em massa mais rápidas.
- **Parâmetros**:
  - `enabled` (bool)
- **Retorna**: `nil`

#### `model:SetAccumulativeOperations(enabled)`
Habilita ou desabilita o modo acumulativo, onde as transformações se somam em vez de substituir.
- **Parâmetros**:
  - `enabled` (bool)
- **Retorna**: `nil`

#### `model:SetPointPosition(pointId, x, y, z)`
Define a posição absoluta de um único ponto.
- **Parâmetros**:
  - `pointId` (int): Índice do ponto.
  - `x, y, z` (float): Nova posição.
- **Retorna**: `nil`

#### `model:TranslatePoint(pointId, x, y, z)`
Move um único ponto por um delta.
- **Parâmetros**:
  - `pointId` (int): Índice do ponto.
  - `x, y, z` (float): Delta de translação.
- **Retorna**: `nil`

#### `model:SetPointsPosition(x, y, z)`
Define todos os pontos para a posição fornecida.
- **Parâmetros**:
  - `x, y, z` (float): Posição.
- **Retorna**: `nil`

#### `model:ScalePoints(x, y, z)`
Escala todos os pontos pelos fatores fornecidos.
- **Parâmetros**:
  - `x, y, z` (float): Fatores de escala.
- **Retorna**: `nil`

#### `model:TranslatePoints(x, y, z)`
Translada todos os pontos por um delta.
- **Parâmetros**:
  - `x, y, z` (float): Delta de translação.
- **Retorna**: `nil`

#### `model:Scale(x, y, z)`
Escala o modelo inteiro.
- **Parâmetros**:
  - `x, y, z` (float): Fatores de escala.
- **Retorna**: `nil`

#### `model:Rotate(x, y, z)`
Rotaciona o modelo.
- **Parâmetros**:
  - `x, y, z` (float): Ângulos de rotação.
- **Retorna**: `nil`

#### `model:Translate(x, y, z)`
Translada o modelo.
- **Parâmetros**:
  - `x, y, z` (float): Delta de translação.
- **Retorna**: `nil`

#### `model:GetCenter()`
Retorna o ponto central do modelo.
- **Retorna**: `float, float, float` (x, y, z)

## Animações por Keyframe

[↑ Voltar ao topo](#tópicos)

Animações por keyframe controlam transformações de modelos 2D ao longo do tempo. São criadas com `newKeyframeAnimation()` e reproduzidas via `setPanelModelAnimation()`.

#### `newKeyframeAnimation(duration)`
Cria uma nova animação por keyframe com a duração fornecida.
- **Parâmetros**:
  - `duration` (int): Duração total em milissegundos.
- **Retorna**: Objeto `KeyframeAnimation`

### Classe `KeyframeAnimation`

#### `anim:Reset()`
Reinicia a animação do início.
- **Retorna**: `nil`

#### `anim:GetId()`
Retorna o ID interno desta animação (usado com `setPanelModelAnimation`).
- **Retorna**: `int`

#### `anim:AddTrack(track)`
Adiciona uma `KeyframeTrack` a esta animação.
- **Parâmetros**:
  - `track` (KeyframeTrack): A track a adicionar.
- **Retorna**: `nil`

### Classe `KeyframeTrack`

Objetos KeyframeTrack podem ser criados diretamente com `KeyframeTrack()`.

#### `KeyframeTrack()`
Cria uma nova track de keyframe vazia.
- **Retorna**: Objeto `KeyframeTrack`

#### `track:Reset()`
Reinicia a track.
- **Retorna**: `nil`

#### `track:SetResource(modelId)`
Associa esta track a um modelo pelo ID.
- **Parâmetros**:
  - `modelId` (int): O ID do modelo a controlar.
- **Retorna**: `nil`

#### `track:AddKeyFrame(keyframe)`
Adiciona um keyframe a esta track.
- **Parâmetros**:
  - `keyframe` (Keyframe): O keyframe a adicionar.
- **Retorna**: `nil`

### `KeyFrame(time, value)`
Cria um novo keyframe. Os parâmetros dependem do tipo de operação sendo animada.
- **Parâmetros**:
  - `time` (uint16): Offset de tempo em ms.
  - `value` (Vec2f ou similar): O valor neste keyframe.
- **Retorna**: Objeto `Keyframe`

---

# Constantes Lua

[↑ Voltar ao topo](#tópicos)

- [Relacionadas ao Motor](#relacionadas-ao-motor)
- [Entrada](#entrada)
- [Comportamento de LED](#comportamento-de-led)
- [Pinos e GPIO](#pinos-e-gpio)
- [Modos de Cor](#modos-de-cor)
- [Shaders](#shaders)
- [Tipos de Keyframe](#tipos-de-keyframe)
- [Níveis de Potência BLE](#níveis-de-potência-ble)
- [Modos de Interrupção](#modos-de-interrupção)
- [Motivo de Reinício do ESP32](#motivo-de-reinício-do-esp32)

## Relacionadas ao Motor

- `PANDA_VERSION`: String representando a versão atual do firmware.
- `VCC_THRESHOLD_START`: Tensão mínima necessária para o sistema iniciar.
- `VCC_THRESHOLD_HALT`: Tensão abaixo da qual o sistema para para evitar danos.
- `OLED_SCREEN_WIDTH`: Largura da tela OLED interna em pixels.
- `OLED_SCREEN_HEIGHT`: Altura da tela OLED interna em pixels.
- `CANVAS_WIDTH`: Largura do painel HUB75 em pixels.
- `CANVAS_HEIGHT`: Altura do painel HUB75 em pixels.
- `POWER_MODE_NONE`: Ignorar qualquer comportamento de alimentação.
- `POWER_MODE_USB_5V`: Modo de alimentação USB 5V.
- `POWER_MODE_USB_9V`: Modo de alimentação USB 9V PD.
- `POWER_MODE_BATTERY`: Modo de alimentação por bateria.
- `BUILT_IN_POWER_MODE`: O modo de alimentação definido em tempo de compilação.
- `SERVO_COUNT`: O número de servos.
- `MODEL_FRAME_ID_OFFSET`: Offset de ID de frame usado para frames de modelo. IDs de frame de modelo começam neste valor.
- `EDIT_MODE_PIN`: Número do pino GPIO usado para entrar no modo de edição.
- `EDIT_ENABLE_LOGIC_LEVEL`: Nível lógico que aciona o modo de edição.
- `ENABLE_EDIT_MODE`: `1` se o modo de edição está compilado.
- `PIN_ENABLE_REGULATOR`: Pino GPIO que controla o regulador de tensão (`-1` se não definido).
- `PIN_USB_BATTERY_IN`: Pino GPIO para leitura de tensão da bateria/USB (`-1` se não definido).
- `USE_PIN_BATTERY_IN`: `1` se a detecção de tensão da bateria está habilitada.
- `RESISTOR_DIVIDER_R8`, `RESISTOR_DIVIDER_R9`: Valores dos resistores do divisor de tensão.
- `V_REF`: Tensão de referência do ADC.
- `BLACK`: Constante de cor OLED (`1`).
- `WHITE`: Constante de cor OLED (`0`).
- `MAX_LED_GROUPS`: Número de grupos/segmentos de LED disponíveis.

## Entrada

- `BUTTON_RELEASED`: Botão não está pressionado.
- `BUTTON_JUST_PRESSED`: Botão acabou de ser pressionado neste frame.
- `BUTTON_PRESSED`: Botão está sendo mantido pressionado.
- `BUTTON_JUST_RELEASED`: Botão acabou de ser solto neste frame.
- `DEVICE_X_BUTTON_LEFT/RIGHT/UP/DOWN/CONFIRM/AUX_A/AUX_B/BACK`: Mapeamentos de botões para o dispositivo X (0 a `MAX_BLE_CLIENTS`).
- `BUTTON_LEFT/RIGHT/UP/DOWN/CONFIRM/AUX_A/AUX_B/BACK`: Aliases para os botões do dispositivo 0.

## Comportamento de LED

- `BEHAVIOR_NONE`: Sem comportamento; LEDs permanecem desligados ou inalterados.
- `BEHAVIOR_PRIDE`: Padrão de cores arco-íris ciclando.
- `BEHAVIOR_ROTATE`: Rotaciona uma cor ao longo do segmento.
- `BEHAVIOR_RANDOM_COLOR`: Cada LED recebe uma cor aleatória.
- `BEHAVIOR_FADE_CYCLE`: Faz o brilho pulsar em ciclo.
- `BEHAVIOR_ROTATE_FADE_CYCLE`: Combina rotação e pulsação.
- `BEHAVIOR_STATIC_RGB`: Cor RGB estática.
- `BEHAVIOR_STATIC_HSV`: Cor HSV estática.
- `BEHAVIOR_RANDOM_BLINK`: Pisca LEDs aleatoriamente.
- `BEHAVIOR_ROTATE_SINE_V`: Onda senoidal no brilho (V no HSV).
- `BEHAVIOR_ROTATE_SINE_S`: Onda senoidal na saturação (S no HSV).
- `BEHAVIOR_ROTATE_SINE_H`: Onda senoidal no matiz (H no HSV).
- `BEHAVIOR_FADE_IN`: Faz os LEDs aparecerem gradualmente até uma cor especificada.
- `BEHAVIOR_NOISE`: Efeito de ruído/estática.
- `BEHAVIOR_ICON_X`: Exibe um padrão de ícone X no segmento.
- `BEHAVIOR_ICON_I`: Exibe um padrão de ícone I no segmento.
- `BEHAVIOR_ICON_V`: Exibe um padrão de ícone V no segmento.
- `MAX_LED_GROUPS`: Total de grupos de LED disponíveis.

## Pinos e GPIO

- `D1`, `D2`: Pinos de IO externos (IO1 e IO2).
- `HIGH`, `LOW`: Estados dos pinos digitais.
- `INPUT`, `OUTPUT`: Modos de pino.
- `INPUT_PULLUP`, `INPUT_PULLDOWN`: Modos de entrada com resistores internos.
- `ANALOG`: Modo de pino analógico.
- `OUTPUT_OPEN_DRAIN`, `OPEN_DRAIN`: Modo de saída em dreno aberto.
- `PULLDOWN`: Modo pull-down.

## Modos de Cor

Usados com `setPanelColorMode()` para painéis com fiação de canal RGB não padrão.

- `COLOR_MODE_RGB`
- `COLOR_MODE_RBG`
- `COLOR_MODE_GRB`
- `COLOR_MODE_GBR`
- `COLOR_MODE_BRG`
- `COLOR_MODE_BGR`

## Shaders

Usados com `setAnimationShader()`.

- `SHADER_NONE`: Sem shader.
- `SHADER_RAINBOW`: Sobreposição de arco-íris.
- `SHADER_FIRE`: Efeito de fogo.
- `SHADER_TEXTURE`: Shader baseado em textura.
- `SHADER_TRANS`: Shader com as cores da bandeira trans 🏳️‍⚧️
- `SHADER_LAST`: Alias para o último shader disponível.

## Tipos de Keyframe

Usados ao construir animações `KeyframeTrack`.

- `KEYFRAME_TRANSLATE`: Keyframe de translação.
- `KEYFRAME_ROTATE`: Keyframe de rotação.
- `KEYFRAME_SCALE`: Keyframe de escala.
- `KEYFRAME_RESET`: Redefine a transformação do modelo.
- `KEYFRAME_COLOR`: Keyframe de mudança de cor.
- `KEYFRAME_VISIBILITY`: Keyframe de alternância de visibilidade.
- `KEYFRAME_SINE`: Keyframe de movimento senoidal.
- `KEYFRAME_SHADER`: Keyframe de mudança de shader.

## Níveis de Potência BLE

Usados com `startBLERadio(powerLevel)`.

- `ESP_PWR_LVL_N24` a `ESP_PWR_LVL_P21`: Níveis de potência TX do BLE em dBm, de -24 dBm (mais fraco) a +21 dBm (mais forte).

## Modos de Interrupção

Usados com `enableIRInterrupt([mode])` e `attachInterrupt`.

- `RISING`: Acionar na borda de subida.
- `FALLING`: Acionar na borda de descida.
- `CHANGE`: Acionar em qualquer mudança.
- `ONLOW`: Acionar quando o pino estiver em nível baixo.
- `ONHIGH`: Acionar quando o pino estiver em nível alto.
- `ONLOW_WE`: Acionar quando baixo (capaz de acordar o sistema).
- `ONHIGH_WE`: Acionar quando alto (capaz de acordar o sistema).

## Motivo de Reinício do ESP32

Retornado por `getResetReason()`.

- `ESP_RST_UNKNOWN`
- `ESP_RST_POWERON`
- `ESP_RST_EXT`
- `ESP_RST_SW`
- `ESP_RST_PANIC`
- `ESP_RST_INT_WDT`
- `ESP_RST_TASK_WDT`
- `ESP_RST_WDT`
- `ESP_RST_DEEPSLEEP`
- `ESP_RST_BROWNOUT`
- `ESP_RST_SDIO`

---

# Comportamentos de LED

[↑ Voltar ao topo](#tópicos)

## Tabela de Parâmetros

Todos os valores de parâmetros estão no intervalo 0–255, exceto onde indicado como **ms** (milissegundos).

| Modo                  | param1       | param2         | param3          | param4            |
|-----------------------|--------------|----------------|-----------------|-------------------|
| `none`                | -            | -              | -               | -                 |
| `pride`               | -            | -              | -               | -                 |
| `rotate`              | -            | -              | -               | velocidade (ms)   |
| `random_color`        | -            | -              | -               | -                 |
| `fade_cycle`          | matiz        | velocidade (ms)| brilho_mínimo   | -                 |
| `rotate_fade_cycle`   | matiz        | velocidade (ms)| brilho_mínimo   | vel_rotação (ms)  |
| `color_rgb`           | vermelho     | verde          | azul            | -                 |
| `color_hsv`           | matiz        | saturação      | valor           | -                 |
| `random_blink`        | matiz_base   | var_matiz      | brilho          | vel_pisca (ms)    |
| `icon_x`              | -            | -              | -               | -                 |
| `icon_i`              | -            | -              | -               | -                 |
| `icon_v`              | -            | -              | -               | -                 |
| `rotate_sine_v`       | matiz        | saturação      | velocidade (ms) | -                 |
| `rotate_sine_s`       | matiz        | brilho         | velocidade (ms) | -                 |
| `rotate_sine_h`       | saturação    | brilho         | velocidade (ms) | -                 |
| `fade_in`             | matiz        | saturação      | passo           | atraso (ms)       |
| `noise`               | -            | -              | passo           | atraso (ms)       |

Os parâmetros são passados posicionalmente para `ledsSegmentBehavior(id, behavior, param1, param2, param3, param4)`. Posições não utilizadas devem ser `0`.

---

* `BEHAVIOR_PRIDE`
  - **Descrição**: Um efeito arco-íris que percorre as cores em um padrão suave.
  - **Parâmetros**: Nenhum.

* `BEHAVIOR_ROTATE`
  - **Descrição**: Rotaciona uma única cor através do segmento, criando um efeito de luz móvel.
  - **Parâmetros**:
    - `4`: Velocidade de rotação (atraso em ms).

* `BEHAVIOR_RANDOM_COLOR`
  - **Descrição**: Define cada LED no segmento para uma cor aleatória.
  - **Parâmetros**: Nenhum.

* `BEHAVIOR_FADE_CYCLE`
  - **Descrição**: Aumenta e diminui o brilho dos LEDs no segmento em um ciclo.
  - **Parâmetros**:
    - `1`: Valor de matiz (0-255).
    - `2`: Velocidade do ciclo de fade (ms).
    - `3`: Valor mínimo de brilho.

* `BEHAVIOR_ROTATE_FADE_CYCLE`
  - **Descrição**: Combina rotação e fade, criando um efeito de luz móvel com brilho variável.
  - **Parâmetros**:
    - `1`: Valor de matiz (0-255).
    - `2`: Velocidade do ciclo de fade (ms).
    - `3`: Valor mínimo de brilho.
    - `4`: Velocidade de rotação (ms).

* `BEHAVIOR_STATIC_RGB`
  - **Descrição**: Define todos os LEDs no segmento para uma cor RGB estática.
  - **Parâmetros**:
    - `1`: Vermelho (0-255).
    - `2`: Verde (0-255).
    - `3`: Azul (0-255).

* `BEHAVIOR_STATIC_HSV`
  - **Descrição**: Define todos os LEDs no segmento para uma cor HSV estática.
  - **Parâmetros**:
    - `1`: Matiz (0-255).
    - `2`: Saturação (0-255).
    - `3`: Brilho (0-255).

* `BEHAVIOR_RANDOM_BLINK`
  - **Descrição**: Pisca LEDs aleatoriamente no segmento com cores e brilhos aleatórios.
  - **Parâmetros**:
    - `1`: Matiz base (0-255).
    - `2`: Variação de matiz.
    - `3`: Brilho máximo.
    - `4`: Atraso entre piscadas (ms).

* `BEHAVIOR_ROTATE_SINE_V`
  - **Descrição**: Onda senoidal variando o brilho rotaciona pelo segmento.
  - **Parâmetros**:
    - `1`: Matiz base (0-255).
    - `2`: Saturação base (0-255).
    - `3`: Velocidade da onda senoidal (ms).

* `BEHAVIOR_ROTATE_SINE_S`
  - **Descrição**: Onda senoidal variando a saturação rotaciona pelo segmento.
  - **Parâmetros**:
    - `1`: Matiz base (0-255).
    - `2`: Brilho base (0-255).
    - `3`: Velocidade da onda senoidal (ms).

* `BEHAVIOR_ROTATE_SINE_H`
  - **Descrição**: Onda senoidal variando o matiz rotaciona pelo segmento.
  - **Parâmetros**:
    - `1`: Saturação base (0-255).
    - `2`: Brilho base (0-255).
    - `3`: Velocidade da onda senoidal (ms).

* `BEHAVIOR_FADE_IN`
  - **Descrição**: Faz os LEDs aparecerem gradualmente até uma cor HSV especificada.
  - **Parâmetros**:
    - `1`: Matiz (0-255).
    - `2`: Saturação (0-255).
    - `3`: Tamanho do passo.
    - `4`: Atraso entre passos (ms).

* `BEHAVIOR_NOISE`
  - **Descrição**: Efeito de ruído/estática piscando pelo segmento.
  - **Parâmetros**:
    - `3`: Tamanho do passo.
    - `4`: Atraso entre passos (ms).

* `BEHAVIOR_ICON_X`, `BEHAVIOR_ICON_I`, `BEHAVIOR_ICON_V`
  - **Descrição**: Exibe um padrão de ícone simples (formato X, I ou V) no segmento.
  - **Parâmetros**: Nenhum.

* `BEHAVIOR_NONE`
  - **Descrição**: Nenhum comportamento aplicado. LEDs permanecem desligados ou inalterados.
  - **Parâmetros**: Nenhum.

---

# Interface Bluetooth

[↑ Voltar ao topo](#tópicos)

## Funções de Rádio e BLE

#### `startBLE()`
Inicializa o ambiente BLE mas não inicia o rádio ainda.
- **Retorna**: `bool`

#### `hasBLEStarted()`
Retorna se o BLE foi inicializado.
- **Retorna**: `bool`

#### `startBLERadio(powerLevel)`
Inicia o rádio BLE no nível de potência especificado.
- **Parâmetros**:
  - `powerLevel` (int): Uma das constantes `ESP_PWR_LVL_*`.
- **Retorna**: `bool`

#### `getRRSI(connId)`
Retorna o RSSI de uma determinada conexão.
- **Parâmetros**:
  - `connId` (int): O ID da conexão.
- **Retorna**: `int`

#### `getClientIdFromControllerId(id)`
Retorna o ID de conexão para um dado ID de controlador.
- **Parâmetros**:
  - `id` (int): ID do controlador.
- **Retorna**: `int`

#### `getConnectedRemoteControls()`
Retorna o número de controles remotos atualmente conectados.
- **Retorna**: `int`

#### `isElementIdConnected(id)`
Verifica se um controle remoto com o ID fornecido está conectado.
- **Parâmetros**:
  - `id` (int): O ID do controlador.
- **Retorna**: `bool`

#### `beginBleScanning()`
Inicia a varredura de dispositivos BLE.
- **Retorna**: `nil`

#### `setLogDiscoveredBleDevices(bool)`
Quando `true`, cada dispositivo escaneado será salvo no arquivo de log.
- **Retorna**: `nil`

#### `setMaximumControls(count)`
Define o número máximo de controles remotos conectados simultaneamente.
- **Parâmetros**:
  - `count` (int): Número máximo de dispositivos.
- **Retorna**: `nil`

#### `getCharacteristicsFromService(connectionId, uuid, refresh)`
Obtém todas as características de um determinado serviço em um dispositivo conectado.
- **Parâmetros**:
  - `connectionId` (int): ID da conexão.
  - `uuid` (string): UUID do serviço.
  - `refresh` (bool): Se deve re-consultar o dispositivo.
- **Retorna**: `nil`

## Gerenciando conexões BLE

Exemplo de como aceitar conexões de um mouse/teclado/joystick:
```lua
drivers.mouseHandler = BleServiceHandler("00001812-0000-1000-8000-00805f9b34fb")
drivers.mouseHandler:SetOnConnectCallback(drivers.onConnectHID)
drivers.mouseHandler:SetOnDisconnectCallback(drivers.onDisconnectHID)
drivers.mouseListener = drivers.mouseHandler:AddCharacteristics("2a4d")
drivers.mouseListener:SetSubscribeCallback(drivers.onMouseCallback)
drivers.mouseListener:SetCallbackModeStream(false)
```

Primeiro criamos um manipulador de serviço e passamos um UUID. Esse UUID específico pertence ao serviço de Dispositivo de Interface Humana (HID). Qualquer dispositivo anunciando esse UUID terá uma tentativa de conexão.

Callback de conexão:
```lua
function drivers.onConnectHID(connectionId, controllerId, address, name)
    log("Conectado conId="..connectionId.." controller="..controllerId.." addr=\""..address.."\" name=["..name.."]")
end
```

O `connectionId` é único por conexão. O `controllerId` é atribuído pelo Protopanda — o primeiro dispositivo conectado é `0`, o segundo é `1`. IDs são liberados na desconexão e podem ser reutilizados.

Callback de desconexão:
```lua
function drivers.onDisconnectHID(connectionId, controllerId, reason)
    log("Desconectado "..connectionId.." devido a ".. reason)
end
```

Anexando um listener a uma característica:
```lua
drivers.mouseListener = drivers.mouseHandler:AddCharacteristics("2a4d")
drivers.mouseListener:SetSubscribeCallback(drivers.onMouseCallback)
drivers.mouseListener:SetRequired(true)
drivers.mouseListener:SetCallbackModeStream(false)
```

Callback da característica:
```lua
function drivers.onMouseCallback(connectionId, controllerId, data)
  -- data é um array de inteiros 0-255
end
```

## Funções BLE

### `BleServiceHandler(uuid)`
Cria um objeto manipulador de serviço. Mantenha este objeto em uma variável global ou de longa duração — o GC Lua destruirá a referência Lua se sair de escopo, mas não a referência C++.
- **Parâmetros**:
  - `uuid` (string): UUID completo de 128 bits.
- **Retorna**: `BleServiceHandlerObject`

### `BleServiceHandler::ReadFromCharacteristics(clientId, uuid)`
Lê dados de uma característica específica em um cliente conectado.
- **Parâmetros**:
  - `clientId` (int): ID da conexão.
  - `uuid` (string): UUID de 16 bits ou completo.
- **Retorna**: `array de int`

### `BleServiceHandler::GetServices(clientId)`
Retorna os serviços disponíveis no cliente conectado.
- **Parâmetros**:
  - `clientId` (int): ID da conexão.
- **Retorna**: `array de string`

### `BleServiceHandler::GetRSSI(clientId)`
Retorna o RSSI de um cliente conectado específico.
- **Parâmetros**:
  - `clientId` (int): ID da conexão.
- **Retorna**: `int`

### `BleServiceHandler::GetClientIdFromControllerId(id)`
Retorna o ID de conexão para um dado ID de controlador.
- **Parâmetros**:
  - `id` (int): ID do controlador.
- **Retorna**: `int`

### `BleServiceHandler::AddAddressRequired(address)`
Restringe conexões apenas a dispositivos com este endereço MAC. Pode ser chamado várias vezes para permitir uma lista. Se tanto `AddAddressRequired` quanto `AddNameRequired` estiverem definidos, ambas as condições devem corresponder.
```lua
Handler:AddAddressRequired("AA:BB:CC:DD:EE")
Handler:AddAddressRequired("FF:00:11:22:33")
```
- **Parâmetros**:
  - `address` (string): String de endereço MAC.

### `BleServiceHandler::AddNameRequired(name)`
Restringe conexões apenas a dispositivos com este nome anunciado. Pode ser chamado várias vezes.
```lua
Handler:AddNameRequired("VR-PARK")
Handler:AddNameRequired("Beauty-r1")
```
- **Parâmetros**:
  - `name` (string): Nome do dispositivo.

### `BleServiceHandler::GetCharacteristics(clientId)`
Retorna as características disponíveis no serviço atual para o cliente conectado.
- **Parâmetros**:
  - `clientId` (int): ID da conexão.
- **Retorna**: `array de string`

### `BleServiceHandler::WriteToCharacteristics(message, clientId, uuid[, reply])`
Escreve uma mensagem em uma característica específica.
- **Parâmetros**:
  - `message` (array de int): Bytes a escrever.
  - `clientId` (int): ID da conexão.
  - `uuid` (string): UUID de 16 bits ou completo.
  - `reply` (bool, opcional): Se `true`, aguarda resposta.
- **Retorna**: `bool`

### `BleServiceHandler::SetOnDisconnectCallback(callback)`
Define o callback para quando um dispositivo desconectar.
- **Parâmetros**:
  - `callback` (função): `function(connectionId, controllerId, reason)`

### `BleServiceHandler::SetOnConnectCallback(callback)`
Define o callback para quando um dispositivo conectar.
- **Parâmetros**:
  - `callback` (função): `function(connectionId, controllerId, address, name)`

### `BleServiceHandler::AddCharacteristics(uuid)`
Retorna um manipulador para o UUID de característica especificado.
- **Parâmetros**:
  - `uuid` (string): UUID de 16 bits ou completo.
- **Retorna**: `BleCharacteristicsHandlerObject`

### `BleCharacteristicsHandler::SetSubscribeCallback(callback)`
Define o callback para mensagens recebidas nesta característica.
- **Parâmetros**:
  - `callback` (função): `function(connectionId, controllerId, data)`

### `BleCharacteristicsHandler::SetCallbackModeStream(stream)`
Quando `true`, processa uma mensagem por loop (mensagens ficam na fila). Quando `false`, processa todas as mensagens da fila por loop.
- **Parâmetros**:
  - `stream` (bool)

### `BleCharacteristicsHandler::SetRequired(req)`
Se `true`, o dispositivo é desconectado se esta característica não estiver presente.
- **Parâmetros**:
  - `req` (bool)