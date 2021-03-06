/*
 *  button.c
 *
 *  Created on: 2016. 7. 13.
 *      Author: Baram
 */
#include "hw.h"
#include "button.h"
#include "nrf_gpio.h"


typedef struct
{
  uint16_t      pin_number;
  uint8_t       on_state;
} button_port_t;

typedef struct
{
  bool        pressed;
  bool        pressed_event;
  uint16_t    pressed_cnt;
  uint32_t    pressed_start_time;
  uint32_t    pressed_end_time;

  uint32_t    released_start_time;
  uint32_t    released_end_time;

} button_t;


static button_port_t button_port_tbl[BUTTON_CH_MAX] =
{
  {20, 0}
};


static button_t button_tbl[BUTTON_CH_MAX];



int buttonCmdif(int argc, char **argv);



void button_isr(void *arg)
{
  uint8_t i;
  uint8_t ret;

  for (i=0; i<BUTTON_CH_MAX; i++)
  {
    ret = nrf_gpio_pin_read(button_port_tbl[i].pin_number);

    if (ret == button_port_tbl[i].on_state)
    {
      if (button_tbl[i].pressed == 0)
      {
        button_tbl[i].pressed_start_time = millis();
      }

      button_tbl[i].pressed       = 1;
      button_tbl[i].pressed_event = 1;
      button_tbl[i].pressed_cnt++;

      button_tbl[i].pressed_end_time = millis();
      button_tbl[i].released_start_time = millis();
      button_tbl[i].released_end_time   = millis();
    }
    else
    {
      button_tbl[i].pressed       = 0;

      if (button_tbl[i].pressed_event)
      {
        button_tbl[i].released_end_time = millis();
      }
    }
  }
}



bool buttonInit(void)
{
  swtimer_handle_t h_button_timer;
  uint32_t i;



  for (i=0; i<BUTTON_CH_MAX; i++)
  {
    button_tbl[i].pressed_cnt = 0;
    button_tbl[i].pressed     = 0;

    nrf_gpio_cfg_input(button_port_tbl[i].pin_number,NRF_GPIO_PIN_PULLUP);
  }


  h_button_timer = swtimerGetHandle();
  swtimerSet(h_button_timer, 1, LOOP_TIME, button_isr, NULL );
  swtimerStart(h_button_timer);


  cmdifAdd("button", buttonCmdif);

  return true;
}

bool buttonGetPressed(uint8_t ch)
{
  bool ret;


  if (ch >= BUTTON_CH_MAX) return false;

  ret = button_tbl[ch].pressed;

  return ret;
}

bool buttonGetPressedEvent(uint8_t ch)
{
  bool ret;


  if (ch >= BUTTON_CH_MAX) return false;

  ret = button_tbl[ch].pressed_event;

  button_tbl[ch].pressed_event = 0;

  return ret;
}

uint32_t buttonGetPressedTime(uint8_t ch)
{
  volatile uint32_t ret;


  if (ch >= BUTTON_CH_MAX) return 0;


  ret = button_tbl[ch].pressed_end_time - button_tbl[ch].pressed_start_time;

  return ret;
}

uint32_t buttonGetReleasedTime(uint8_t ch)
{
  volatile uint32_t ret;


  if (ch >= BUTTON_CH_MAX) return 0;


  ret = button_tbl[ch].released_end_time - button_tbl[ch].released_start_time;

  return ret;
}

//-- buttonCmdif
//
int buttonCmdif(int argc, char **argv)
{
  bool ret = true;
  uint8_t number;


  if (argc == 3)
  {
    number = (uint8_t) strtoul((const char * ) argv[2], (char **)NULL, (int) 0);

    if(strcmp("state", argv[1]) == 0)
    {
      while(cmdifRxAvailable() == 0)
      {
        ledToggle(number);
        delay(200);
        cmdifPrintf("button pressed : %d, event : %d, time %d, %d\n",
                    buttonGetPressed(number),
                    buttonGetPressedEvent(number),
                    buttonGetPressedTime(number),
                    buttonGetReleasedTime(number)
                    );
      }
    }
    else
    {
      ret = false;
    }
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "button state number ...\n");
  }

  return 0;
}
