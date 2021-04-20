# hydroponic-wemos

## hardware info

- [Wemos D1 Mini Layout](https://www.wemos.cc/en/latest/d1/d1_mini.html)
    - version 1: add base layout

## hardware layout

![hardware layout](layout_v1.png)

## additional hardware

- 12V power supply
- 12V-5V voltage regulator
- 12V water pump
- Multiplexer PCF8574
- SSD1306 OLED Screen 128x32px
- 4 channel relay breakout board
- water resistant temperature sensor DS18B20

## oled layout

The used OLED uses 128x32px in landscape orientation. For easier management of pixelart, the display is logically divided in 8x2 squares with 16x16px each and used for the following purposes:

| **X**     | **Column 1** | **Column 2**  | **Column 3**      | **Column 4**           | **Column 5**    | **Column 6** |  **Column 7**    | **Column 8**          |
|-----------|:------------:|:-------------:|:-----------------:|:----------------------:|:---------------:|:------------:|:----------------:|:---------------------:|
| **Row 1** | temp icon    | empty         | temp value pipe 1 | temp value pipe 1 unit | pipe state icon | empty        | pipe 1 countdown | pipe 1 countdown unit |
| **Row 2** | temp icon    | empty         | temp value pipe 2 | temp value pipe 2 unit | pipe state icon | empty        | pipe 2 countdown | pipe 2 countdown unit |

Icons are based on the wonderful [Ionicon collection](https://ionicons.com/).

If you want to convert your icons you can use this awesome page: [image2cpp](https://javl.github.io/image2cpp/)