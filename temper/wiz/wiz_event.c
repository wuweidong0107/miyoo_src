#include "wiz_common.h"

// When you press a button on Wiz, which config option should it
// correspond to? That is, which in config.pad, the order that
// the Wiz buttons are given in the menu. This says what, for each
// bit.

u32 wiz_to_config_map[] =
{
  WIZ_CONF_NONE,         // 0
  WIZ_CONF_NONE,         // 1
  WIZ_CONF_NONE,         // 2
  WIZ_CONF_NONE,         // 3
  WIZ_CONF_NONE,         // 4
  WIZ_CONF_NONE,         // 5
  WIZ_CONF_R,            // 6
  WIZ_CONF_L,            // 7
  WIZ_CONF_SELECT,       // 8
  WIZ_CONF_MENU,         // 9
  WIZ_CONF_VOL_UP,       // 10
  WIZ_CONF_VOL_DOWN,     // 11
  WIZ_CONF_NONE,         // 12
  WIZ_CONF_NONE,         // 13
  WIZ_CONF_NONE,         // 14
  WIZ_CONF_NONE,         // 15
  WIZ_CONF_LEFT,         // 16
  WIZ_CONF_RIGHT,        // 17
  WIZ_CONF_UP,           // 18
  WIZ_CONF_DOWN,         // 19
  WIZ_CONF_A,            // 20
  WIZ_CONF_B,            // 21
  WIZ_CONF_X,            // 22
  WIZ_CONF_Y,            // 23
  WIZ_CONF_NONE,         // 0
  WIZ_CONF_NONE,         // 0
  WIZ_CONF_NONE,         // 0
  WIZ_CONF_NONE,         // 0
  WIZ_CONF_NONE,         // 0
  WIZ_CONF_NONE,         // 0
  WIZ_CONF_NONE,         // 0
};

void update_input()
{
  u32 wiz_buttons = wiz_joystick_read();
  u32 button_status = 0;
  static u32 last_wiz_buttons;

  u32 i;

  // vol up + vol down cancels both but presses the imaginary vol mid button
  if((wiz_buttons & WIZ_VOL_UP) && (wiz_buttons & WIZ_VOL_DOWN))
  {
    wiz_buttons &= ~(WIZ_VOL_UP | WIZ_VOL_DOWN);
    wiz_buttons |= WIZ_VOL_MID;
  }

  for(i = 0; i < 28; i++)
  {
    if(wiz_buttons & (1 << i))
    {
      // Now perform the action and update the button status
      button_status |= button_action(wiz_to_config_map[i],
       last_wiz_buttons & (1 << i));
    }
  }

  last_wiz_buttons = wiz_buttons;

  if(button_status & IO_BUTTON_CLEAR)
    button_status = IO_BUTTON_NONE;

  io.button_status = (~button_status) & 0xFFF;
}

u32 platform_specific_button_action_direct(u32 button, u32 old_action)
{
  switch(button)
  {
    case CONFIG_BUTTON_VOLUME_DOWN:
      if(!old_action)
        wiz_sound_volume(-4);

      break;

    case CONFIG_BUTTON_VOLUME_UP:
      if(!old_action)
        wiz_sound_volume(4);

      break;
  }

  return 0;
}

u32 last_buttons = 0;
button_repeat_state_type button_repeat_state = BUTTON_NOT_HELD;

gui_action_type get_gui_input()
{
  gui_action_type new_button = CURSOR_NONE;
  u32 buttons = wiz_joystick_read();
  u32 new_buttons;

  static u64 button_repeat_timestamp;
  static u32 button_repeat = 0;
  static gui_action_type cursor_repeat = CURSOR_NONE;

  delay_us(10000);

  new_buttons = (last_buttons ^ buttons) & buttons;
  last_buttons = buttons;

  if(new_buttons & WIZ_A)
    new_button = CURSOR_BACK;

  if(new_buttons & WIZ_X)
    new_button = CURSOR_EXIT;

  if(new_buttons & WIZ_B)
    new_button = CURSOR_SELECT;

  if(new_buttons & WIZ_UP)
    new_button = CURSOR_UP;

  if(new_buttons & WIZ_DOWN)
    new_button = CURSOR_DOWN;

  if(new_buttons & WIZ_LEFT)
    new_button = CURSOR_LEFT;

  if(new_buttons & WIZ_RIGHT)
    new_button = CURSOR_RIGHT;

  if(new_buttons & WIZ_L)
    new_button = CURSOR_PAGE_UP;

  if(new_buttons & WIZ_R)
    new_button = CURSOR_PAGE_DOWN;

  if(new_button != CURSOR_NONE)
  {
    get_ticks_us(&button_repeat_timestamp);
    button_repeat_state = BUTTON_HELD_INITIAL;
    button_repeat = new_buttons;
    cursor_repeat = new_button;
  }
  else
  {
    if(buttons & button_repeat)
    {
      u64 new_ticks;
      get_ticks_us(&new_ticks);

      if(button_repeat_state == BUTTON_HELD_INITIAL)
      {
        if((new_ticks - button_repeat_timestamp) >
         BUTTON_REPEAT_START)
        {
          new_button = cursor_repeat;
          button_repeat_timestamp = new_ticks;
          button_repeat_state = BUTTON_HELD_REPEAT;
        }
      }

      if(button_repeat_state == BUTTON_HELD_REPEAT)
      {
        if((new_ticks - button_repeat_timestamp) >
         BUTTON_REPEAT_CONTINUE)
        {
          new_button = cursor_repeat;
          button_repeat_timestamp = new_ticks;
        }
      }
    }
  }

  return new_button;
}

void clear_gui_actions()
{
  last_buttons = 0;
  button_repeat_state = BUTTON_NOT_HELD;
}

void initialize_event()
{
}

