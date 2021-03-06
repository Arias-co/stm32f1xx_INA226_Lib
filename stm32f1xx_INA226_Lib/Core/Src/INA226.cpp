#include "INA226.h"
#include "math.h"

INA226::INA226( I2C_HandleTypeDef * hi2cx, uint16_t address ) // Contructor
{
    hi2c = hi2cx;
    inaAddress = address << 1;
}

bool INA226::configure( ina226_averages_t avg, ina226_busConvTime_t busConvTime,
        ina226_shuntConvTime_t shuntConvTime, ina226_mode_t mode )
{
    uint16_t config = 0;

    config |= ( avg << 9 | busConvTime << 6 | shuntConvTime << 3 | mode );

    vBusMax = 36;
    vShuntMax = 0.08192f;

    writeRegister16( INA226_REG_CONFIG, config );

    return true;
}

bool INA226::calibrate( float rShuntValue, float iMaxCurrentExcepted )
{
    uint16_t calibrationValue;
    rShunt = rShuntValue;

    float iMaxPossible, minimumLSB;

    iMaxPossible = vShuntMax / rShunt;

    minimumLSB = iMaxCurrentExcepted / 32767;

    currentLSB = (uint32_t) ( minimumLSB * 100000000 );
    currentLSB /= 100000000;
    currentLSB /= 0.0001;
    currentLSB = ceil( currentLSB );
    currentLSB *= 0.0001;

    powerLSB = currentLSB * 25;

    calibrationValue = (uint16_t) ( ( 0.00512 ) / ( currentLSB * rShunt ) );

    writeRegister16( INA226_REG_CALIBRATION, calibrationValue );

    return true;
}

double INA226::getMaxPossibleCurrent( void )
{
    return ( vShuntMax / rShunt );
}

double INA226::getMaxCurrent( void )
{
    double maxCurrent = ( currentLSB * 32767 );
    double maxPossible = getMaxPossibleCurrent();

    if ( maxCurrent > maxPossible )
    {
        return maxPossible;
    }
    else
    {
        return maxCurrent;
    }
}

double INA226::getMaxShuntVoltage( void )
{
    double maxVoltage = getMaxCurrent() * rShunt;

    if ( maxVoltage >= vShuntMax )
    {
        return vShuntMax;
    }
    else
    {
        return maxVoltage;
    }
}

double INA226::getMaxPower( void )
{
    return ( getMaxCurrent() * vBusMax );
}

double INA226::readBusPower( void )
{
    return ( readRegister16( INA226_REG_POWER ) * powerLSB );
}

double INA226::readShuntCurrent( void )
{
    return ( readRegister16( INA226_REG_CURRENT ) * currentLSB );
}

double INA226::readShuntVoltage( void )
{
    double voltage;

    voltage = readRegister16( INA226_REG_SHUNTVOLTAGE );

    return ( voltage * 0.0000025 );
}

double INA226::readBusVoltage( void )
{
    int16_t voltage;

    voltage = readRegister16( INA226_REG_BUSVOLTAGE );

    return ( voltage * 0.00125 );
}

double INA226::readResLoad( void )
{
    double i, v;
    v = INA226::readBusVoltage();
    i = INA226::readShuntCurrent();
    return v / i;
}

ina226_averages_t INA226::getAverages( void )
{
    uint16_t value;

    value = readRegister16( INA226_REG_CONFIG );
    value &= 0b0000111000000000;
    value >>= 9;

    return (ina226_averages_t) value;
}

ina226_busConvTime_t INA226::getBusConversionTime( void )
{
    uint16_t value;

    value = readRegister16( INA226_REG_CONFIG );
    value &= 0b0000000111000000;
    value >>= 6;

    return (ina226_busConvTime_t) value;
}

ina226_shuntConvTime_t INA226::getShuntConversionTime( void )
{
    uint16_t value;

    value = readRegister16( INA226_REG_CONFIG );
    value &= 0b0000000000111000;
    value >>= 3;

    return (ina226_shuntConvTime_t) value;
}

ina226_mode_t INA226::getMode( void )
{
    uint16_t value;

    value = readRegister16( INA226_REG_CONFIG );
    value &= 0b0000000000000111;

    return (ina226_mode_t) value;
}

void INA226::setMaskEnable( uint16_t mask )
{
    writeRegister16( INA226_REG_MASKENABLE, mask );
}

uint16_t INA226::getMaskEnable( void )
{
    return readRegister16( INA226_REG_MASKENABLE );
}

void INA226::enableShuntOverLimitAlert( void )
{
    writeRegister16( INA226_REG_MASKENABLE, INA226_BIT_SOL );
}

void INA226::enableShuntUnderLimitAlert( void )
{
    writeRegister16( INA226_REG_MASKENABLE, INA226_BIT_SUL );
}

void INA226::enableBusOvertLimitAlert( void )
{
    writeRegister16( INA226_REG_MASKENABLE, INA226_BIT_BOL );
}

void INA226::enableBusUnderLimitAlert( void )
{
    writeRegister16( INA226_REG_MASKENABLE, INA226_BIT_BUL );
}

void INA226::enableOverPowerLimitAlert( void )
{
    writeRegister16( INA226_REG_MASKENABLE, INA226_BIT_POL );
}

void INA226::enableConversionReadyAlert( void )
{
    writeRegister16( INA226_REG_MASKENABLE, INA226_BIT_CNVR );
}

void INA226::setBusVoltageLimit( float voltage )
{
    uint16_t value = voltage / 0.00125;
    writeRegister16( INA226_REG_ALERTLIMIT, value );
}

void INA226::setShuntVoltageLimit( float voltage )
{
    uint16_t value = voltage / 0.0000025;
    writeRegister16( INA226_REG_ALERTLIMIT, value );
}

void INA226::setPowerLimit( float watts )
{
    uint16_t value = watts / powerLSB;
    writeRegister16( INA226_REG_ALERTLIMIT, value );
}

void INA226::setAlertInvertedPolarity( bool inverted )
{
    uint16_t temp = getMaskEnable();

    if ( inverted )
    {
        temp |= INA226_BIT_APOL;
    }
    else
    {
        temp &= ~INA226_BIT_APOL;
    }

    setMaskEnable( temp );
}

void INA226::setAlertLatch( bool latch )
{
    uint16_t temp = getMaskEnable();

    if ( latch )
    {
        temp |= INA226_BIT_LEN;
    }
    else
    {
        temp &= ~INA226_BIT_LEN;
    }

    setMaskEnable( temp );
}

bool INA226::isMathOverflow( void )
{
    return ( ( getMaskEnable() & INA226_BIT_OVF ) == INA226_BIT_OVF );
}

bool INA226::isAlert( void )
{
    return ( ( getMaskEnable() & INA226_BIT_AFF ) == INA226_BIT_AFF );
}

int16_t INA226::readRegister16( uint8_t reg )
{
    int16_t value;
    uint8_t pData[2];

    HAL_I2C_Master_Transmit( hi2c, inaAddress, &reg, 1, HAL_MAX_DELAY );
    HAL_I2C_Master_Receive( hi2c, inaAddress, pData, 2, HAL_MAX_DELAY );

    value = pData[0] << 8 | pData[1];

    return value;
}

void INA226::writeRegister16( uint8_t reg, uint16_t val )
{
    uint8_t vla;
    vla = (uint8_t) val;
    val >>= 8;
    uint8_t pData[3];

    pData[0] = reg;
    pData[1] = (uint8_t) val;
    pData[2] = vla;
    HAL_I2C_Master_Transmit( hi2c, inaAddress, pData, 3, HAL_MAX_DELAY );

}
