#include "stdio.h"
#include "main.h"
#include "i2c.h"
#include "setup.h"
#include "control.h"
#include "ds18b20.h"						
#include "ssd1306.h"

// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Screen object
static SSD1306_t SSD1306;


//
//  Send a byte to the command register
//
static uint8_t ssd1306_WriteCommand(I2C_HandleTypeDef *hi2c, uint8_t command)
{
    return HAL_I2C_Mem_Write(hi2c, SSD1306_I2C_ADDR, 0x00, 1, &command, 1, 10);
}


//
//  Initialize the oled screen
//
uint8_t ssd1306_Init(I2C_HandleTypeDef *hi2c)
{
    // Wait for the screen to boot
    HAL_Delay(100);
    int status = 0;

    // Init LCD
    status += ssd1306_WriteCommand(hi2c, 0xAE);   // Display off
    status += ssd1306_WriteCommand(hi2c, 0x20);   // Set Memory Addressing Mode
    status += ssd1306_WriteCommand(hi2c, 0x10);   // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    status += ssd1306_WriteCommand(hi2c, 0xB0);   // Set Page Start Address for Page Addressing Mode,0-7
    status += ssd1306_WriteCommand(hi2c, 0xC8);   // Set COM Output Scan Direction
    status += ssd1306_WriteCommand(hi2c, 0x00);   // Set low column address
    status += ssd1306_WriteCommand(hi2c, 0x10);   // Set high column address
    status += ssd1306_WriteCommand(hi2c, 0x40);   // Set start line address
    status += ssd1306_WriteCommand(hi2c, 0x81);   // set contrast control register
    status += ssd1306_WriteCommand(hi2c, 0xFF);
    status += ssd1306_WriteCommand(hi2c, 0xA1);   // Set segment re-map 0 to 127
    status += ssd1306_WriteCommand(hi2c, 0xA6);   // Set normal display

    status += ssd1306_WriteCommand(hi2c, 0xA8);   // Set multiplex ratio(1 to 64)
    status += ssd1306_WriteCommand(hi2c, SSD1306_HEIGHT - 1);

    status += ssd1306_WriteCommand(hi2c, 0xA4);   // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    status += ssd1306_WriteCommand(hi2c, 0xD3);   // Set display offset
    status += ssd1306_WriteCommand(hi2c, 0x00);   // No offset
    status += ssd1306_WriteCommand(hi2c, 0xD5);   // Set display clock divide ratio/oscillator frequency
    status += ssd1306_WriteCommand(hi2c, 0xF0);   // Set divide ratio
    status += ssd1306_WriteCommand(hi2c, 0xD9);   // Set pre-charge period
    status += ssd1306_WriteCommand(hi2c, 0x22);

    status += ssd1306_WriteCommand(hi2c, 0xDA);   // Set com pins hardware configuration
#ifdef SSD1306_COM_LR_REMAP
    status += ssd1306_WriteCommand(hi2c, 0x32);   // Enable COM left/right remap
#else
    status += ssd1306_WriteCommand(hi2c, 0x12);   // Do not use COM left/right remap
#endif // SSD1306_COM_LR_REMAP

    status += ssd1306_WriteCommand(hi2c, 0xDB);   // Set vcomh
    status += ssd1306_WriteCommand(hi2c, 0x20);   // 0x20,0.77xVcc
    status += ssd1306_WriteCommand(hi2c, 0x8D);   // Set DC-DC enable
    status += ssd1306_WriteCommand(hi2c, 0x14);   //
    status += ssd1306_WriteCommand(hi2c, 0xAF);   // Turn on SSD1306 panel

    if (status != 0) {
        return 1;
    }

    // Clear screen
    ssd1306_Fill(Black);

    // Flush buffer to screen
    ssd1306_UpdateScreen(hi2c);

    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    SSD1306.Initialized = 1;

    return 0;
}

//
//  Fill the whole screen with the given color
//
void ssd1306_Fill(SSD1306_COLOR color)
{
    // Fill screenbuffer with a constant value (color)
    uint32_t i;

    for(i = 0; i < sizeof(SSD1306_Buffer); i++)
    {
        SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

//
//  Write the screenbuffer with changed to the screen
//
void ssd1306_UpdateScreen(I2C_HandleTypeDef *hi2c)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        ssd1306_WriteCommand(hi2c, 0xB0 + i);
        ssd1306_WriteCommand(hi2c, 0x00);
        ssd1306_WriteCommand(hi2c, 0x10);

        HAL_I2C_Mem_Write(hi2c, SSD1306_I2C_ADDR, 0x40, 1, &SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH, 100);
    }
}

//
//  Draw one pixel in the screenbuffer
//  X => X Coordinate
//  Y => Y Coordinate
//  color => Pixel color
//
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    {
        // Don't write outside the buffer
        return;
    }

    // Check if pixel should be inverted
    if (SSD1306.Inverted)
    {
        color = (SSD1306_COLOR)!color;
    }

    // Draw in the correct color
    if (color == White)
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    }
    else
    {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}


//
//  Draw 1 char to the screen buffer
//  ch      => Character to write
//  Font    => Font to use
//  color   => Black or White
//
char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
{
    uint32_t i, b, j;

    // Check remaining space on current line
    if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font.FontWidth) ||
        SSD1306_HEIGHT <= (SSD1306.CurrentY + Font.FontHeight))
    {
        // Not enough space on current line
        return 0;
    }

    // Translate font to screenbuffer
    for (i = 0; i < Font.FontHeight; i++)
    {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++)
        {
            if ((b << j) & 0x8000)
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
            }
            else
            {
                ssd1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }

    // The current space is now taken
    SSD1306.CurrentX += Font.FontWidth;

    // Return written char for validation
    return ch;
}

//
//  Write full string to screenbuffer
//
char ssd1306_WriteString(char* str, FontDef Font, SSD1306_COLOR color)
{
    // Write until null-byte
    while (*str)
    {
        if (ssd1306_WriteChar(*str, Font, color) != *str)
        {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

//
//  Invert background/foreground colors
//
void ssd1306_InvertColors(void)
{
    SSD1306.Inverted = !SSD1306.Inverted;
}

//
//  Set cursor position
//
void ssd1306_SetCursor(uint8_t x, uint8_t y)
{
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}
//=========================================================================
u16 aver_vdev(float arg)
{
	u16 tmp10 = (u16)(arg*10.0f);
	u16 tmp100 = (u16)(arg*100.0f);
	u16 tmp1000 = (u16)(arg*1000.0f);
	u16 tmp = tmp1000 - (tmp100*10);

	if(tmp>=5) tmp100++;
	tmp = tmp100 - (tmp10*10);
	if(tmp>=5) tmp10++;

	return tmp10;
}
//===================================================================
void ssd1306_SetPresent(void)
{
	char buffer[16];
    ssd1306_SetCursor(31, 0);
    ssd1306_WriteString("Term 4", Font_11x18, White);
    ssd1306_SetCursor(32, 16);
    ssd1306_WriteString("kvim", Font_16x26, White);
    ssd1306_SetCursor(25, 40);
    sprintf(buffer, "SN: %d", Setup.SerialId);
    ssd1306_WriteString(buffer, Font_11x18, White);
    
    ssd1306_UpdateScreen(&hi2c1);
}
//===================================================================
uint8_t shift_dig(float dig)
{
    uint8_t shift;
    
    if(dig < 0.0f) shift = 0;
    else if(dig < 10.0f) shift = 22;
    else shift = 11;
    
    return shift;
}
//===================================================================
void set_humd_poz(u16 poz)
{
	char buffer[16];
    uint8_t shift;
    
    switch( poz )
    {
    case 0:	
        break;
    case 1:	
        ssd1306_SetCursor(11, 20);
        sprintf(buffer, "H:%d %.1f", dev_var.shtc3_hum, dev_var.term_real[1]);
        ssd1306_WriteString(buffer, Font_11x18, White);
    //---------------------------------------------------------------------------
        shift = shift_dig(dev_var.term_real[2]);
        ssd1306_SetCursor(shift, 44);
        sprintf(buffer, "%.1f %.1f", dev_var.term_real[2], dev_var.term_real[3]);
        ssd1306_WriteString(buffer, Font_11x18, White);
        break;
    case 2:	
        shift = shift_dig(dev_var.term_real[0]);
        ssd1306_SetCursor(shift, 20);
        sprintf(buffer, "%.1f H:%d", dev_var.term_real[0], dev_var.shtc3_hum);
        ssd1306_WriteString(buffer, Font_11x18, White);
    //---------------------------------------------------------------------------
        shift = shift_dig(dev_var.term_real[2]);
        ssd1306_SetCursor(shift, 44);
        sprintf(buffer, "%.1f %.1f", dev_var.term_real[2], dev_var.term_real[3]);
        ssd1306_WriteString(buffer, Font_11x18, White);
        break;
    case 3:	
        shift = shift_dig(dev_var.term_real[0]);
        ssd1306_SetCursor(shift, 20);
        sprintf(buffer, "%.1f %.1f", dev_var.term_real[0], dev_var.term_real[1]);
        ssd1306_WriteString(buffer, Font_11x18, White);
    //---------------------------------------------------------------------------
        ssd1306_SetCursor(11, 44);
        sprintf(buffer, "H:%d %.1f", dev_var.shtc3_hum, dev_var.term_real[3]);
        ssd1306_WriteString(buffer, Font_11x18, White);
        break;
    case 4:	
        shift = shift_dig(dev_var.term_real[0]);
        ssd1306_SetCursor(shift, 20);
        sprintf(buffer, "%.1f %.1f", dev_var.term_real[0], dev_var.term_real[1]);
        ssd1306_WriteString(buffer, Font_11x18, White);
    //---------------------------------------------------------------------------
        shift = shift_dig(dev_var.term_real[2]);
        ssd1306_SetCursor(shift, 44);
        sprintf(buffer, "%.1f H:%d", dev_var.term_real[2], dev_var.shtc3_hum);
        ssd1306_WriteString(buffer, Font_11x18, White);
        break;
        default: ;
    }
}
//===================================================================
void ssd1306_Proc(void)
{
	char buffer[16];
	static Uint32 timeout_type = 0;
    
	if(timeout_type+500 <= HAL_GetTick())
	{
        timeout_type = HAL_GetTick();
    
        ssd1306_Fill(Black);
        
        ssd1306_SetCursor(0, 0);
        sprintf(buffer, "SN:%d nt:%d RH:%d", Setup.SerialId, n_datch, dev_var.shtc3_hum);
        ssd1306_WriteString(buffer, Font_7x10, White);
        
        uint8_t shift;
        if(n_datch==1)
        {
            if(Setup.humd)
            {
                
                shift = shift_dig(dev_var.term_real[0]);
                ssd1306_SetCursor(shift+8, 20);
                sprintf(buffer, "T: %.1f", dev_var.term_real[0]);
                ssd1306_WriteString(buffer, Font_11x18, White);

                ssd1306_SetCursor(20, 44);
                sprintf(buffer, "RH:%d", dev_var.shtc3_hum);
                ssd1306_WriteString(buffer, Font_11x18, White);
            }
            else
            {
                shift = shift_dig(dev_var.term_real[0]);
                ssd1306_SetCursor(shift+14, 26);
                sprintf(buffer, "%.1f", dev_var.term_real[0]);
                ssd1306_WriteString(buffer, Font_16x26, White);
            }
        }
        else if(n_datch==2)
        {
            shift = shift_dig(dev_var.term_real[0]);
            ssd1306_SetCursor(shift, 20);
            sprintf(buffer, "%.1f %.1f", dev_var.term_real[0], dev_var.term_real[1]);
            ssd1306_WriteString(buffer, Font_11x18, White);
            if(Setup.humd)
            {
                ssd1306_SetCursor(25, 44);
                sprintf(buffer, "RH: %d", dev_var.shtc3_hum);
                ssd1306_WriteString(buffer, Font_11x18, White);
            }
        }
        else if(n_datch==3)
        {
            shift = shift_dig(dev_var.term_real[0]);
            ssd1306_SetCursor(shift, 20);
            sprintf(buffer, "%.1f %.1f", dev_var.term_real[0], dev_var.term_real[1]);
            ssd1306_WriteString(buffer, Font_11x18, White);
            if(Setup.humd)
            {
                shift = shift_dig(dev_var.term_real[2]);
                ssd1306_SetCursor(shift, 44);
                sprintf(buffer, "%.1f RH:%d", dev_var.term_real[2], dev_var.shtc3_hum);
                ssd1306_WriteString(buffer, Font_11x18, White);
            }
            else
            {
                shift = shift_dig(dev_var.term_real[2]);
                ssd1306_SetCursor(shift, 44);
                sprintf(buffer, "%.1f ", dev_var.term_real[2]);
                ssd1306_WriteString(buffer, Font_11x18, White);
            }
        }
        else if(n_datch==4)
        {
            if(Setup.humd) set_humd_poz(Setup.hum_poz);
            else
            {
                shift = shift_dig(dev_var.term_real[0]);
                ssd1306_SetCursor(shift, 20);
                sprintf(buffer, "%.1f %.1f", dev_var.term_real[0], dev_var.term_real[1]);
                ssd1306_WriteString(buffer, Font_11x18, White);
                shift = shift_dig(dev_var.term_real[2]);
                ssd1306_SetCursor(shift, 44);
                sprintf(buffer, "%.1f %.1f", dev_var.term_real[2], dev_var.term_real[3]);
                ssd1306_WriteString(buffer, Font_11x18, White);
            }
        }
        ssd1306_UpdateScreen(&hi2c1);
    }
    
}
//===================================================================

