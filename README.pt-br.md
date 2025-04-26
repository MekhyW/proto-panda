# Protopanda  

Protopanda é uma plataforma open source (firmware e hardware) para controlar protogens. A ideia é ser simples o suficiente para que você só precise de um pouco de conhecimento técnico para fazê-lo funcionar, mas ao mesmo tempo flexível para que uma pessoa com o mínimo conhecimento de Lua possa fazer quase de tudo.

1. [Features](#features)   
2. [Alimentação](#alimentação)  
3. [Painéis](#painéis)  
4. [Rosto e Expressões](#rosto-e-expressões)  
5. [Tiras de LED](#tiras-de-led)  
6. [Bluetooth](#bluetooth)  
7. [Hardware](#hardware)   
8. [Programação em Lua](#programação-em-lua)  

# Features  

- Dois núcleos Xtensa LX7 de 32 bits 240 MHz  
- 16 MB RAM  
- Suporte a vários painéis HUB75  
- Suporte a WLED  
- Usa Lua  
- Dados armazenados em um cartão SD  
- Expressões podem ser adicionadas ao cartão SD como arquivos PNG  
- Pode facilmente atingir 60 FPS  
- Suporte a BLE para controle remoto  
- Suporte a USB-C com PD (Power Delivery)  
- Saída de até 5V e 5A no modo PD  
- Suporte a tela OLED interna de 128x64 pixels para menu e interface  
- gay 🏳️‍🌈  

# Alimentação  

__TLDR: Use um power bank de pelo menos 20W com PD e USB-C.__

Existem dois modos: um que alimenta diretamente em 5V via USB e outro que possui gerenciamento de energia (conversor buck), que requer de 6,5V até 12V. Este segundo modo só é habilitado através de alterações físicas na PCB.

Cada painel HUB75 pode consumir até 2A no brilho máximo, então alimentar diretamente via USB em 5V pode ser problemático. Por isso, esta versão com o regulador ativa o PD (Power Delivery) no USB, solicitando 9V 3A, o que fornece energia suficiente para ambos os painéis. Infelizmente, esta versão consome muito mais energia.

Na maioria dos casos, você não estará operando os painéis no brilho máximo nem com todos os LEDs em branco, então a versão em 5V é a recomendada.da suporta [tiras de LED](#tiras-de-led), e há uma porta dedicada a elas. A saída também é de 5V, a mesma dos painéis. Como os LEDs são do tipo WS2812B, eles podem consumir até 20 mA por LED a 100% de brilho.  

# Painéis  

Os painéis utilizados também são conhecidos como painéis HUB75. Eles são controlados pela [biblioteca hub75 do mrcodetastic](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA), e estes são os [painéis recomendados](https://pt.aliexpress.com/item/4000002686894.html).  
![Painéis HUB75](doc/panels.jpg "Painéis HUB75")  
Eles são multiplexados, o que significa que apenas alguns LEDs estão ligados por vez. É rápido o suficiente para não ser percebido pelo olho humano, mas sob luz solar direta, é difícil tirar uma boa foto sem efeito de tearing.  
![Tearing capturado na câmera](doc/tearing.jpg "Tearing capturado na câmera")  

A resolução é de 64 pixels de largura e 32 pixels de altura. Com dois painéis lado a lado, a área total é de 128x32 pixels. A profundidade de cor é de 16 bits, no formato RGB565, o que significa vermelho (0-32), verde (0-64) e azul (0-32).  

## Buffer Duplo  

Para evitar outro tipo de tearing, quando um quadro está sendo desenhado enquanto o quadro está sendo alterado, habilitamos o uso de buffer duplo. Isso significa que desenhamos os pixels na memória, mas eles não aparecem imediatamente na tela. Quando chamamos `flipPanelBuffer()`, a memória em que desenhamos é enviada para o DMA para ser constantemente exibida no painel. Então, o buffer que usamos para desenhar muda. Isso aumenta o uso de memória, mas é um preço necessário a pagar.  

# Rosto e Expressões  

O Protopanda usa imagens do cartão SD e alguns arquivos JSON para construir as sequências de animação. Todas as imagens devem ser `PNG`; posteriormente, são decodificadas para um formato bruto e armazenadas no [arquivo bulk de quadros](#arquivo-bulk).  

- [Carregando Quadros](#carregando-quadros)  
- [Expressões](#expressões)  
- [Pilha de Expressões](#pilha-de-expressões)  
- [Arquivo Bulk](#arquivo-bulk)  
- [Modo managed](#modo-managed)  

## Carregando Quadros  

Para carregar quadros, você precisa adicioná-los ao cartão SD e especificar seus locais no arquivo `config.json`:  
```json  
{  
  "frames": [  
    {"pattern": "/bolinha/input-onlinegiftools-%d.png", "from": 1, "to": 155, "flip_left": false, "alias": "bolinha"},  
    {"pattern": "/atlas/Atlas comissão expressão padrao frame %d Azul.png", "flip_left": true, "from": 1, "to": 4, "color_scheme_left": "rbg", "alias": "atlas"},  
    {"pattern": "/atlas/Atlas comissão expressão Vergonha e boop frame %d.png", "flip_left": true, "from": 1, "to": 4, "color_scheme_left": "rbg", "alias": "vergonha"},  
    {"pattern": "/atlas/trans%d.png", "flip_left": true, "from": 1, "to": 4, "color_scheme_left": "rbg", "alias": "vergonha_trans"},  
    {"pattern": "/atlas/Atlas comissão expressão OWO frame %d.png", "flip_left": true, "from": 1, "to": 4, "color_scheme_left": "rbg", "alias": "owo"},  
    {"pattern": "/atlas/Atlas comissão expressão cute pidao frame %d.png", "flip_left": true, "from": 1, "to": 3, "color_scheme_left": "rbg", "alias": "cute"},  
    {"pattern": "/atlas/Atlas comissão expressão raiva frame %d.png", "flip_left": true, "from": 1, "to": 5, "color_scheme_left": "rbg", "alias": "anger"},  
    {"pattern": "/atlas/Atlas comissão expressão apaixonado %d.png", "flip_left": true, "from": 1, "to": 2, "color_scheme_left": "rbg", "alias": "apaixonado"},  
    {"pattern": "/atlas/Atlas comissão expressão surpreso frame %d.png", "flip_left": true, "from": 1, "to": 5, "color_scheme_left": "rbg", "alias": "surpreso"},  
    {"pattern": "/atlas/Atlas comissão expressão feliz frame %d.png", "flip_left": true, "from": 1, "to": 4, "color_scheme_left": "rbg", "alias": "feliz"},  
    {"pattern": "/atlas/Atlas comissão expressão morte frame %d.png", "flip_left": true, "from": 1, "to": 2, "color_scheme_left": "rbg", "alias": "morto"},  
    {"file": "/atlas/Atlas comissão expressão tela azul.png", "flip_left": true, "alias": "tela_azul"}  
  ]  
}  
```  

> Modificar o arquivo `config.json` adicionando ou removendo arquivos forçará o sistema a reconstruir o [arquivo bulk de quadros](#arquivo-bulk).  

Cada elemento no array `frames` pode ser o caminho do arquivo ou um objeto que descreve múltiplos arquivos.  

* **pattern** (string)  
  Semelhante ao `printf`, que usa `%d` para especificar um número. Ao usar `pattern`, é necessário ter os campos `from` e `to`. Por exemplo:  
  ```json  
  {"pattern": "/bolinha/input-onlinegiftools-%d.png", "from": 1, "to": 155},  
  ```  
  Isso carregará `/bolinha/input-onlinegiftools-1.png` até `/bolinha/input-onlinegiftools-155.png`.  

* **flip_left** (bool)  
  Devido à orientação dos painéis, pode ser necessário inverter horizontalmente o lado esquerdo.  

* **alias** (string)  
  As animações são basicamente como:  
  ```  
  Desenhar quadro 1  
  esperar algum tempo  
  Desenhar quadro 2  
  ```  
  Isso pode ser um problema se você codificar as animações e precisar adicionar um quadro no meio. Para resolver isso, você pode criar um alias para uma imagem ou um grupo de imagens. O alias é apenas um nome dado ao primeiro quadro do `pattern`. Funciona como um deslocamento.  

* **color_scheme_left** (string)  
  Se você precisar inverter um ou mais canais, use isso para fazê-lo.  

## Expressões  

Uma vez que os quadros são carregados e a execução começa, é trabalho dos [scripts Lua](#programação-em-lua) lidar com as expressões. As expressões são armazenadas em `expressions.json` na raiz do cartão SD.  
```json  
[  
  {"name": "atlas", "use_alias": "atlas", "frames": [1,1,1,1,1,2,3,4,3,2,1], "duration": 150},  
  {"name": "vergonha", "use_alias": "vergonha", "frames": [1,2,3,4], "duration": 100},  
  {"name": "owo", "use_alias": "owo", "frames": [1,2,3,4,3,2,1], "duration": 250},  
  {"name": "vergonha_transicao_in", "use_alias": "vergonha_trans", "frames":  [4,3,2,1], "duration": 100, "single": true},  
  {"name": "vergonha_transicao_out", "use_alias": "vergonha_trans", "frames": [1,2,3,4], "duration": 100, "single": true},  
  {"name": "cute", "use_alias": "cute", "frames": [1,2,3], "duration": 100},  
  {"name": "anger", "use_alias": "anger", "frames": [1,1,1,2,3,4,5,4,3,2,1,1], "duration": 100},  
  {"name": "apaixonado", "use_alias": "apaixonado", "frames": [1,2], "duration": 100},  
  {"name": "surpreso", "use_alias": "surpreso", "frames":  [1,1,1,1,1,2,3,4,5,4,3,2,1], "duration": 100},  
  {"name": "feliz", "use_alias": "feliz", "frames": [1,1,1,1,1,2,3,4,3,2,1], "duration": 150},  
  {"name": "morto", "use_alias": "morto", "frames": [1,2], "duration": 100},  
  {"name": "tela_azul", "use_alias": "tela_azul", "frames": [1], "duration": 100}  
]  
```  

Cada elemento do array é uma expressão.  

* **name** (string)  
  Não é obrigatório ter um nome, mas é uma forma de facilitar a chamada de uma animação e visualizar seu nome no menu.  

* **use_alias** (string)  
  Imagine um cenário onde você tem 200 quadros e quer criar uma expressão com os quadros 50 a 55. Para isso, você preenche o objeto `frames` com 50 a 55. Mas se você adicionar um novo quadro no ID 40, isso bagunçará sua expressão existente. Para evitar isso, você define aliases na seção `frames` e depois usa o alias para adicionar um deslocamento.  

  Por exemplo, digamos que seu alias é "angry". Usar `"use_alias": "angry"` e os quadros `[0, 1, 2, 3, 4, 5]` na verdade chamará os quadros 50 a 55 porque o alias "angry" começa no 50.  

* **frames** (array de int)  
  O ID de cada quadro a ser exibido.  

* **duration** (int)  
  A duração de cada quadro em milissegundos.  

* **single** (bool)  
  Isso força a animação a não se repetir. Uma vez que termina, ela volta para a animação anterior.  

## Pilha de Expressões  

As expressões são armazenadas em uma pilha. Quando você adiciona uma animação que não se repete, ela pausa a animação atual e executa até o final da nova animação. Se você adicionar duas ao mesmo tempo, a última será executada. Quando terminar, a anterior será executada.  

## Arquivo Bulk  

Mesmo com o cartão SD, mudar os quadros não é tão rápido. A interface do cartão SD não é rápida o suficiente. Para acelerar, as imagens são decodificadas de PNG para dados brutos de pixels no formato RGB565 e armazenadas na flash interna. Todos os quadros são armazenados em um único arquivo chamado `arquivo bulk`. Isso é feito de forma que os quadros são armazenados sequencialmente e, mantendo o arquivo aberto, a velocidade de transferência é acelerada, atingindo 60 FPS.  

Toda vez que você adiciona ou modifica um novo quadro, é necessário reconstruir esse arquivo. Isso pode ser feito no menu ou chamando a função Lua `composeBulkFile`.  

## Modo managed

As animações são processadas pelo Núcleo 1, então você não precisa perder tempo precioso nos [scripts Lua](#programação-em-lua) atualizando-as. É possível mudar o quadro usando scripts Lua... Mas também é um desperdício. Então deixe isso para o outro núcleo, e você só precisa se preocupar em selecionar quais expressões deseja!  

Durante o modo gerenciado, o desenho dos quadros é tratado pelo Núcleo 1.  
![alt text](mdoc/managed.png "Modo Gerenciado")  

# Tiras de LED  

O Protopanda suporta o protocolo de LED endereçável WS2812B e fornece um sistema simples para definir alguns comportamentos para a tira/matriz.  
![alt text](doc/A7301542.JPG)  
![alt text](doc/ewm.drawio.png)  

# Bluetooth  

O Protopanda usa BLE. Até agora, está configurado para lidar com a pata de fursuit. Todas as informações, código-fonte e hardware podem ser encontrados aqui: [https://github.com/mockthebear/ble-fursuit-paw](https://github.com/mockthebear/ble-fursuit-paw)  
![Integração BLE](doc/integration.png)  

Consiste em um dispositivo BLE com um acelerômetro/giroscópio LSM6DS3 de 3 eixos e 5 botões. Ele continua enviando as leituras dos sensores e botões a cada 50~100 ms.  

O UUID padrão da pata de fursuit BLE é `d4d31337-c4c3-c2c3-b4b3-b2b1a4a3a2a1`, e o serviço para o acelerômetro/giroscópio/botões é `d4d3afaf-c4c3-c2c3-b4b3-b2b1a4a3a2a1`.  

Se você quiser mais de um controle remoto, é recomendado reprogramar o firmware de outro controlador e definir a parte `c4c3` do UUID para `c4c4` ou algo parecido.  

Para definir ambos os dispositivos como aceitos, adicione no seu `onSetup`:  
```lua  
function onSetup()  
    startBLE()  
    acceptBLETypes("d4d31337-c4c1-c2c3-b4b3-b2b1a4a3a2a1", "afaf", "fafb")
    beginBleScanning()  
```  

Eu sei, eu sei... é estático e não tem flexibilidade para aceitar qualquer tipo de dispositivo BLE/serviços... Talvez no futuro~

# Hardware  

O Protopanda foi projetado para rodar no ESP32-S3-N16R8, uma versão com 16 MB de flash, 384 kB de ROM, 512 kB de RAM e 8 MB de PSRAM octal. É necessário essa versão com mais espaço e PSRAM para ter RAM suficiente para rodar os painéis, BLE e Lua simultaneamente.  

No hardware, há uma porta para os dados HUB75, um conector para cartão SD, dois terminais parafusados para a saída de 5V, os pinos de alimentação, uma porta I2C e um pino para tira de LED.  

## Diagrama  

![Diagrama](doc/noitegrama.png "Diagrama")  

## Portas  

![Portas](doc/ports.png "Portas")  

## Esquemático  

![Diagrama](doc/schematic.png "Diagrama")  

## Dois Núcleos  
O Protopanda utiliza e abusa dos dois núcleos do ESP32.  
* **Núcleo 0**  
Por padrão, o Núcleo 0 é projetado principalmente para gerenciar o Bluetooth. Quando não está fazendo isso, ele gerencia as animações e, quando o [Modo Gerenciado](#modo-gerenciado) está ativo, também cuida da atualização da tela de LED.  
* **Núcleo 1**  
O segundo núcleo lida com tarefas não relacionadas à tela. Ele possui a rotina que verifica o [nível de energia](#alimentação), atualiza as entradas, lê os sensores e chama a função Lua `onLoop`.  


# Programação em Lua  

__[Lua functions reference](doc/lua-doc.pt-br.md)__

- [Script Lua Mínimo](#script-lua-mínimo)  
- [Ciclar Expressões a Cada Segundo](#ciclar-expressões-a-cada-segundo)  

## Script Lua mínimo  
```lua  
-- Script Lua mínimo em init.lua  

function onSetup()  
  -- Esta função é chamada uma vez. Aqui você pode iniciar o BLE, começar a escanear, configurar o painel, definir o modo de energia, carregar bibliotecas e preparar as tiras de LED e até ligar a energia.  
  -- Todas as chamadas aqui são feitas durante o SETUP, rodando no núcleo 0.  
end  

function onPreflight()  
  -- Aqui, todas as chamadas Lua são feitas a partir do núcleo 1. Você pode até deixar esta função em branco.  
  -- O Núcleo 0 só começará a gerenciar após 100 ms (o bip final).  
end  

function onLoop(dt)  
  -- Esta função será chamada em loop.  
  -- O parâmetro dt é a diferença em MS entre o início do último quadro e o atual. Útil para armazenar tempo decorrido.  
end  
```  

## Ciclar expressões a cada segundo  
```lua  
local expressions = dofile("/lualib/expressions.lua")  
local changeExpressionTimer = 1000 -- 1 segundo  

function onSetup()  
  setPanelMaxBrightness(64)  
  panelPowerOn() -- O brilho sempre começa em 0  
  gentlySetPanelBrightness(64)  
end  

function onPreflight()  
  setPanelManaged(true)  
  expressions.Next()  
end  

function onLoop(dt)  
  changeExpressionTimer = changeExpressionTimer - dt  
  if changeExpressionTimer <= 0 then  
    changeExpressionTimer = 1000 -- 1 segundo  
    expressions.Next()  
  end  
end  
```  
