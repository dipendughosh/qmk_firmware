# Enable the OLED feature
OLED_ENABLE = yes

# Set the OLED driver (SSD1306 is the most common)
OLED_DRIVER = ssd1306

# Enable WPM to display your typing speed
WPM_ENABLE = yes

# Define the I2C pins for the OLED. These are the correct pins for a Pro Micro.
OLED_I2C_SDA_PIN = D1
OLED_I2C_SCL_PIN = D0

# Optional: Set the display size and a timeout
OLED_DISPLAY_128X64 = yes
OLED_TIMEOUT = 10000 # 10 seconds
