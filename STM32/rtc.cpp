#include "rtc.h"

#if defined(HAL_RTC_MODULE_ENABLED)

#include "system.h"
#include <cstdio>

#define LSI_FREQUENCY ((uint32_t)(32768))

RealTimeClock rtc;

#if defined(STM32F4xx)
uint8_t* const RealTimeClock::BSRAM = reinterpret_cast<uint8_t*>(BKPSRAM_BASE);
#endif

RealTimeClock::RealTimeClock()
{ }

bool RealTimeClock::init(bool useExternalOscillator)
{
    if(useExternalOscillator)
    {
        __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);
        while(__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET);
    }
    else
    {
        __HAL_RCC_LSI_ENABLE();
        while(__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == RESET);
    }
    
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    __HAL_RCC_BACKUPRESET_FORCE();
    __HAL_RCC_BACKUPRESET_RELEASE();

    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {};
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    __HAL_RCC_RTC_CONFIG(useExternalOscillator?RCC_RTCCLKSOURCE_LSE:RCC_RTCCLKSOURCE_LSI);

    __HAL_RCC_RTC_ENABLE();

#if defined(STM32F4xx)
    __HAL_RCC_BKPSRAM_CLK_ENABLE();
    HAL_PWREx_EnableBkUpReg();
#endif

    m_rtc.Instance = RTC;

#if defined(STM32F4xx)
    m_rtc.Init.HourFormat       = RTC_HOURFORMAT_24;
    m_rtc.Init.OutPut           = RTC_OUTPUT_DISABLE;
    m_rtc.Init.OutPutPolarity   = RTC_OUTPUT_POLARITY_HIGH;
    m_rtc.Init.OutPutType       = RTC_OUTPUT_TYPE_OPENDRAIN;

    if(useExternalOscillator)
    {
        m_rtc.Init.AsynchPrediv = 128;
        m_rtc.Init.SynchPrediv  = 256;
    }
    else
    {
        m_rtc.Init.AsynchPrediv = LSI_FREQUENCY/254 + 1;
        m_rtc.Init.SynchPrediv  = 50*LSI_FREQUENCY/(m_rtc.Init.AsynchPrediv) - 1;
    }
#else
    m_rtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
    m_rtc.Init.OutPut       = RTC_OUTPUTSOURCE_NONE;
#endif

    if(HAL_RTC_Init(&m_rtc) != HAL_OK)
        return true;

 //   HAL_RTCEx_EnableBypassShadow(&m_rtc);

    HAL_RTC_WaitForSynchro(&m_rtc);

    return false;
}

#if !defined(STM32F1xx)
bool RealTimeClock::configureWakeUpTimer(uint32_t us, void (*wakeupCallback) (void))
{
    __HAL_RTC_WAKEUPTIMER_DISABLE(&m_rtc);
	__HAL_RTC_WAKEUPTIMER_DISABLE_IT(&m_rtc, RTC_IT_WUT);
	__HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();
	__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&m_rtc, RTC_FLAG_WUTF);
	
    if(us >= (uint64_t)(1<<16) / (uint64_t)(LSI_FREQUENCY/16) * 1000000LU)
        return true;

    if(us != 0)
    {
        uint32_t div;
        uint32_t cnt;

        if(us >= 2000000*8)
        {
            div = RTC_WAKEUPCLOCK_RTCCLK_DIV16;
            cnt = (uint64_t)(us)*((uint64_t)(LSI_FREQUENCY)/16LU)/1000000LU;
        }
        else if(us >= 2000000*4)
        {
            div = RTC_WAKEUPCLOCK_RTCCLK_DIV8;
            cnt = (uint64_t)(us)*((uint64_t)(LSI_FREQUENCY)/8LU)/1000000LU;
        }
        else if(us >= 2000000*2)
        {
            div = RTC_WAKEUPCLOCK_RTCCLK_DIV4;
            cnt = (uint64_t)(us)*((uint64_t)(LSI_FREQUENCY)/4LU)/1000000LU;
        }
        else
        {
            div = RTC_WAKEUPCLOCK_RTCCLK_DIV2;
            cnt = (uint64_t)(us)*((uint64_t)(LSI_FREQUENCY)/2LU)/1000000LU;
        }

		if(cnt == 0)
			return true;

		if(cnt > 0xFFFF) /* That shouldn't happen... */
			cnt = 0xFFFF;

	    HAL_RTCEx_SetWakeUpTimer_IT(&m_rtc, (uint16_t)(cnt), div);
		
	    HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 4, 0);
	    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
    }

    m_wakeupCallback = wakeupCallback;

    return false;
}
#endif

DateTime RealTimeClock::getDateTime()
{
    RTC_DateTypeDef d = {};
    RTC_TimeTypeDef t = {};

    if(HAL_RTC_GetTime(&m_rtc, &t, RTC_FORMAT_BIN) != HAL_OK)
        return DateTime();

    HAL_RTC_WaitForSynchro(&m_rtc);

    if(HAL_RTC_GetDate(&m_rtc, &d, RTC_FORMAT_BIN) != HAL_OK)
        return DateTime();

    DateTime::Date date(d.Date,  d.Month,   2000+d.Year);
    DateTime::Time time(t.Hours, t.Minutes, t.Seconds);

    return DateTime(date, time);
}

bool RealTimeClock::setDateTime(const DateTime& dt)
{
    DateTime::Date date = dt.getDate();
    RTC_DateTypeDef d = {};
    d.Date  = date.d;
    d.Month = date.m;
    d.Year  = date.y - 2000;
    d.WeekDay = 0;

    DateTime::Time time = dt.getTime();
    RTC_TimeTypeDef t = {};
    t.Hours     = time.h;
    t.Minutes   = time.m;
    t.Seconds   = time.s;

#if !defined(STM32F1xx)
    t.TimeFormat     = RTC_HOURFORMAT_24;
    t.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    t.StoreOperation = RTC_STOREOPERATION_RESET;
#endif

    HAL_RTC_DeactivateAlarm(&(rtc.m_rtc), RTC_ALARM_A);

    if( HAL_RTC_SetTime(&m_rtc, &t, RTC_FORMAT_BIN) != HAL_OK ||
        HAL_RTC_SetDate(&m_rtc, &d, RTC_FORMAT_BIN) != HAL_OK)
        return true;

    HAL_RTC_WaitForSynchro(&m_rtc);

    return false;
}

bool RealTimeClock::setAlarm(const DateTime::Time& when)
{
    HAL_RTC_DeactivateAlarm(&m_rtc, RTC_ALARM_A);

    __HAL_RTC_ALARM_DISABLE_IT(&m_rtc, RTC_IT_ALRA);

#if !defined(STM32F1xx)
    __HAL_RTC_ALARM_DISABLE_IT(&m_rtc, RTC_IT_ALRB);
#endif

    RTC_AlarmTypeDef alarm = {};

    alarm.Alarm                 = RTC_ALARM_A;
    alarm.AlarmTime.Hours       = when.h;
    alarm.AlarmTime.Minutes     = when.m;
    alarm.AlarmTime.Seconds     = when.s;

#if !defined(STM32F1xx)
    alarm.AlarmTime.SubSeconds  = 0;
    alarm.AlarmTime.TimeFormat  = RTC_HOURFORMAT_24;
    alarm.AlarmMask             = RTC_ALARMMASK_DATEWEEKDAY;
    alarm.AlarmSubSecondMask    = RTC_ALARMSUBSECONDMASK_ALL;
    alarm.AlarmDateWeekDaySel   = RTC_ALARMDATEWEEKDAYSEL_DATE;
    alarm.AlarmDateWeekDay      = RTC_WEEKDAY_MONDAY;
#endif

    if(HAL_RTC_SetAlarm_IT(&m_rtc, &alarm, RTC_FORMAT_BIN) != HAL_OK)
        return true;

    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 4, 1);
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
/*
    __HAL_RTC_ALARM_ENABLE_IT(&m_rtc, RTC_IT_ALRA);
    __HAL_RTC_ALARM_ENABLE_IT(&m_rtc, RTC_IT_ALRB);

    __HAL_RTC_ALARM_EXTI_ENABLE_EVENT();*/

    return false;
}

bool RealTimeClock::isInitialized() const
{
    return HAL_RTC_GetState(const_cast<RTC_HandleTypeDef*>(&m_rtc)) != HAL_RTC_STATE_RESET;
}

void RealTimeClock::wakeupHandler()
{
    if(m_wakeupCallback)
        m_wakeupCallback();
}

#if !defined(STM32F1xx)
extern "C" void RTC_WKUP_IRQHandler(void)
{
	/* Check for RTC interrupt */
	if (__HAL_RTC_WAKEUPTIMER_GET_IT(&(rtc.m_rtc), RTC_IT_WUT) != RESET)
    {
		rtc.wakeupHandler();
		/* Clear interrupt flags */
		__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&(rtc.m_rtc), RTC_FLAG_WUTF);
	}
	
	/* Clear EXTI line 22 bit */
	__HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();
}
#endif

extern "C" void RTC_Alarm_IRQHandler(void)
{
    if (__HAL_RTC_ALARM_GET_IT(&(rtc.m_rtc), RTC_IT_ALRA) != RESET)
    {
		__HAL_RTC_ALARM_CLEAR_FLAG(&(rtc.m_rtc), RTC_FLAG_ALRAF);
        HAL_RTC_DeactivateAlarm(&(rtc.m_rtc), RTC_ALARM_A);
	}
	
#if !defined(STM32F1xx)
	if (__HAL_RTC_ALARM_GET_IT(&(rtc.m_rtc), RTC_IT_ALRB) != RESET)
    {
		__HAL_RTC_ALARM_CLEAR_FLAG(&(rtc.m_rtc), RTC_FLAG_ALRBF);
        HAL_RTC_DeactivateAlarm(&(rtc.m_rtc), RTC_ALARM_B);
	}
#endif

	__HAL_RTC_ALARM_EXTI_CLEAR_FLAG();
}


#endif /* #if defined(HAL_RTC_MODULE_ENABLED) */
