#include "ssd2119.h"
#include "system.h"
#include <memory>


#define ENTRY_MODE_DEFAULT 0x6830
#define ENTRY_MODE_BMP 	   0x6810
#define MAKE_ENTRY_MODE(x) ((ENTRY_MODE_DEFAULT & 0xFF00) | (x))


SSD2119::SSD2119(Pin rst, Pin backlight) :
	Display(SSD2119::LCD_HEIGHT, SSD2119::LCD_WIDTH), m_rst(rst), m_backlight(backlight)
{
}

bool SSD2119::init(Orientation orientation)
{
    if(initGpio() || initFSMC())
        return true;

    reset();

	m_backlight.setHigh();
    System::delay(100);

    auto v = readReg(SSD2119::DEVICE_CODE_READ);

    /* Enter sleep mode (if we are not already there).*/
    writeReg(SSD2119::SLEEP_MODE_1,     0x0001);

    /* Set initial power parameters. */
    writeReg(SSD2119::PWR_CTRL_5,       0x00B2);
    writeReg(SSD2119::VCOM_OTP_1,       0x0006);
 
    /* Start the oscillator.*/
    writeReg(SSD2119::OSC_START,        0x0001);

    /* Set pixel format and basic display orientation (scanning direction).*/
    writeReg(SSD2119::OUTPUT_CTRL,       0x30EF);
    writeReg(SSD2119::LCD_DRIVE_AC_CTRL, 0x0600);

    /* Exit sleep mode.*/
    writeReg(SSD2119::SLEEP_MODE_1,     0x0000);
    System::delay(5);
	  
    /* Configure pixel color format and MCU interface parameters.*/
    writeReg(SSD2119::ENTRY_MODE,       ENTRY_MODE_DEFAULT);

    /* Set analog parameters */
    writeReg(SSD2119::SLEEP_MODE_2,     0x0999);
    writeReg(SSD2119::ANALOG_SET,       0x3800);

    /* Enable the display */
    writeReg(SSD2119::DISPLAY_CTRL,     0x0033);

    /* Set VCIX2 voltage to 6.1V.*/
    writeReg(SSD2119::PWR_CTRL_2,       0x0005);

    /* Configure gamma correction.*/
    writeReg(SSD2119::GAMMA_CTRL_1,     0x0000);
    writeReg(SSD2119::GAMMA_CTRL_2,     0x0303);
    writeReg(SSD2119::GAMMA_CTRL_3,     0x0407);
    writeReg(SSD2119::GAMMA_CTRL_4,     0x0301);
    writeReg(SSD2119::GAMMA_CTRL_5,     0x0301);
    writeReg(SSD2119::GAMMA_CTRL_6,     0x0403);
    writeReg(SSD2119::GAMMA_CTRL_7,     0x0707);
    writeReg(SSD2119::GAMMA_CTRL_8,     0x0400);
    writeReg(SSD2119::GAMMA_CTRL_9,     0x0a00);
    writeReg(SSD2119::GAMMA_CTRL_10,    0x1000);

    /* Configure Vlcd63 and VCOMl */
    writeReg(SSD2119::PWR_CTRL_3,       0x000A);
    writeReg(SSD2119::PWR_CTRL_4,       0x2E00);

    /* Set the display size and ensure that the GRAM window is set to allow
        access to the full display buffer.*/
    writeReg(SSD2119::V_RAM_POS,    (LCD_HEIGHT-1) << 8);
    writeReg(SSD2119::H_RAM_START,  0x0000);
    writeReg(SSD2119::H_RAM_END,    LCD_WIDTH-1);

    writeReg(SSD2119::X_RAM_ADDR,   0x00);
    writeReg(SSD2119::Y_RAM_ADDR,   0x00);

    writeReg(SSD2119::RAM_DATA,     0x0000);
    for(uint32_t ulCount = 0; ulCount < (LCD_WIDTH * LCD_HEIGHT); ulCount++)
        writeRam(0x0000);

	setOrientation(orientation);

    return false;
}

void SSD2119::writePixel(int16_t x, int16_t y, uint16_t color)
{
	//Point2D p = orient(Point2D(x, y));	

    if( orientation() == Orientation::PORTRAIT_1 ||
        orientation() == Orientation::PORTRAIT_2)
        std::swap(x, y);

	writeReg(SSD2119::X_RAM_ADDR, x);
	writeReg(SSD2119::Y_RAM_ADDR, y);

	*cmdAdr = SSD2119::RAM_DATA;
	*baseAdr = color;
}

void SSD2119::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if( w <= 0 ||
        h <= 0)
        return;

    volatile uint16_t px = x;
    volatile uint16_t py = y;
    volatile uint16_t ph = h;
    volatile uint16_t pw = w;

    volatile uint16_t c = color;

    if( orientation() == Orientation::PORTRAIT_1 ||
        orientation() == Orientation::PORTRAIT_2)
    {
        std::swap(px, py);
        std::swap(ph, pw);
    }

	writeReg(SSD2119::X_RAM_ADDR, px);


    for (uint32_t i = ph; i != 0; --i)
	{
		writeReg(SSD2119::Y_RAM_ADDR, py++);
		writeCmd(SSD2119::RAM_DATA);

		for(uint32_t j=pw; j!=0 ; --j)
			writeRam(color);
	}
}

uint16_t SSD2119::readReg(uint16_t reg)
{
	*cmdAdr = reg;
	return (uint16_t)(*baseAdr);
}



void SSD2119::reset()
{
    if(m_rst)
    {
        m_rst.setLow();
        System::delay(10);
        m_rst.setHigh();
    }
}

void SSD2119::setOrientation(Orientation orientation)
{
    constexpr uint16_t RL   = (1<<14);
    constexpr uint16_t REV  = (1<<13);
    constexpr uint16_t GD   = (1<<12);
    constexpr uint16_t BGR  = (1<<11);
    constexpr uint16_t SM   = (1<<10);
    constexpr uint16_t TB   = (1<< 9);
    constexpr uint16_t MUX  = 0xEF;

    uint16_t reg = GD | REV | /*BGR |*/ MUX;

    switch(orientation)
    {
        case Orientation::LANDSCAPE_1:
            reg |= (RL | TB);
            break;
        case Orientation::PORTRAIT_2:
            reg |= (RL);
            break;
        case Orientation::PORTRAIT_1:
            reg |= (TB);
            break;
        default:
            break;
    };

    writeReg(SSD2119::OUTPUT_CTRL, reg);

    Display::setOrientation(orientation);
}

bool SSD2119::initFSMC()
{
    __HAL_RCC_FSMC_CLK_ENABLE();

    m_ram.Instance  = FSMC_NORSRAM_DEVICE;
    m_ram.Extended  = FSMC_NORSRAM_EXTENDED_DEVICE;
	
	m_ram.Init = {};
    m_ram.Init.NSBank             = FSMC_NORSRAM_BANK1;
    m_ram.Init.DataAddressMux     = FSMC_DATA_ADDRESS_MUX_DISABLE;
    m_ram.Init.MemoryType         = FSMC_MEMORY_TYPE_SRAM;
    m_ram.Init.MemoryDataWidth    = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    m_ram.Init.BurstAccessMode    = FSMC_BURST_ACCESS_MODE_DISABLE;
    m_ram.Init.AsynchronousWait   = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    m_ram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    m_ram.Init.WrapMode           = FSMC_WRAP_MODE_DISABLE;
    m_ram.Init.WaitSignalActive   = FSMC_WAIT_TIMING_BEFORE_WS;
    m_ram.Init.WriteOperation     = FSMC_WRITE_OPERATION_ENABLE;
    m_ram.Init.WaitSignal         = FSMC_WAIT_SIGNAL_DISABLE;
    m_ram.Init.ExtendedMode       = FSMC_EXTENDED_MODE_DISABLE;
    m_ram.Init.WriteBurst         = FSMC_WRITE_BURST_DISABLE;

    FSMC_NORSRAM_TimingTypeDef timing;
    timing.AddressSetupTime       = 1;
    timing.AddressHoldTime        = 0;
    timing.DataSetupTime          = 9;
    timing.BusTurnAroundDuration  = 0;
    timing.CLKDivision            = 0;
    timing.DataLatency            = 0;
    timing.AccessMode             = FSMC_ACCESS_MODE_A;

    if (HAL_SRAM_Init(&m_ram, &timing, &timing) != HAL_OK)
        return true;

    __FMC_NORSRAM_ENABLE(FSMC_NORSRAM_DEVICE, FSMC_NORSRAM_BANK1);
    HAL_SRAM_WriteOperation_Enable(&m_ram);

    return false;
}

bool SSD2119::initGpio()
{
    System::enableGPIOPortClock(GPIOB);
    System::enableGPIOPortClock(GPIOD);
    System::enableGPIOPortClock(GPIOE);
    System::enableGPIOPortClock(GPIOF);

    GPIO_InitTypeDef gpio;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_HIGH;
    gpio.Alternate = GPIO_AF12_FSMC;
  
    gpio.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 |
                 GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &gpio);

    gpio.Pin   = GPIO_PIN_3 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                 GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio);

    if(m_rst)
        m_rst.init(GPIO_MODE_OUTPUT_PP);
    
    if(m_backlight)
        m_backlight.init(GPIO_MODE_OUTPUT_PP);

    return false;
}

bool SSD2119::drawBmpFromFile(File& file, int32_t x, int32_t y, bool invert)
{
    /* Outside the screen */
    if(x >= m_size.x() ||
       y >= m_size.y())
       return false;

    /* Too small to be a BMP file */
    if(file.size() < 54)
        return true;

    ByteArray buff;
    
    if(file.rewind())
        return true;

    /* Read header and DIB */
    /* See Wikipedia for BMP format details */
    if(file.read(buff, 54) || buff.size() != 54)
        return true;

    /* Check magic constant */
    if(! (  buff[0] == 0x42  &&
            buff[1] == 0x4D))
        return true;

    /* Pixel array offset field */
    uint32_t pixelArrayOffset =   (uint32_t)(buff[0x0A])        | ((uint32_t)(buff[0x0B])<<8) |
                                 ((uint32_t)(buff[0x0C])<<16)   | ((uint32_t)(buff[0x0D])<<24);
    
    /* Offset is beyond EOF */
    if(pixelArrayOffset >= file.size() || file.seek(pixelArrayOffset))
        return true;


    /* Check color depth (only 16bits is accepted) */
    uint16_t colorDepth = (uint16_t)(buff[0x1C]) | ((uint16_t)(buff[0x1D])<<8);
    if(colorDepth != 16)
        return true;

    uint32_t width =         (uint32_t)(buff[0x12])         | ((uint32_t)(buff[0x13])<<8) |
                            ((uint32_t)(buff[0x14])<<16)    | ((uint32_t)(buff[0x15])<<24);

    uint32_t height=         (uint32_t)(buff[0x16])         | ((uint32_t)(buff[0x17])<<8) |
                            ((uint32_t)(buff[0x18])<<16)    | ((uint32_t)(buff[0x19])<<24);

    /* Opposite corner coordinates */
    int32_t x2 = x+width-1;
    int32_t y2 = y+height-1;

    /* Outside the screen */
    if(x2 <= 0 || y2 <= 0)
        return false;

    uint32_t rh = height;
    uint32_t rw = width;

    /* Clip for partial image */
    if(y2 >= m_size.y())
        rh = m_size.y() - y;

    if(x2 >= m_size.x())
        rw = m_size.x() - x;

    if(x < 0)
        rw = width + x;

    const uint32_t rowSize = width*sizeof(uint16_t) + (width%2)*sizeof(uint16_t);
    
    if(y < 0 && !invert)
        if(file.seek(pixelArrayOffset + (-y)*rowSize))
            return true;

    if(invert && height > rh)
        if(file.seek(pixelArrayOffset + (height - rh)*rowSize))
            return true;

    uint32_t zx = (x>0?x:0);
    uint32_t zy = (x>0?x:0);

    bool error = false;

    if(orientation() == Orientation::PORTRAIT_1 || orientation() == Orientation::PORTRAIT_2)
    {
        for(uint32_t line = (y>0)?0:(-y); line < rh; ++line)
        {
            if(file.read(buff, rowSize) || buff.size() != rowSize)
            {
                error = true;
                break;
            }

            if(invert)
                writeReg(SSD2119::X_RAM_ADDR, zx + (rh - line));
            else
                writeReg(SSD2119::X_RAM_ADDR, zx + line);

            for(uint32_t i=0; i<rw; ++i)
            {
                writeReg(SSD2119::Y_RAM_ADDR, zy+i);

                *cmdAdr = SSD2119::RAM_DATA;
                *baseAdr = /*(uint16_t)(buff[i*2]) << 8 |  (uint16_t)(buff[i*2+1])*/*(uint16_t*)(buff.internalBuffer()+(i*2));
            }
        }
    }
    else
    {
        writeReg(SSD2119::X_RAM_ADDR, zx);

        for(uint32_t line = (y>0)?0:(-y); line < rh; ++line)
        {
            if(file.read(buff, rowSize) || buff.size() != rowSize)
            {
                error = true;
                break;
            }

            if(invert)
                writeReg(SSD2119::Y_RAM_ADDR, zy + (rh - line));
            else
                writeReg(SSD2119::Y_RAM_ADDR, zy + line);

		    *cmdAdr = SSD2119::RAM_DATA;

            for(uint32_t i=0; i<rw*2; i += 2)
                *baseAdr = /*(uint16_t)(buff[i]) << 8 |  (uint16_t)(buff[i+1])*/*(uint16_t*)(buff.internalBuffer()+i);
        }

    }

    return error;
}
