#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

// Nurodome, kad naudosime I2C sąsają
#define SSD1306_USE_I2C

// Nurodome jūsų CubeMX sugeneruotą I2C1 sąsajos pavadinimą
#define SSD1306_I2C_PORT        hi2c1

// Standartinis OLED ekrano adresas
#define SSD1306_I2C_ADDR        (0x3C << 1)

// ĮJUNGIAME REIKALINGUS ŠRIFTUS (pridėtos šios dvi eilutės!)
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18

#endif /* __SSD1306_CONF_H__ */
